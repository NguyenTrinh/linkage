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

#ifndef ALERTMANAGER_HH
#define ALERTMANAGER_HH

#include <sigc++/signal.h>

#include <glibmm/ustring.h>

#include "libtorrent/alert_types.hpp"
#include "libtorrent/peer_id.hpp"
#include "libtorrent/time.hpp"
#include "libtorrent/intrusive_ptr_base.hpp"

#include "linkage/Torrent.hh"

namespace Linkage
{
class AlertManager;
typedef boost::intrusive_ptr<AlertManager> AlertManagerPtr;

class AlertManager : public sigc::trackable, public libtorrent::intrusive_ptr_base<AlertManager>
{
	sigc::signal<void, const Glib::ustring&> m_signal_listen_failed;
	sigc::signal<void, const Glib::ustring&> m_signal_portmap_failed;
	sigc::signal<void, const Glib::ustring&> m_signal_portmap_success;
	sigc::signal<void, const TorrentPtr&, const Glib::ustring&> m_signal_file_error;
	sigc::signal<void, const TorrentPtr&, const Glib::ustring&> m_signal_tracker_announce;
	sigc::signal<void, const TorrentPtr&, const Glib::ustring&, const Glib::ustring&, int, int> m_signal_tracker_failed;
	sigc::signal<void, const TorrentPtr&, const Glib::ustring&, const Glib::ustring&, int> m_signal_tracker_reply;
	sigc::signal<void, const TorrentPtr&, const Glib::ustring&> m_signal_tracker_warning;
	sigc::signal<void, const TorrentPtr&, const Glib::ustring&, const Glib::ustring&> m_signal_http_seed_failed;
	sigc::signal<void, const TorrentPtr&, const Glib::ustring&, int> m_signal_hash_failed;
	sigc::signal<void, const TorrentPtr&, const Glib::ustring&, const libtorrent::address&> m_signal_peer_banned;
	sigc::signal<void, const TorrentPtr&> m_signal_torrent_finished;
	sigc::signal<void, const TorrentPtr&, const Glib::ustring&> m_signal_fastresume_rejected;
	sigc::signal<void, const TorrentPtr&, const Glib::ustring&> m_signal_storage_moved;

	void check_alerts();

	AlertManager();
	
public:
	sigc::signal<void, const Glib::ustring&> signal_listen_failed()
	{
		return m_signal_listen_failed;
	}
	sigc::signal<void, const Glib::ustring&> signal_portmap_failed()
	{
		return m_signal_portmap_failed;
	}
	sigc::signal<void, const Glib::ustring&> signal_portmap_success()
	{
		return m_signal_portmap_success;
	}
	sigc::signal<void, const TorrentPtr&, const Glib::ustring&> signal_file_error()
	{
		return m_signal_file_error;
	}
	sigc::signal<void, const TorrentPtr&, const Glib::ustring&> signal_tracker_announce()
	{
		return m_signal_tracker_announce;
	}
	sigc::signal<void, const TorrentPtr&, const Glib::ustring&, const Glib::ustring&, int, int> signal_tracker_failed()
	{
		return m_signal_tracker_failed;
	}
	sigc::signal<void, const TorrentPtr&, const Glib::ustring&, const Glib::ustring&, int> signal_tracker_reply()
	{
		return m_signal_tracker_reply;
	}
	sigc::signal<void, const TorrentPtr&, const Glib::ustring&> signal_tracker_warning()
	{
		return m_signal_tracker_warning;
	}
	sigc::signal<void, const TorrentPtr&, const Glib::ustring&, const Glib::ustring&> signal_http_seed_failed()
	{
		return m_signal_http_seed_failed;
	}
	sigc::signal<void, const TorrentPtr&, const Glib::ustring&, int> signal_hash_failed()
	{
		return m_signal_hash_failed;
	}
	sigc::signal<void, const TorrentPtr&, const Glib::ustring&, const libtorrent::address&> signal_peer_banned()
	{
		return m_signal_peer_banned;
	}
	sigc::signal<void, const TorrentPtr&> signal_torrent_finished()
	{
		return m_signal_torrent_finished;
	}
	sigc::signal<void, const TorrentPtr&, const Glib::ustring&> signal_fastresume_rejected()
	{
		return m_signal_fastresume_rejected;
	}
	sigc::signal<void, const TorrentPtr&, const Glib::ustring&> signal_storage_moved()
	{
		return m_signal_storage_moved;
	}

	static AlertManagerPtr create();
	~AlertManager();
};

} /* namespace */

#endif /* ALERTMANAGER_HH */

