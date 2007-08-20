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
#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <fstream>
#include <iterator>
#include <exception>
#include <iostream>

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/session_settings.hpp"
#include "libtorrent/fingerprint.hpp"

#include "linkage/SettingsManager.hh"
#include "linkage/Torrent.hh"
#include "linkage/Utils.hh"
#include "linkage/RefCounter.hh"

class SessionManager : public RefCounter<SessionManager>, public libtorrent::session
{
	void on_settings();

	bool decode(const Glib::ustring& file, libtorrent::entry& e);
	bool decode(const Glib::ustring& file, libtorrent::entry& e, std::vector<char>& buffer);

	void on_torrent_finished(const libtorrent::sha1_hash& hash, const Glib::ustring& msg);

	sigc::signal<void, const Glib::ustring&, const Glib::ustring&> m_signal_invalid_bencoding;
	sigc::signal<void, const Glib::ustring&, const Glib::ustring&> m_signal_missing_file;
	sigc::signal<void, const Glib::ustring&, const libtorrent::sha1_hash&> m_signal_duplicate_torrent;
	
	SessionManager();
	
public:
	sigc::signal<void, const Glib::ustring&, const Glib::ustring&> signal_invalid_bencoding();
	sigc::signal<void, const Glib::ustring&, const Glib::ustring&> signal_missing_file();
	sigc::signal<void, const Glib::ustring&, const libtorrent::sha1_hash&> signal_duplicate_torrent();

	libtorrent::sha1_hash open_torrent(const Glib::ustring& file, const Glib::ustring& save_path);
	libtorrent::sha1_hash resume_torrent(const Glib::ustring& hash_str);
	libtorrent::sha1_hash resume_torrent(const libtorrent::sha1_hash& hash);
	void recheck_torrent(const libtorrent::sha1_hash& hash);
	void resume_session();
	void stop_torrent(const libtorrent::sha1_hash& hash);
	void erase_torrent(const libtorrent::sha1_hash& hash, bool erase_content = false);

	static Glib::RefPtr<SessionManager> create();
	virtual ~SessionManager();
};
#endif