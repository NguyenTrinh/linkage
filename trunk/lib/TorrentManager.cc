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
	m_session_manager = Engine::instance()->get_session_manager();

	m_session_manager->signal_update_queue().connect(sigc::mem_fun(*this, &TorrentManager::check_queue));
	
	Glib::RefPtr<AlertManager> am = Engine::instance()->get_alert_manager();
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

sigc::signal<void, const sha1_hash&, unsigned int> TorrentManager::signal_position_changed()
{
	return m_signal_position_changed;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&> TorrentManager::signal_group_changed()
{
	return m_signal_group_changed;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&, const Glib::ustring&, unsigned int> TorrentManager::signal_added()
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
		scode = "timed out"
	else	
		scode = str(code);
	
	Glib::ustring msg = "Failed: " + scode + " (" + str(times) + " times in a row)";
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
	m_signal_position_changed.emit(hash, m_torrents[hash]->get_position());
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
		add_torrent(e);
	m_torrents[handle.info_hash()]->set_handle(handle);
}

void TorrentManager::add_torrent(const entry& e)
{
	entry::dictionary_type ed(e.dict());

	if (!e.find_key("group"))
		ed["group"] = Engine::instance()->get_settings_manager()->get_string("Files", "DefGroup");

	if (!e.find_key("upload-limit"))
		ed["upload-limit"] = 0;

	if (!e.find_key("download-limit"))
		ed["download-limit"] = 0;

	if (!e.find_key("downloaded"))
		ed["downloaded"] = 0;

	if (!e.find_key("uploaded"))
		ed["uploaded"] = 0;

	if (!e.find_key("position"))
		ed["position"] = m_torrents.size() + 1;

	Torrent* torrent = new Torrent(ed, false);

	sha1_hash hash = info_hash(ed["info-hash"].string());
	m_torrents[hash] = torrent;

	m_signal_added.emit(hash, torrent->get_name(), torrent->get_group(), torrent->get_position());
}

void TorrentManager::remove_torrent(const sha1_hash& hash)
{
	int pos = m_torrents[hash]->get_position();
	delete m_torrents[hash];
	m_torrents.erase(hash);

	for (TorrentIter iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		Torrent* torrent = iter->second;
		if (torrent->get_position() > pos)
			torrent->set_position(torrent->get_position() - 1);
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

void TorrentManager::check_queue()
{
	std::vector<sha1_hash> order;
	unsigned int num_active = 0;
	unsigned int max_active = Engine::instance()->get_settings_manager()->get_int("Network", "MaxActive");
	//Sort m_torrents by position
	for (TorrentIter iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		Torrent* torrent = iter->second;
		if (torrent->is_running())
		{
			if (!torrent->is_queued())
				num_active++;
		}
		else
			continue;

		if (order.empty())
			order.push_back(torrent->get_hash());
		else
		{
			unsigned int offset = 0;
			while (offset < order.size() && m_torrents[order[offset]]->get_position() < torrent->get_position())
				offset++;
			order.insert(order.begin() + offset, torrent->get_hash());
		}
	}

	if (num_active == max_active) //Make sure queue is correct
	{
		/* Loop backwards until we hit the first running torrent */
		for (unsigned int i = order.size()-1; i > 0; i--)
		{
			Torrent* torrent = m_torrents[order[i]];
			if (!torrent->is_running() || (torrent->get_state() & Torrent::SEEDING))
				continue;

			if (!torrent->is_queued())
			{
				/* Loop trough all m_torrents above the current one, if they are queued,
					 unqueue all above and queue the current one */
				bool should_queue = false;
				for (unsigned int j = i; j > 0; j--)
				{
					Torrent* torrent_above = m_torrents[order[i]];
					if (torrent_above->is_queued())
					{
						should_queue = true;
						torrent_above->unqueue();
						num_active++;
					}
				}
				if (should_queue)
				{
					torrent->queue();
					num_active--;
				}
			}
		}
	}

	if (num_active > max_active) //Stop everything from bottom until queue is ok
	{
		unsigned int i = order.size() - 1;
		while (num_active != max_active && i > 0)
		{
			Torrent* torrent = m_torrents[order[i]];
			if (torrent->get_state() & Torrent::SEEDING)
				continue;

			if (torrent->is_running() && !torrent->is_queued())
			{
				torrent->queue();
				num_active--;
			}
			i--;
		}
	}

	if (num_active < max_active) //Start everything paused until queue is maxed
	{
		unsigned int i = 0;
		while (num_active != max_active && i < order.size())
		{
			Torrent* torrent = m_torrents[order[i]];
			if (torrent->is_running() && torrent->is_queued())
			{
				torrent->unqueue();
				num_active++;
			}
			i++;
		}
	}

	/*if (!num_active)
		Engine::instance()->stop_tick();
	else if (!Engine::instance()->is_ticking())
		Engine::instance()->start_tick();*/

}
