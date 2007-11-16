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

#include <glibmm/i18n.h>

#include "linkage/TorrentManager.hh"
#include "linkage/Engine.hh"
#include "linkage/AlertManager.hh"
#include "linkage/DbusManager.hh"
#include "linkage/SessionManager.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/Utils.hh"
#include "linkage/compose.hpp"

Glib::RefPtr<TorrentManager> TorrentManager::create()
{
	return Glib::RefPtr<TorrentManager>(new TorrentManager());
}

TorrentManager::TorrentManager()
{
	Glib::RefPtr<AlertManager> am = Engine::get_alert_manager();

	am->signal_tracker_announce().connect(sigc::mem_fun(*this, &TorrentManager::on_tracker_announce));
	am->signal_tracker_reply().connect(sigc::mem_fun(*this, &TorrentManager::on_tracker_reply));
	am->signal_tracker_warning().connect(sigc::mem_fun(*this, &TorrentManager::on_tracker_warning));
	am->signal_tracker_failed().connect(sigc::mem_fun(*this, &TorrentManager::on_tracker_failed));
	am->signal_torrent_finished().connect(sigc::mem_fun(*this, &TorrentManager::on_update_queue));
	am->signal_file_error().connect(sigc::mem_fun(*this, &TorrentManager::on_update_queue));

	Engine::get_settings_manager()->signal_key_changed().connect(sigc::mem_fun(*this, &TorrentManager::on_key_changed));
}

TorrentManager::~TorrentManager()
{
	for (TorrentMap::iterator iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		libtorrent::sha1_hash hash = iter->first;
		Glib::RefPtr<Torrent> torrent = iter->second;

		Engine::get_dbus_manager()->unregister_torrent(torrent);

		if (!torrent->is_stopped())
			torrent->get_handle().pause();
		libtorrent::entry e = torrent->get_resume_entry(torrent->is_stopped(), true);
		save_entry(Glib::build_filename(get_data_dir(), String::compose("%1", hash) + ".resume"), e);
		if (!torrent->is_stopped())
			Engine::get_session_manager()->remove_torrent(torrent->get_handle());
	}
}

sigc::signal<void, Glib::RefPtr<Torrent> > TorrentManager::signal_added()
{
	return m_signal_added;
}

sigc::signal<void, Glib::RefPtr<Torrent> > TorrentManager::signal_removed()
{
	return m_signal_removed;
}

void TorrentManager::on_key_changed(const Glib::ustring& key, const Value& value)
{
	if (Glib::str_has_prefix(key, "torrent/"))
	{
		Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();
		for (TorrentMap::iterator iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
			set_torrent_settings(iter->second);

		check_queue();
	}
}

void TorrentManager::on_tracker_announce(const libtorrent::sha1_hash& hash, const Glib::ustring& msg)
{
	if (exists(hash) && !Glib::str_has_suffix(msg, "event=stopped"))
	{
		// FIXME: should we save resume data more often?
		libtorrent::entry e = m_torrents[hash]->get_resume_entry(false);
		save_entry(Glib::build_filename(get_data_dir(), String::compose("%1", hash) + ".resume"), e);

		m_torrents[hash]->set_tracker_reply(_("Announcing"), "", Torrent::REPLY_ANNOUNCING);
	}
}

void TorrentManager::on_tracker_reply(const libtorrent::sha1_hash& hash, const Glib::ustring& reply, int peers)
{
	if (exists(hash) && reply.size() > 27)
	{
		Glib::ustring tracker = reply.substr(27);
		m_torrents[hash]->set_tracker_reply(String::ucompose(_(
			"OK, got %1 peers"), peers), tracker, Torrent::REPLY_OK);
	}
}

void TorrentManager::on_tracker_warning(const libtorrent::sha1_hash& hash, const Glib::ustring& reply)
{
	if (exists(hash) && !Glib::str_has_prefix(reply, "Redirecting to \""))
		m_torrents[hash]->set_tracker_reply(reply);
}

void TorrentManager::on_tracker_failed(const libtorrent::sha1_hash& hash, const Glib::ustring& reply, int code, int times)
{
	if (exists(hash))
	{
		int pos = reply.find(" ", 9);
		// FIXME: use our own error messages so we can translate them
		Glib::ustring msg = reply.substr(pos + 1);
		Glib::ustring tracker = reply.substr(10, pos - 11);

		Glib::ustring s = msg;

		if (times > 1)
			s = String::ucompose(_("%1 (%2 times in a row)"), s, times);

		m_torrents[hash]->set_tracker_reply(s, tracker);
	}
}

void TorrentManager::on_update_queue(const libtorrent::sha1_hash& hash, const Glib::ustring& msg)
{
	check_queue();
}

void TorrentManager::on_handle_changed(const libtorrent::sha1_hash& hash)
{
	Glib::RefPtr<Torrent> torrent = m_torrents[hash];
	set_torrent_settings(torrent);

	check_queue();
}

void TorrentManager::on_position_changed(const libtorrent::sha1_hash& hash)
{
	Glib::RefPtr<Torrent> torrent = m_torrents[hash];
	unsigned int position = torrent->get_position();
	int diff = 0;

	TorrentVector torrents;
	for (TorrentMap::iterator iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		torrents.push_back(iter->second);
	}

	std::sort(torrents.begin(), torrents.end(), pred());

	//find if the torrent was moved up or down, and how far
	for (int i = 0; i < (int)torrents.size() - 1; i++)
	{
		unsigned int p1 = torrents[i]->get_position();
		unsigned int p2 = torrents[i + 1]->get_position();
		// special case, first torrent moved down
		if (i == 0 && p1 > 1)
		{
			diff = 1 - p1;
			break;
		}
		//special case, last torrent moved up
		if (i == (int)(torrents.size() - 2) && p2 != torrents.size())
		{
			diff = p2 - p1 + 1;
			break;
		}
		//otherwise
		if (p1 != (p2-1) && p1 != p2)
		{
			diff = p2 - position - 1;
			break;
		}
	}

	TorrentMap::iterator iter;
	for (iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		unsigned int p = iter->second->get_position();
		if (p == position && iter->first != torrent->get_hash())
		{
			iter->second->set_position(position + diff);
			break;
		}
	}

	if (iter == m_torrents.end())
		check_queue();
}

void TorrentManager::set_torrent_settings(const Glib::RefPtr<Torrent>& torrent)
{
	if (!torrent->is_stopped())
	{
		Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();
		libtorrent::torrent_handle handle = torrent->get_handle();
		handle.set_ratio(sm->get_float("torrent/seed_ratio"));
		handle.set_max_uploads(sm->get_int("torrent/max_uploads"));
		handle.set_max_connections(sm->get_int("torrent/max_connections"));
		/* FIXME: Make this configurable ? */
		handle.resolve_countries(true);
	}
}

bool TorrentManager::exists(const libtorrent::sha1_hash& hash)
{
	return (m_torrents.find(hash) != m_torrents.end());
}

bool TorrentManager::exists(const Glib::ustring& hash_str)
{
	for (TorrentMap::iterator iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		if (hash_str == String::compose("%1", iter->first))
			return true;
	}
	return false;
}

Glib::RefPtr<Torrent> TorrentManager::add_torrent(const libtorrent::entry& e,
	const boost::intrusive_ptr<libtorrent::torrent_info>& info)
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

	if (!ri.resume.find_key("completed"))
		ri.resume["completed"] = 0;

	if (!ri.resume.find_key("position"))
		ri.resume["position"] = m_torrents.size() + 1;

	Glib::RefPtr<Torrent> torrent = Torrent::create(ri, false);
	libtorrent::sha1_hash hash = info->info_hash();
	// Seems we can't bind with the torrent RefPtr
	// if we do we crash when the signalproxies destroys
	// and there seems to be no way of disconnect them either :/
	torrent->property_handle().signal_changed().connect(sigc::bind(
		sigc::mem_fun(this, &TorrentManager::on_handle_changed), hash));
	torrent->property_position().signal_changed().connect(sigc::bind(
		sigc::mem_fun(this, &TorrentManager::on_position_changed), hash));

	m_torrents[hash] = torrent;

	Engine::get_dbus_manager()->register_torrent(torrent);
	m_signal_added.emit(torrent);

	return torrent;
}

void TorrentManager::remove_torrent(const libtorrent::sha1_hash& hash)
{
	Glib::RefPtr<Torrent> torrent = m_torrents[hash];
	unsigned int pos = torrent->get_position();

	Engine::get_dbus_manager()->unregister_torrent(torrent);

	m_torrents.erase(hash);

	TorrentVector torrents;
	for (TorrentMap::iterator iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		Glib::RefPtr<Torrent> torrent = iter->second;
		if (torrent->get_position() > pos)
			torrents.push_back(torrent);
	}

	std::sort(torrents.begin(), torrents.end(), pred());

	for (unsigned int i = 0; i < torrents.size(); i++)
	{
		torrents[i]->set_position(pos + i);
	}

	m_signal_removed.emit(torrent);

	check_queue();
}

Glib::RefPtr<Torrent>TorrentManager::get_torrent(const libtorrent::sha1_hash& hash)
{
	if (exists(hash))
		return m_torrents[hash];

	return Glib::RefPtr<Torrent>();
}

TorrentManager::TorrentList TorrentManager::get_torrents()
{
	TorrentList list;
	for (TorrentMap::iterator iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
		list.push_back(iter->second);

	return list;
}

unsigned int TorrentManager::get_torrents_count()
{
	return m_torrents.size();
}

void TorrentManager::check_queue()
{
	int last_active = 0;
	int num_active = 0;
	int max_active = Engine::get_settings_manager()->get_int("torrent/queue/max_active");

	g_return_if_fail(max_active > 0);

	TorrentVector torrents;
	for (TorrentMap::iterator iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		Glib::RefPtr<Torrent> torrent = iter->second;
		int state = torrent->get_state();
		if (state & (Torrent::SEEDING|Torrent::STOPPED|Torrent::ERROR))
			continue;

		torrents.push_back(torrent);
		if (!torrent->is_queued())
		{
			num_active++;
			if (torrent->get_position() > last_active)
				last_active = torrent->get_position();
		}
	}

	std::sort(torrents.begin(), torrents.end(), pred());

	for (unsigned int k = 0; k < torrents.size(); k++)
	{
		Glib::RefPtr<Torrent> torrent = torrents[k];
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
