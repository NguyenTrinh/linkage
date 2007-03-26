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

#include "libtorrent/alert_types.hpp"

#include "linkage/AlertManager.hh"
#include "linkage/Engine.hh"

Glib::RefPtr<AlertManager> AlertManager::create()
{
	return Glib::RefPtr<AlertManager>(new AlertManager());
}

AlertManager::AlertManager() : RefCounter<AlertManager>::RefCounter(this)
{
	Engine::signal_tick().connect(sigc::mem_fun(this, &AlertManager::check_alerts));
}

AlertManager::~AlertManager()
{
}

sigc::signal<void, const Glib::ustring&> 
AlertManager::signal_listen_failed()
{
	return m_signal_listen_failed;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&, int, int> 
AlertManager::signal_tracker_failed()
{
	return m_signal_tracker_failed;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&, int> 
AlertManager::signal_tracker_reply()
{
	return m_signal_tracker_reply;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&> 
AlertManager::signal_tracker_warning()
{
	return m_signal_tracker_warning;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&> 
AlertManager::signal_tracker_announce()
{
	return m_signal_tracker_announce;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&> 
AlertManager::signal_torrent_finished()
{
	return m_signal_torrent_finished;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&> 
AlertManager::signal_file_error()
{
	return m_signal_file_error;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&> AlertManager::signal_fastresume_rejected()
{
	return m_signal_fastresume_rejected;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&, int> 
AlertManager::signal_hash_failed()
{
	return m_signal_hash_failed;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&, const Glib::ustring&> 
AlertManager::signal_peer_ban()
{
	return m_signal_peer_ban;
}

	
void AlertManager::check_alerts()
{
	std::auto_ptr<alert> a = Engine::get_session_manager()->pop_alert();

	while (a.get())
	{
		if (listen_failed_alert* p = dynamic_cast<listen_failed_alert*>(a.get()))
		{
			m_signal_listen_failed.emit(p->msg());
		}
		else if (tracker_warning_alert* p = dynamic_cast<tracker_warning_alert*>(a.get()))
		{
			m_signal_tracker_warning.emit(p->handle.info_hash(), p->msg());
		}
		else if (tracker_alert* p = dynamic_cast<tracker_alert*>(a.get()))
		{
			m_signal_tracker_failed.emit(p->handle.info_hash(), p->msg(), p->status_code, p->times_in_row);
		}
		else if (tracker_reply_alert* p = dynamic_cast<tracker_reply_alert*>(a.get()))
		{
			m_signal_tracker_reply.emit(p->handle.info_hash(), p->msg(), 0 /* FIXME: for LT 0.12: p->num_peers*/);
		}
		else if (tracker_announce_alert* p = dynamic_cast<tracker_announce_alert*>(a.get()))
		{
			m_signal_tracker_announce.emit(p->handle.info_hash(), p->msg());
		}
		else if (torrent_finished_alert* p = dynamic_cast<torrent_finished_alert*>(a.get()))
		{
			m_signal_torrent_finished.emit(p->handle.info_hash(), p->msg());
		}
		else if (file_error_alert* p = dynamic_cast<file_error_alert*>(a.get()))
		{
			m_signal_file_error.emit(p->handle.info_hash(), p->msg());
		}
		else if (fastresume_rejected_alert* p = dynamic_cast<fastresume_rejected_alert*>(a.get()))
		{
			m_signal_fastresume_rejected.emit(p->handle.info_hash(), p->msg());
		}
		else if (hash_failed_alert* p = dynamic_cast<hash_failed_alert*>(a.get()))
		{
			m_signal_hash_failed.emit(p->handle.info_hash(), p->msg(), p->piece_index);
		}
		else if (peer_ban_alert* p = dynamic_cast<peer_ban_alert*>(a.get()))
		{
			m_signal_peer_ban.emit(p->handle.info_hash(), p->msg(), p->ip.address().to_string());
		}
		a = Engine::get_session_manager()->pop_alert();
	}
}
