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

#include "linkage/Torrent.hh"
#include "linkage/Utils.hh"

Torrent::Torrent(int position, sha1_hash hash, const Glib::ustring& group, int time)
{
  time_ = time;
  position_ = position;
  hash_ = hash;
  group_ = group;
  up_limit = 0;
  down_limit = 0;
}

Torrent::~Torrent()
{
}

sigc::signal<void, const sha1_hash&, Torrent::Direction> Torrent::signal_position_changed()
{
  return signal_position_changed_;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&> Torrent::signal_group_changed()
{
  return signal_group_changed_;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&> Torrent::signal_response_changed()
{
  return signal_response_changed_;
}

const torrent_handle& Torrent::get_handle()
{
  return handle_;
}

/* TODO: Remove? This is only needed when resuming 
   a stopped torrent from previous session */
const Glib::ustring& Torrent::get_name()
{
  return name_;
}

const Glib::ustring& Torrent::get_group()
{
  return group_;
}

const Glib::ustring& Torrent::get_tracker_response()
{
  return response_;
}

const int Torrent::get_position()
{
  return position_;
}

const int Torrent::get_time_active()
{
  //int time = SettingsManager::instance()->get<int>(str(hash_), "Time"); Signal for this instead?
  int time = time_;
  if (handle_.is_valid())
  {
    Glib::TimeVal diff;
    g_get_current_time(&diff);
    diff -= time_val;
    time += (int)diff.as_double();
  }
  return time;
}

const std::vector<bool>& Torrent::get_filter()
{
  return filter_;
}

const int Torrent::get_up_limit()
{
  return up_limit;
}

const int Torrent::get_down_limit()
{
  return down_limit;
}

const sha1_hash& Torrent::get_hash()
{
  return hash_;
}  

void Torrent::set_handle(torrent_handle handle)
{
  handle_ = handle;
}

void Torrent::set_name(const Glib::ustring& name)
{
  name_ = name;
}

void Torrent::set_group(const Glib::ustring& group)
{
  group_ = group;
  signal_group_changed_.emit(hash_, group_);
}

void Torrent::set_tracker_response(const Glib::ustring& response)
{
  response_ = response;
  signal_response_changed_.emit(hash_, response_);
}

void Torrent::set_position(int position)
{
  Direction direction = (position < position_) ? DIR_UP : DIR_DOWN;
  position_ = position;
  signal_position_changed_.emit(hash_, direction);
}

void Torrent::set_filter(std::vector<bool>& filter)
{
  if (filter != filter_)
    filter_.assign(filter.begin(), filter.end());
    
  /* TODO: Thread this? It completly freezes UI on large files.. */
  if (handle_.is_valid())
    handle_.filter_files(filter_);
}

void Torrent::filter_file(const Glib::ustring& name)
{
  torrent_info info = handle_.get_torrent_info();
  
  /* Get the file index to i */
  int i = 0;
  for (i = 0; i < info.num_files(); i++)
    if (name == info.file_at(i).path.string())
      break;

  filter_[i] = true;
  
  set_filter(filter_);
}

void Torrent::set_up_limit(int limit)
{
  up_limit = limit;
}

void Torrent::set_down_limit(int limit)
{
  down_limit = limit;
}

void Torrent::set_start_time()
{
  time_val.assign_current_time();
}

void Torrent::set_stop_time()
{
  time_ = get_time_active();
}
