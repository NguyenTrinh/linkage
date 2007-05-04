/*
Copyright (C) 2006	Christian Lundgren

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA	02110-1301, USA.
*/

#include <vector>

#include "linkage/Engine.hh"
#include "linkage/TorrentManager.hh"

Glib::RefPtr<TorrentManager> TorrentManager::create()
{
	return Glib::RefPtr<TorrentManager>(new TorrentManager());
}

TorrentManager::TorrentManager() : RefCounter<TorrentManager>::RefCounter(this)
{
	m_session_manager = Engine::get_session_manager();
	
	Glib::RefPtr<AlertManager> am = Engine::get_alert_manager();

	am->signal_tracker_announce().connect(sigc::mem_fun(*this, &TorrentManager::on_tracker_announce));
	am->signal_tracker_reply().connect(sigc::mem_fun(*this, &TorrentManager::on_tracker_reply));
	am->signal_tracker_warning().connect(sigc::mem_fun(*this, &TorrentManager::on_tracker_warning));
	am->signal_tracker_failed().connect(sigc::mem_fun(*this, &TorrentManager::on_tracker_failed));
	am->signal_torrent_finished().connect(sigc::mem_fun(*this, &TorrentManager::on_update_queue));
	am->signal_file_error().connect(sigc::mem_fun(*this, &TorrentManager::on_update_queue));

	Engine::get_settings_manager()->signal_update_settings().connect(sigc::mem_fun(*this, &TorrentManager::on_settings));
}

TorrentManager::~TorrentManager()
{
	for (TorrentIter iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		sha1_hash hash = iter->first;
		Torrent* torrent = iter->second;

		Glib::ustring hash_str = str(hash);

		entry e = torrent->get_resume_entry(!torrent->is_stopped());
		save_entry(hash, e, ".resume");
		if (!torrent->is_stopped())
			m_session_manager->remove_torrent(torrent->get_handle());

		delete torrent;
	}
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&, unsigned int> TorrentManager::signal_added()
{
	return m_signal_added;
}

sigc::signal<void, const sha1_hash&> TorrentManager::signal_removed()
{
	return m_signal_removed;
}

void TorrentManager::on_settings()
{
	Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();
	for (TorrentIter iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		on_handle_changed(iter->second);
	}

	check_queue();
}

void TorrentManager::on_tracker_announce(const sha1_hash& hash, const Glib::ustring& msg)
{
	if (exists(hash) && !Glib::str_has_suffix(msg, "event=stopped"))
		m_torrents[hash]->set_tracker_reply("Announcing");
}

void TorrentManager::on_tracker_reply(const sha1_hash& hash, const Glib::ustring& reply, int peers)
{
	if (exists(hash))
	{
		Glib::ustring tracker = reply.substr(27);
		m_torrents[hash]->set_tracker_reply("OK, got " + str(peers) + " peers", tracker);
	}
}

void TorrentManager::on_tracker_warning(const sha1_hash& hash, const Glib::ustring& reply)
{
	if (exists(hash) && !Glib::str_has_prefix(reply, "Redirecting to \""))
		m_torrents[hash]->set_tracker_reply(reply);
}

void TorrentManager::on_tracker_failed(const sha1_hash& hash, const Glib::ustring& reply, int code, int times)
{
	if (exists(hash))
	{
		int pos = reply.find(" ", 9);
		Glib::ustring msg = reply.substr(pos + 1);
		Glib::ustring tracker = reply.substr(10, pos - 11);

		Glib::ustring s = msg;

		if (times > 1)
			s = s + " (" + str(times) + " times in a row)";

		m_torrents[hash]->set_tracker_reply(s, tracker);
	}
}

void TorrentManager::on_update_queue(const sha1_hash& hash, const Glib::ustring& msg)
{
	check_queue();
}

void TorrentManager::on_handle_changed(Torrent* torrent)
{
	if (!torrent->is_stopped())
	{
		Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();
		torrent_handle handle = torrent->get_handle();
		float ratio;
		std::istringstream(sm->get_string("Network", "SeedRatio")) >> ratio;
		handle.set_ratio(ratio);
		handle.set_max_uploads(sm->get_int("Network", "MaxTorrentUploads"));
		handle.set_max_connections(sm->get_int("Network", "MaxTorrentConnections"));
		/* FIXME: Make this configurable */
		handle.resolve_countries(true);
	}

	check_queue();
}

void TorrentManager::set_torrent_position(const sha1_hash& hash, int diff)
{
	for (TorrentIter iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		unsigned int position = iter->second->get_position();
		if (position == m_torrents[hash]->get_position() && iter->first != hash)
			iter->second->set_position(position + diff);
	}
	check_queue();
}

bool TorrentManager::exists(const sha1_hash& hash)
{
	return (m_torrents.find(hash) != m_torrents.end());
}

bool TorrentManager::exists(const Glib::ustring& hash_str)
{
	for (TorrentIter iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		if (hash_str == str(iter->first))
			return true;
	}
	return false;
}

WeakPtr<Torrent> TorrentManager::add_torrent(const entry& e, const torrent_info& info)
{
	Torrent::ResumeInfo ri(e, info);

	/* FIXME: Make sure ri.resume["path"] not is empty, this is rare but could happen */

	if (!ri.resume.find_key("upload-limit"))
		ri.resume["upload-limit"] = 0;

	if (!ri.resume.find_key("download-limit"))
		ri.resume["download-limit"] = 0;

	if (!ri.resume.find_key("downloaded"))
		ri.resume["downloaded"] = 0;

	if (!ri.resume.find_key("uploaded"))
		ri.resume["uploaded"] = 0;

	/*
		Make sure we don't have two torrents with the same position,
		or a gap in the queue i.e. 1,2,4,5 -> 1,2,3,4.

		This might completly screw the previous order, but oh well.
	*/
	int position;
	if (ri.resume.find_key("position"))
		position = ri.resume["position"].integer();
	else
		position = m_torrents.size() + 1;

	std::vector<Torrent*> torrents;
	for (TorrentIter iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
		torrents.push_back(iter->second);
	sort(torrents);

	bool should_change = false;
	int last_position = 0;
	for (unsigned int i = 0; i < torrents.size(); i++)
	{
		should_change = (position == torrents[i]->get_position()); 
										//|| ((++last_position) != torrents[i]->get_position());
		if (should_change)
			break;
	}

	int new_position = 0;
	bool found = !should_change;
	while (!found)
	{
		new_position++;
		for (TorrentIter iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
		{
			found = (new_position != iter->second->get_position());
			if (!found)
				break;
		}
		if (found)
			position = new_position;
	}

	ri.resume["position"] = position;

	Torrent* torrent = new Torrent(ri, false);
	torrent->property_handle().signal_changed().connect(sigc::bind(
		sigc::mem_fun(this, &TorrentManager::on_handle_changed), torrent));

	sha1_hash hash = info.info_hash();
	m_torrents[hash] = torrent;

	m_signal_added.emit(hash, torrent->get_name(), torrent->get_position());

	check_queue();

	return WeakPtr<Torrent>(torrent);
}

void TorrentManager::remove_torrent(const sha1_hash& hash)
{
	int pos = m_torrents[hash]->get_position();
	delete m_torrents[hash];
	m_torrents.erase(hash);

	std::vector<Torrent*> torrents;
	for (TorrentIter iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		Torrent* torrent = iter->second;
		if (torrent->get_position() > pos)
			torrents.push_back(torrent);
	}
	sort(torrents);
	for (unsigned int i = 0; i < torrents.size(); i++)
	{
		torrents[i]->set_position(pos + i);
	}

	m_signal_removed.emit(hash);

	check_queue();
}

torrent_handle TorrentManager::get_handle(const sha1_hash& hash)
{
	torrent_handle handle;
	
	if (exists(hash))
		handle = m_torrents[hash]->get_handle();

	return handle;
}

WeakPtr<Torrent> TorrentManager::get_torrent(const sha1_hash& hash)
{
	if (exists(hash))
		return WeakPtr<Torrent>(m_torrents[hash]);

	return WeakPtr<Torrent>();
}

TorrentManager::TorrentList TorrentManager::get_torrents()
{
	TorrentList list;
	for (TorrentIter iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
		list.push_back(WeakPtr<Torrent>(iter->second));

	return list;
}

unsigned int TorrentManager::get_torrents_count()
{
	return m_torrents.size();
}

void TorrentManager::sort(std::vector<Torrent*>& torrents)
{
	for (unsigned int i = 0; i < torrents.size(); i++)
	{
		Torrent* torrent = torrents[i];
		unsigned int j = i;
		while (j > 0)
		{
			if (torrents[j - 1]->get_position() > torrent->get_position())
				torrents[j] = torrents[j - 1];
			else
				break;
			j--;
		}
		torrents[j] = torrent;
	}
}

void TorrentManager::check_queue()
{
	unsigned int last_active = 0;
	unsigned int num_active = 0;
	unsigned int max_active = Engine::get_settings_manager()->get_int("Network", "MaxActive");
	std::vector<Torrent*> torrents;
	for (TorrentIter iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		Torrent* torrent = iter->second;
		Torrent::State state = torrent->get_state();
		if (state == Torrent::SEEDING || state == Torrent::STOPPED || state == Torrent::ERROR)
			continue;

		torrents.push_back(torrent);
		if (!torrent->is_queued())
		{
			num_active++;
			if (torrent->get_position() > last_active)
				last_active = torrent->get_position();
		}
	}

	sort(torrents);

	for (unsigned int k = 0; k < torrents.size(); k++)
	{
		Torrent* torrent = torrents[k];
		if (!torrent->is_queued())
		{
			if (torrent->get_position() >= last_active && num_active > max_active)
			{
				num_active--;
				torrent->queue();
				for (unsigned int l = k; l > 0; l--)
					if (torrents[l]->get_state() != Torrent::QUEUED)
						last_active = torrents[l]->get_position();
			}
		}
		else
		{
			if (torrent->get_position() < last_active || num_active < max_active)
			{
				num_active++;
				torrent->unqueue();
				for (unsigned int l = k; l < torrents.size(); l++)
					if (torrents[l]->get_state() != Torrent::QUEUED)
						last_active = torrents[l]->get_position();
			}
		}
	}

	if (num_active > max_active)
		check_queue();
}
