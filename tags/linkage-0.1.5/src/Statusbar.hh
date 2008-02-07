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

#ifndef STATUSBAR_HH
#define STATUSBAR_HH

#include <gtkmm/statusbar.h>
#include <gtkmm/label.h>

#include <libglademm.h>

#include "libtorrent/peer_id.hpp"

#include "linkage/Torrent.hh"

class Statusbar : public Gtk::Statusbar
{
	Gtk::Label* m_connections;
	Gtk::Label* m_download;
	Gtk::Label* m_upload;

	void post(const Glib::ustring& msg);

	void on_duplicate_torrent(const Glib::ustring& msg, const libtorrent::sha1_hash& hash);

	void on_listen_failed(const Glib::ustring& msg);
	void on_portmap_failed(const Glib::ustring& msg);
	void on_portmap_success(const Glib::ustring& msg);
	void on_tracker_announce(const Linkage::TorrentPtr& torrent, const Glib::ustring& msg);
	void on_tracker_failed(const Linkage::TorrentPtr& torrent, const Glib::ustring& msg, const Glib::ustring& tracker, int code, int times);
	void on_tracker_reply(const Linkage::TorrentPtr& torrent, const Glib::ustring& msg, const Glib::ustring& tracker, int peers);
	void on_tracker_warning(const Linkage::TorrentPtr& torrent, const Glib::ustring& msg);
	void on_http_seed_failed(const Linkage::TorrentPtr& torrent, const Glib::ustring& msg, const Glib::ustring& url);
	void on_hash_failed(const Linkage::TorrentPtr& torrent, const Glib::ustring& msg, int);
	void on_peer_banned(const Linkage::TorrentPtr& torrent, const Glib::ustring& msg, const libtorrent::address&);

public:
	void set_status(int connections, float down_rate, float up_rate);
	
	Statusbar(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	~Statusbar();
};

#endif /* STATUSBAR_HH */
