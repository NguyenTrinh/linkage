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

#define INVALID_HASH 0

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
  enum State { 
               CHECK_QUEUE  = 0x001,
               CHECKING     = 0x002,
               ANNOUNCING   = 0x004,
               DOWNLOADING  = 0x008,
               FINISHED     = 0x010,
               SEEDING      = 0x020,
               ALLOCATING   = 0x040,
               STOPED       = 0x080,
               QUEUED       = 0x100
             };
  
  struct ResumeInfo {
                      int time;
                      int uploaded;
                      int downloaded;
                    };

  const torrent_handle& get_handle();
  const Glib::ustring& get_tracker_reply();
  const Glib::ustring& get_name();
  const Glib::ustring& get_group();
  const Glib::ustring& get_save_path();
  const int get_position();
  const int get_time_active();
  const std::vector<bool>& get_filter();
  const int get_up_limit();
  const int get_down_limit();
  const sha1_hash& get_hash();
  
  const int get_total_downloaded();
  const int get_total_uploaded();
  
  const State get_state();
  const Glib::ustring get_state_string();
  const Glib::ustring get_state_string(State state);
  
  const torrent_info get_info();
  const torrent_status get_status();
  const std::vector<partial_piece_info> get_download_queue();
  const std::vector<float> get_file_progress();
  
  void set_tracker_reply(const Glib::ustring& reply);
  void set_group(const Glib::ustring& group);
  void set_position(int position);
  void set_filter(std::vector<bool>& filter);
  void filter_file(const Glib::ustring& name);
  void set_up_limit(int limit);
  void set_down_limit(int limit);
  
  void start();
  void stop();
  
  void queue();
  void unqueue();
  bool is_queued();
  bool is_running();
  bool is_valid();
  
  const entry get_resume_entry(bool running = false);
  
  /* FIXME: Friend access only? */
  void set_handle(const torrent_handle& handle);
  
  Torrent(const entry& resume_data, bool queued = false);
  Torrent();
  ~Torrent();

protected: 
  torrent_handle m_handle;
  sha1_hash m_hash;
  
  Glib::TimeVal m_time_val;
  
  Glib::ustring m_tracker_reply;
  
  std::vector<bool> m_filter;
  
  bool m_is_queued;
  
  int m_position;
  Glib::ustring m_name, m_group, m_save_path;
  ResumeInfo m_resume;
  int m_up_limit, m_down_limit;
};

#endif /* TORRENT_HH */
