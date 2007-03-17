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
	/* FIXME: add on_tracker_announce and set reply to <i>Trying http://my.tracker</i> */
	am->signal_tracker_reply().connect(sigc::mem_fun(*this, &TorrentManager::on_tracker_reply));
	am->signal_tracker_warning().connect(sigc::mem_fun(*this, &TorrentManager::on_tracker_warning));
	am->signal_tracker_failed().connect(sigc::mem_fun(*this, &TorrentManager::on_tracker_failed));
	am->signal_torrent_finished().connect(sigc::mem_fun(*this, &TorrentManager::on_update_queue));
	am->signal_file_error().connect(sigc::mem_fun(*this, &TorrentManager::on_update_queue));
}

TorrentManager::~TorrentManager()
{
	for (TorrentIter iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		sha1_hash hash = iter->first;
		Torrent* torrent = iter->second;

		Glib::ustring hash_str = str(hash);

		entry e = torrent->get_resume_entry(torrent->is_running());
		save_entry(hash, e, ".resume");
		if (torrent->is_running())
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

void TorrentManager::on_tracker_reply(const sha1_hash& hash, const Glib::ustring& reply, int peers)
{
	m_torrents[hash]->set_tracker_reply("OK, got " + str(peers) + " peers");
}

void TorrentManager::on_tracker_warning(const sha1_hash& hash, const Glib::ustring& reply)
{
	m_torrents[hash]->set_tracker_reply(reply);
}

void TorrentManager::on_tracker_failed(const sha1_hash& hash, const Glib::ustring& reply, int code, int times)
{
	Glib::ustring scode;
	if (code == 200)
	{
		/* Get rid of tracker: "http://foo.bar" */
		Glib::ustring tracker_msg = reply.substr(reply.find(" ", 9));
		m_torrents[hash]->set_tracker_reply(tracker_msg);
		return;
	}
	
	if (code == -1)
		scode = "connection refused";
	else if (code == 0)
		scode = "timed out";
	else	
		scode = str(code);
	
	Glib::ustring msg = "Failed: " + scode;
	if (times > 1)
		msg = msg + " (" + str(times) + " times in a row)";

	m_torrents[hash]->set_tracker_reply(msg);
}

void TorrentManager::on_update_queue(const sha1_hash& hash, const Glib::ustring& msg)
{
	check_queue();
}

void TorrentManager::on_handle_changed()
{
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

	if (!ri.resume.find_key("upload-limit"))
		ri.resume["upload-limit"] = 0;

	if (!ri.resume.find_key("download-limit"))
		ri.resume["download-limit"] = 0;

	if (!ri.resume.find_key("downloaded"))
		ri.resume["downloaded"] = 0;

	if (!ri.resume.find_key("uploaded"))
		ri.resume["uploaded"] = 0;

	if (!ri.resume.find_key("position"))
		ri.resume["position"] = m_torrents.size() + 1;
	else
	{
		/* Make sure we don't have two torrents with the same position */
		int position = ri.resume["position"].integer();
		bool taken = false;
		for (TorrentIter iter = m_torrents.begin(); 
					iter != m_torrents.end() && !taken; ++iter)
		{
			taken = (position == iter->second->get_position());
		}

		int new_position = 0;
		while (taken)
		{
			new_position++;
			for (TorrentIter iter = m_torrents.begin(); 
					iter != m_torrents.end(); ++iter)
			{
				taken = (new_position == iter->second->get_position());
				if (taken)
					break;
			}
			if (!taken)
				position = new_position;
		}

		ri.resume["position"] = position;
	}

	Torrent* torrent = new Torrent(ri, false);
	torrent->property_handle().signal_changed().connect(sigc::mem_fun(this, &TorrentManager::on_handle_changed));

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
			if (num_active < max_active)
			{
				num_active++;
				if (torrent->get_position() > last_active)
					last_active = torrent->get_position();
			}
			else
				torrent->queue();
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
				last_active = torrent->get_position();
			}
		}
	}
}
