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

#ifndef TORRENTMANAGER_HH
#define TORRENTMANAGER_HH

#include <map>

#include "linkage/Torrent.hh"

typedef std::map<sha1_hash, Torrent*> TorrentMap;
typedef TorrentMap::iterator TorrentIter;

class TorrentManager
{
  TorrentMap torrents;
  static TorrentManager* smInstance;
  
  void on_torrent_position_changed(const sha1_hash& hash, Torrent::Direction direction);
  void on_torrent_group_changed(const sha1_hash& hash, const Glib::ustring& group);
  void on_torrent_response_changed(const sha1_hash& hash, const Glib::ustring& response);
  
  sigc::signal<void, const sha1_hash&, int> signal_position_changed_;
  sigc::signal<void, const sha1_hash&, const Glib::ustring&> signal_group_changed_;
  sigc::signal<void, const sha1_hash&, const Glib::ustring&> signal_response_changed_;
  sigc::signal<void, const sha1_hash&, const Glib::ustring&, const Glib::ustring&, int> signal_added_;
  sigc::signal<void, const sha1_hash&> signal_removed_;
  
  friend class Torrent;
  
public:
  sigc::signal<void, const sha1_hash&, int> signal_position_changed();
  sigc::signal<void, const sha1_hash&, const Glib::ustring&> signal_group_changed();
  sigc::signal<void, const sha1_hash&, const Glib::ustring&> signal_response_changed();
  sigc::signal<void, const sha1_hash&, const Glib::ustring&, const Glib::ustring&, int> signal_added();
  sigc::signal<void, const sha1_hash&> signal_removed();
  
  static TorrentManager* instance();
  static void goodnight();
  
  void check_queue(); /* FIXME: Should be protected, not public? */
  
  bool exists(const sha1_hash& hash);
  bool exists(const Glib::ustring& hash_str);
  
  void add_torrent(torrent_handle handle);
  void add_torrent(torrent_info info);
  void remove_torrent(const sha1_hash& hash);
  
  torrent_handle get_handle(const sha1_hash& hash);
  Torrent* get_torrent(const sha1_hash& hash);
  Torrent* get_torrent(int position);
  
  int get_torrents_count();
  
  TorrentManager();
  ~TorrentManager();
};

#endif /* TORRENTMANAGER_HH */
