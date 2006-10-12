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

#ifndef TORRENT_HH
#define TORRENT_HH

#include <vector>

#include <glibmm/ustring.h>
#include <glibmm/timeval.h>
#include <sigc++/signal.h>

#include "libtorrent/torrent.hpp"

using namespace libtorrent;


typedef std::list<sha1_hash> HashList;

class Torrent
{
public:
  enum Direction { DIR_UP, DIR_DOWN };

  sigc::signal<void, const sha1_hash&, Direction> signal_position_changed();
  sigc::signal<void, const sha1_hash&, const Glib::ustring&> signal_group_changed();
  sigc::signal<void, const sha1_hash&, const Glib::ustring&> signal_response_changed();

  const torrent_handle& get_handle();
  const Glib::ustring& get_name();
  const Glib::ustring& get_group();
  const Glib::ustring& get_tracker_response();
  const int get_position();
  const int get_time_active();
  const std::vector<bool>& get_filter();
  const int get_up_limit();
  const int get_down_limit();
  const sha1_hash& get_hash();
  
  void set_handle(torrent_handle handle);
  void set_name(const Glib::ustring& name);
  void set_group(const Glib::ustring& group);
  void set_tracker_response(const Glib::ustring& response);
  void set_position(int position);
  void set_filter(std::vector<bool>& filter);
  void filter_file(const Glib::ustring& name);
  void set_up_limit(int limit);
  void set_down_limit(int limit);
  
  void set_start_time();
  void set_stop_time();
  
  Torrent(int position, sha1_hash hash, const Glib::ustring& group, int time = 0);
  ~Torrent();

protected:
  Glib::TimeVal time_val;
  int time_;
  sha1_hash hash_;
  torrent_handle handle_;
  int position_;
  Glib::ustring response_;
  Glib::ustring name_;
  Glib::ustring group_;
  
  std::vector<bool> filter_;
  int up_limit, down_limit;
  
  sigc::signal<void, const sha1_hash&, Direction> signal_position_changed_;
  sigc::signal<void, const sha1_hash&, const Glib::ustring&> signal_group_changed_;
  sigc::signal<void, const sha1_hash&, const Glib::ustring&> signal_response_changed_;
};

#endif /* TORRENT_HH */
