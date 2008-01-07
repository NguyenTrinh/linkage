/*
Copyright (C) 2006	Christian Lundgren

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied waUrranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA	02110-1301, USA.
*/
#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <fstream>
#include <iterator>
#include <exception>
#include <iostream>
#include <list>

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/session_settings.hpp"
#include "libtorrent/fingerprint.hpp"
#include "libtorrent/intrusive_ptr_base.hpp"

#include "linkage/Torrent.hh"

namespace Linkage
{
class SessionManager;
typedef boost::intrusive_ptr<SessionManager> SessionManagerPtr;

class Value;
//class Torrent;

class SessionManager : public libtorrent::intrusive_ptr_base<SessionManager>, public libtorrent::session
{
	void on_key_changed(const Glib::ustring& key, const Value& value);
	void update_pe_settings();
	void update_proxy_settings();
	void update_session_settings();

	void on_torrent_finished(const libtorrent::sha1_hash& hash, const Glib::ustring& msg);

	sigc::signal<void, const Glib::ustring&, const Glib::ustring&> m_signal_invalid_bencoding;
	sigc::signal<void, const Glib::ustring&, const Glib::ustring&> m_signal_missing_file;
	sigc::signal<void, const Glib::ustring&, const libtorrent::sha1_hash&> m_signal_duplicate_torrent;

	SessionManager();

public:
	sigc::signal<void, const Glib::ustring&, const Glib::ustring&> signal_invalid_bencoding();
	sigc::signal<void, const Glib::ustring&, const Glib::ustring&> signal_missing_file();
	sigc::signal<void, const Glib::ustring&, const libtorrent::sha1_hash&> signal_duplicate_torrent();

	TorrentPtr open_torrent(const Glib::ustring& file, const Glib::ustring& save_path);
	void resume_torrent(const TorrentPtr& torrent, const libtorrent::entry& resume = libtorrent::entry());
	void recheck_torrent(const TorrentPtr& torrent);
	void stop_torrent(const TorrentPtr& torrent);
	void erase_torrent(const TorrentPtr& torrent, bool erase_content = false);

	static SessionManagerPtr create();
	virtual ~SessionManager();
};

} /* namespace */

#endif
