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
#include <fstream>

#include "linkage/Engine.hh"
#include "linkage/TorrentManager.hh"

Glib::RefPtr<TorrentManager> TorrentManager::create()
{
	return Glib::RefPtr<TorrentManager>(new TorrentManager());
}

TorrentManager::TorrentManager() : RefCounter<TorrentManager>::RefCounter(this)
{
	m_session_manager = Engine::get_session_manager();

	Engine::signal_tick().connect(sigc::mem_fun(*this, &TorrentManager::check_queue));
	
	Glib::RefPtr<AlertManager> am = Engine::get_alert_manager();
	/* FIXME: add on_tracker_announce and set reply to <i>Trying http://my.tracker</i> */
	am->signal_tracker_reply().connect(sigc::mem_fun(*this, &TorrentManager::on_tracker_reply));
	am->signal_tracker_warning().connect(sigc::mem_fun(*this, &TorrentManager::on_tracker_warning));
	am->signal_tracker_failed().connect(sigc::mem_fun(*this, &TorrentManager::on_tracker_failed));
}

TorrentManager::~TorrentManager()
{
	for (TorrentIter iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		sha1_hash hash = iter->first;
		Torrent* torrent = iter->second;

		Glib::ustring hash_str = str(hash);

		entry e = torrent->get_resume_entry(torrent->is_running());
		save_fastresume(hash, e);
		if (torrent->is_running())
			m_session_manager->remove_torrent(torrent->get_handle());

		delete torrent;
	}
}

void TorrentManager::save_fastresume(const sha1_hash& hash, const entry& e)
{
	Glib::ustring file = Glib::build_filename(get_data_dir(), str(hash) + ".resume");
	std::ofstream out(file.c_str(), std::ios_base::binary);
	out.unsetf(std::ios_base::skipws);
	bencode(std::ostream_iterator<char>(out), e);
	out.close();
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

void TorrentManager::set_torrent_position(const sha1_hash& hash, Torrent::Direction direction)
{
	for (TorrentIter iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		unsigned int position = iter->second->get_position();
		if (position == m_torrents[hash]->get_position() && iter->first != hash)
		{
			if (direction == Torrent::DIR_UP)
				iter->second->set_position(position + 1);
			else if (direction == Torrent::DIR_DOWN)
				iter->second->set_position(position - 1);
		}
	}
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

void TorrentManager::add_torrent(const torrent_handle& handle, const entry& e)
{
	if (m_torrents.find(handle.info_hash()) == m_torrents.end())
		add_torrent(e, handle.get_torrent_info());
	m_torrents[handle.info_hash()]->set_handle(handle);
}

void TorrentManager::add_torrent(const entry& e, const torrent_info& info)
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

	Torrent* torrent = new Torrent(ri, false);

	sha1_hash hash = info.info_hash();
	m_torrents[hash] = torrent;

	m_signal_added.emit(hash, torrent->get_name(), torrent->get_position());
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
}

torrent_handle TorrentManager::get_handle(const sha1_hash& hash)
{
	torrent_handle handle;
	for (TorrentIter iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		if (iter->first == hash)
			handle = iter->second->get_handle();
	}
	return handle;
}

WeakPtr<Torrent> TorrentManager::get_torrent(const sha1_hash& hash)
{
	for (TorrentIter iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		if (iter->first == hash)
			return WeakPtr<Torrent>(iter->second);
	}
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
	std::vector<Torrent*> torrents;
	for (TorrentIter iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		Torrent* torrent = iter->second;
		Torrent::State state = torrent->get_state();
		if (state == Torrent::SEEDING || state == Torrent::STOPPED || state == Torrent::ERROR)
			continue;

		torrents.push_back(torrent);
		if (torrent->get_state() != Torrent::QUEUED)
		{
			num_active++;
			if (torrent->get_position() > last_active)
				last_active = torrent->get_position();
		}
	}

	sort(torrents);

	unsigned int max_active = Engine::get_settings_manager()->get_int("Network", "MaxActive");
	for (unsigned int k = 0; k < torrents.size(); k++)
	{
		Torrent* torrent = torrents[k];
		Torrent::State state = torrent->get_state();
		if (state != Torrent::QUEUED)
		{
			if (torrent->get_position() >= last_active && num_active > max_active)
			{
				num_active--;
				torrent->queue();
				for (unsigned int l = 0; l < k; l++)
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
