/*
Copyright (C) 2006  Christian Lundgren

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#define MAJOR 0;
#define MINOR 19;

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

using namespace libtorrent;

class SessionManager : public session, public RefCounter<SessionManager>
{
  void on_settings();
  
  sigc::signal<void> m_signal_update_queue;
  sigc::signal<void> m_signal_session_resumed;
  
  sigc::signal<void, const Glib::ustring&, const Glib::ustring&> m_signal_invalid_bencoding;
  sigc::signal<void, const Glib::ustring&, const Glib::ustring&> m_signal_missing_file;
  sigc::signal<void, const Glib::ustring&, const sha1_hash&> m_signal_duplicate_torrent;
  
  SessionManager();
  
public:
  sigc::signal<void> signal_update_queue();
  sigc::signal<void> signal_session_resumed();
  
  sigc::signal<void, const Glib::ustring&, const Glib::ustring&> signal_invalid_bencoding();
  sigc::signal<void, const Glib::ustring&, const Glib::ustring&> signal_missing_file();
  sigc::signal<void, const Glib::ustring&, const sha1_hash&> signal_duplicate_torrent();
  
  sha1_hash open_torrent(const Glib::ustring& file, const Glib::ustring& save_path);
  sha1_hash resume_torrent(const Glib::ustring& hash_str);
  sha1_hash resume_torrent(const sha1_hash& hash);
  void resume_session();
  void stop_torrent(const sha1_hash& hash);
  void erase_torrent(const sha1_hash& hash);

  static Glib::RefPtr<SessionManager> create();
  virtual ~SessionManager();
};
#endif
