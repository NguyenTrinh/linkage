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

#include <glibmm/i18n.h>

#include "linkage/AlertManager.hh"
#include "linkage/Engine.hh"
#include "linkage/SessionManager.hh"
#include "linkage/TorrentManager.hh"
#include "linkage/ucompose.hpp"

using namespace Linkage;

static TorrentPtr get_torrent(const libtorrent::torrent_handle& handle)
{
	return Engine::get_torrent_manager()->get_torrent(handle.info_hash());
}

AlertManagerPtr AlertManager::create()
{
	return AlertManagerPtr(new AlertManager());
}

AlertManager::AlertManager()
{
	Engine::signal_tick().connect(sigc::mem_fun(this, &AlertManager::check_alerts));
}

AlertManager::~AlertManager()
{
}

void AlertManager::check_alerts()
{
	using namespace libtorrent;

	std::auto_ptr<alert> a = Engine::get_session_manager()->pop_alert();

	while (a.get())
	{
		if (listen_failed_alert* p = dynamic_cast<listen_failed_alert*>(a.get()))
		{
			Glib::ustring translated, msg = p->msg();
			if (Glib::str_has_prefix(msg, "cannot listen on interface '"))
			{
				Glib::ustring interface = msg.substr(28, msg.length() - msg.rfind("'") - 2);
				translated = String::ucompose(_("Cannot listen on interface %1"), interface);
			}
			else if (Glib::str_has_prefix(msg, "error accepting connection on '"))
			{
				Glib::ustring interface = msg.substr(31, msg.length() - msg.rfind("'") - 2);
				translated = String::ucompose(_("Error accepting connection on %1"), interface);
			}
			else if (Glib::str_has_prefix(msg, "cannot bind to interface '"))
			{
				Glib::ustring interface = msg.substr(26, msg.length() - msg.rfind("'") - 2);
				translated = String::ucompose(_("Cannot bind to interface %1"), interface);
			}
			m_signal_listen_failed.emit(translated);
		}
		else if (portmap_error_alert* p = dynamic_cast<portmap_error_alert*>(a.get()))
		{
			m_signal_portmap_failed.emit(_("Failed to map ports"));
		}
		else if (portmap_alert* p = dynamic_cast<portmap_alert*>(a.get()))
		{
			Glib::ustring translated, msg = p->msg();
			Glib::ustring port = msg.substr(29);
			if (Glib::str_has_prefix(msg, "successfully mapped UDP port "))
				translated = String::ucompose(_("Mapped UDP port %1"), port);
			else
				translated = String::ucompose(_("Mapped TCP port %1"), port);
			m_signal_portmap_success.emit(translated);
		}
		else if (file_error_alert* p = dynamic_cast<file_error_alert*>(a.get()))
		{
			Glib::ustring translated, msg = p->msg();
			if (Glib::str_has_prefix(msg, "torrent paused: disk write error"))
				translated = _("Disk write failed");
			else if (Glib::str_has_prefix(msg, "torrent paused: disk read error"))
				translated = _("Disk read failed");
			else
				translated = _("No file permissions or invalid filename");
			TorrentPtr torrent = get_torrent(p->handle);
			m_signal_file_error.emit(torrent, translated);
		}
		else if (tracker_announce_alert* p = dynamic_cast<tracker_announce_alert*>(a.get()))
		{
			Glib::ustring translated, msg = p->msg();
			if (Glib::str_has_suffix(msg, "event=stopped"))
				translated = _("Announcing as stopped");
			else
				translated = _("Announcing");
			TorrentPtr torrent = get_torrent(p->handle);
			if (torrent)
				m_signal_tracker_announce.emit(torrent, translated);
		}
		else if (tracker_alert* p = dynamic_cast<tracker_alert*>(a.get()))
		{
			Glib::ustring translated, msg = p->msg();
			Glib::ustring tracker = msg.substr(10, msg.find(" ", 9) - 11);
			switch (p->status_code)
			{
			case 0:
				translated = _("Timed out");
				break;
			case -1:
				translated = _("No route to host");
				break;
			case 200:
				translated = msg.substr(msg.find(" ", 9) + 1);
				break;
			default:
				translated = String::ucompose(_("Failed: %1"), p->status_code);
				break;
			}
			TorrentPtr torrent = get_torrent(p->handle);
			m_signal_tracker_failed.emit(torrent, translated, tracker, p->status_code, p->times_in_row);
		}
		else if (tracker_reply_alert* p = dynamic_cast<tracker_reply_alert*>(a.get()))
		{
			Glib::ustring translated, tracker, msg = p->msg();
			if (msg.size() > 27)
				tracker = msg.substr(27);
			else
				tracker = "DHT";
			translated = String::ucompose(_("OK, got %1 peers"), p->num_peers);
			TorrentPtr torrent = get_torrent(p->handle);
			m_signal_tracker_reply.emit(torrent, translated, tracker, p->num_peers);
		}
		else if (tracker_warning_alert* p = dynamic_cast<tracker_warning_alert*>(a.get()))
		{
			TorrentPtr torrent = get_torrent(p->handle);
			m_signal_tracker_warning.emit(torrent, p->msg());
		}
		else if (url_seed_alert* p = dynamic_cast<url_seed_alert*>(a.get()))
		{
			TorrentPtr torrent = get_torrent(p->handle);
			m_signal_http_seed_failed(torrent, p->msg(), p->url);
		}
		else if (hash_failed_alert* p = dynamic_cast<hash_failed_alert*>(a.get()))
		{
			Glib::ustring translated = String::ucompose(_("Hash failed for piece %1"), p->piece_index);
			TorrentPtr torrent = get_torrent(p->handle);
			m_signal_hash_failed.emit(torrent, translated, p->piece_index);
		}
		else if (peer_ban_alert* p = dynamic_cast<peer_ban_alert*>(a.get()))
		{
			Glib::ustring translated = String::ucompose(_("Banning peer %1 for sending bad data"), p->ip.address().to_string());
			TorrentPtr torrent = get_torrent(p->handle);
			m_signal_peer_banned.emit(torrent, translated, p->ip.address());
		}
		else if (torrent_finished_alert* p = dynamic_cast<torrent_finished_alert*>(a.get()))
		{
			TorrentPtr torrent = get_torrent(p->handle);
			if (torrent && !torrent->is_completed())
			{
				torrent->set_completed(true);
				m_signal_torrent_finished.emit(torrent);
			}
		}
		else if (fastresume_rejected_alert* p = dynamic_cast<fastresume_rejected_alert*>(a.get()))
		{
			Glib::ustring translated, tracker, msg = p->msg();
			if (Glib::str_has_prefix(msg, "incompatible file version "))
			{
				Glib::ustring version = msg.substr(26);
				translated = String::ucompose(_("Incompatible file version: %1"), version);
			}
			else if (Glib::str_has_prefix(msg, "mismatching info-hash: "))
			{
				Glib::ustring hash = msg.substr(23);
				translated = String::ucompose(_("Mismatching hash: %1"), hash);
			}
			else if (Glib::str_has_prefix(msg, "checksum mismatch on piece "))
			{
				Glib::ustring index = msg.substr(27);
				translated = String::ucompose(_("Checksum failed for piece %1"), index);
			}
			else if (Glib::str_has_prefix(msg, "the number of files does not match the torrent"))
			{
				translated = _("Mismatching number of files");
			}
			else if (Glib::str_has_prefix(msg, "file size for "))
			{
				Glib::ustring::size_type pos = msg.find(" was expected to be ");
				Glib::ustring file = msg.substr(14, pos);
				translated = String::ucompose(_("File size mismatch for %1"), file);
			}
			else // Other too detailed errors
				translated = _("Fast resume rejected, content check forced");
			TorrentPtr torrent = get_torrent(p->handle);
			if (torrent)
				torrent->set_completed(false);

			m_signal_fastresume_rejected.emit(torrent, translated);
		}
		else if (storage_moved_alert* p = dynamic_cast<storage_moved_alert*>(a.get()))
		{
			TorrentPtr torrent = get_torrent(p->handle);
			m_signal_storage_moved.emit(torrent, p->msg());
		}

		a = Engine::get_session_manager()->pop_alert();
	}
}
