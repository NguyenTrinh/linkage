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
#include "linkage/SessionManager.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/Utils.hh"
#include "linkage/compose.hpp"

using namespace Linkage;

TorrentManagerPtr TorrentManager::create()
{
	return TorrentManagerPtr(new TorrentManager());
}

TorrentManager::TorrentManager()
{
	load_torrents();

	/* connect signal handlers */
	AlertManagerPtr am = Engine::get_alert_manager();

	am->signal_tracker_announce().connect(sigc::mem_fun(*this, &TorrentManager::on_tracker_announce));
	am->signal_tracker_reply().connect(sigc::mem_fun(*this, &TorrentManager::on_tracker_reply));
	am->signal_tracker_warning().connect(sigc::mem_fun(*this, &TorrentManager::on_tracker_warning));
	am->signal_tracker_failed().connect(sigc::mem_fun(*this, &TorrentManager::on_tracker_failed));
	am->signal_torrent_finished().connect(sigc::bind(sigc::mem_fun(*this, &TorrentManager::on_update_queue), Glib::ustring()));
	am->signal_file_error().connect(sigc::mem_fun(*this, &TorrentManager::on_update_queue));

	Engine::get_settings_manager()->signal_key_changed().connect(sigc::mem_fun(*this, &TorrentManager::on_key_changed));
}

TorrentManager::~TorrentManager()
{
	for (TorrentMap::iterator iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		libtorrent::sha1_hash hash = iter->first;
		TorrentPtr torrent = iter->second;

		if (!torrent->is_stopped())
			torrent->get_handle().pause();
		/* FIXME: wait for torrent_paused_alert */
		libtorrent::entry e = torrent->get_resume_entry();
	
		save_entry(Glib::build_filename(get_data_dir(), String::compose("%1", hash) + ".resume"), e);
		if (!torrent->is_stopped())
			Engine::get_session_manager()->remove_torrent(torrent->get_handle());
	}
}

sigc::signal<void, TorrentPtr> TorrentManager::signal_added()
{
	return m_signal_added;
}

sigc::signal<void, TorrentPtr> TorrentManager::signal_removed()
{
	return m_signal_removed;
}

void TorrentManager::on_key_changed(const Glib::ustring& key, const Value& value)
{
	//FIXME: also update priorities on key "files/prioritize_firstlast"
	if (Glib::str_has_prefix(key, "torrent/"))
	{
		SettingsManagerPtr sm = Engine::get_settings_manager();
		for (TorrentMap::iterator iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
			set_torrent_settings(iter->second);

		check_queue();
	}
}

void TorrentManager::on_tracker_announce(const TorrentPtr& torrent, const Glib::ustring& msg)
{
	if (!torrent->is_stopped())
	{
		// FIXME: should we save resume data more often?
		libtorrent::entry e = torrent->get_resume_entry();
		save_entry(Glib::build_filename(get_data_dir(), String::compose("%1", torrent->get_hash()) + ".resume"), e);
	}

	torrent->set_tracker_reply(msg, "", Torrent::REPLY_ANNOUNCING);
}

void TorrentManager::on_tracker_reply(const TorrentPtr& torrent, const Glib::ustring& reply, const Glib::ustring& tracker, int peers)
{
	if (tracker != "DHT")
		torrent->set_tracker_reply(reply, tracker, Torrent::REPLY_OK);
}

void TorrentManager::on_tracker_warning(const TorrentPtr& torrent, const Glib::ustring& reply)
{
	if (!Glib::str_has_prefix(reply, "Redirecting to \""))
		torrent->set_tracker_reply(reply);
}

void TorrentManager::on_tracker_failed(const TorrentPtr& torrent, const Glib::ustring& reply, const Glib::ustring& tracker, int code, int times)
{
	Glib::ustring msg = reply;
	if (times > 1)
		msg = String::ucompose(_("%1 (%2 times in a row)"), msg, times);

	torrent->set_tracker_reply(msg, tracker);
}

void TorrentManager::on_update_queue(const TorrentPtr& torrent, const Glib::ustring& msg)
{
	check_queue();
}

void TorrentManager::on_handle_changed(const WeakTorrentPtr& weak)
{
	if (weak.expired())
		return;

	set_torrent_settings(weak.lock());

	check_queue();
}

void TorrentManager::on_position_changed(const WeakTorrentPtr& weak)
{
	TorrentPtr torrent = weak.lock();
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

void TorrentManager::load_torrents()
{
	ResumeList resumes;

	/* load torrent info from disk cache */
	Glib::Dir dir(get_data_dir());
	for (Glib::DirIterator iter = dir.begin(); iter != dir.end(); ++iter)
	{
		Glib::ustring hash_str = *iter;
		Glib::ustring file = Glib::build_filename(get_data_dir(), hash_str);
		/* only load torrents that has a .resume file */
		if (Glib::file_test(file + ".resume", Glib::FILE_TEST_EXISTS))
			load_torrent(file, resumes);
	}

	/* resume previously active torrents */
	for (ResumeList::iterator iter = resumes.begin(); iter != resumes.end(); ++iter)
	{
		Engine::get_session_manager()->resume_torrent(iter->first, iter->second);
	}
}

void TorrentManager::load_torrent(const Glib::ustring& file, ResumeList& resumes)
{
	libtorrent::entry e, er;

	/* skip if we can't load torrent file */
	if (!load_entry(file, e))
		return;

	Torrent::InfoPtr info(new libtorrent::torrent_info(e));

	/* signal if we can't load resume file */
	if (!load_entry(file + ".resume", er))
	{
		m_signal_load_failed.emit(file + ".resume", info);
		return;
	}

	/* make sure we have not lost track of the content */
	if (!er.find_key("path"))
	{
		m_signal_load_failed.emit(file + ".resume", info);
		return;
	}

	TorrentPtr torrent = add_torrent(er, info);
	if (er.find_key("stopped") && !er["stopped"].integer())
		resumes.push_back(std::make_pair(torrent, er));
}

TorrentPtr TorrentManager::add_torrent(libtorrent::entry& er,
	const Torrent::InfoPtr& info)
{
	/* set default values if they don't exist */
	if (!er.find_key("upload-limit"))
		er["upload-limit"] = 0;
	if (!er.find_key("download-limit"))
		er["download-limit"] = 0;
	if (!er.find_key("downloaded"))
		er["downloaded"] = 0;
	if (!er.find_key("uploaded"))
		er["uploaded"] = 0;
	if (!er.find_key("completed"))
		er["completed"] = 0;
	if (!er.find_key("position"))
		er["position"] = m_torrents.size() + 1;

	TorrentPtr torrent = Torrent::create(er, info, false);
	m_torrents[info->info_hash()] = torrent;

	WeakTorrentPtr weak(torrent);
	torrent->property_handle().signal_changed().connect(sigc::bind(
		sigc::mem_fun(this, &TorrentManager::on_handle_changed), weak));
	torrent->property_position().signal_changed().connect(sigc::bind(
		sigc::mem_fun(this, &TorrentManager::on_position_changed), weak));

	m_signal_added.emit(torrent);

	return torrent;
}

void TorrentManager::set_torrent_settings(const TorrentPtr& torrent)
{
	if (!torrent->is_stopped())
	{
		SettingsManagerPtr sm = Engine::get_settings_manager();
		libtorrent::torrent_handle handle = torrent->get_handle();
		torrent->set_stop_ratio(sm->get_float("torrent/stop_ratio"));
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

void TorrentManager::remove_torrent(const TorrentPtr& torrent)
{
	unsigned int pos = torrent->get_position();

	m_torrents.erase(torrent->get_hash());

	TorrentVector torrents;
	for (TorrentMap::iterator iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		TorrentPtr t = iter->second;
		if (t->get_position() > pos)
			torrents.push_back(t);
	}

	std::sort(torrents.begin(), torrents.end(), pred());

	for (unsigned int i = 0; i < torrents.size(); i++)
	{
		torrents[i]->set_position(pos + i);
	}

	m_signal_removed.emit(torrent);

	/* duplicated in on_handle_changed? */
	check_queue();
}

TorrentPtr TorrentManager::get_torrent(const libtorrent::sha1_hash& hash)
{
	if (exists(hash))
		return m_torrents[hash];

	return TorrentPtr();
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
	unsigned int last_active = 0;
	int num_active = 0;
	int max_active = Engine::get_settings_manager()->get_int("torrent/queue/max_active");

	g_return_if_fail(max_active > 0);

	TorrentVector torrents;
	for (TorrentMap::iterator iter = m_torrents.begin(); iter != m_torrents.end(); ++iter)
	{
		TorrentPtr torrent = iter->second;
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
		TorrentPtr torrent = torrents[k];
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

