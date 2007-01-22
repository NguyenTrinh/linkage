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
#include "linkage/Engine.hh"

Torrent::Torrent()
{
  m_hash = INVALID_HASH;
}

Torrent::Torrent(const entry& resume_entry, bool queued)
{  
  m_is_queued = queued;
  
  m_hash = info_hash(resume_entry["info-hash"].string());
  
  m_resume.downloaded = resume_entry["downloaded"].integer();
  m_resume.uploaded = resume_entry["uploaded"].integer();
  m_resume.time = resume_entry["time"].integer();
  
  m_name = resume_entry["name"].string();
  m_group = resume_entry["group"].string();
  m_save_path = resume_entry["path"].string();
  m_position = resume_entry["position"].integer();
  
  m_up_limit = resume_entry["upload-limit"].integer();
  m_down_limit = resume_entry["download-limit"].integer();
  
  std::list<entry> fl = resume_entry["filter"].list();
  entry first = *fl.begin();
  m_filter.assign(first.integer(), false);
  for (std::list<entry>::iterator iter = ++fl.begin();
        iter != fl.end(); ++iter)
  {
    entry e = *iter;
    m_filter[e.integer()] = true;
  }
}

Torrent::~Torrent()
{
}

const torrent_handle& Torrent::get_handle() const
{
  /* This should maybe be removed completly or only available for friends */
  return m_handle;
}

const Glib::ustring& Torrent::get_name() const
{
  return m_name;
}

const Glib::ustring& Torrent::get_group() const
{
  return m_group;
}

const Glib::ustring& Torrent::get_save_path() const
{
  return m_save_path;
}

const Glib::ustring& Torrent::get_tracker_reply() const
{
  return m_tracker_reply;
}

const unsigned int Torrent::get_position() const
{
  return m_position;
}

const unsigned int Torrent::get_time_active() const
{
  unsigned int total_time = m_resume.time;
  if (m_handle.is_valid())
  {
    Glib::TimeVal diff;
    g_get_current_time(&diff);
    diff -= m_time_val;
    total_time += (unsigned int)diff.as_double();
  }
  return total_time;
}

const std::vector<bool>& Torrent::get_filter() const
{
  return m_filter;
}

const int Torrent::get_up_limit() const
{
  return m_up_limit;
}

const int Torrent::get_down_limit() const
{
  return m_down_limit;
}

const sha1_hash& Torrent::get_hash() const
{
  return m_hash;
}

const unsigned int Torrent::get_total_downloaded() const
{
	unsigned int total = m_resume.downloaded;
	if (m_handle.is_valid())
		total += m_handle.status().total_download;
	return total;
}

const unsigned int Torrent::get_total_uploaded() const
{
	unsigned int total = m_resume.downloaded;
	if (m_handle.is_valid())
		total += m_handle.status().total_upload;
	return total;
}

const Torrent::State Torrent::get_state() const
{
  if (!m_is_queued)
  {
    if (m_handle.is_valid())
    {
      unsigned int state = m_handle.status().state;
      switch (state)
      {
        case torrent_status::queued_for_checking:
          return CHECK_QUEUE;
        case torrent_status::checking_files:
          return CHECKING;
        case torrent_status::connecting_to_tracker:
          return ANNOUNCING;
        case torrent_status::downloading:
          return DOWNLOADING;
        case torrent_status::finished:
          return FINISHED;
        case torrent_status::seeding:
          return SEEDING;
        case torrent_status::allocating:
          return ALLOCATING;
      }
    }
    else
      return STOPED;
  }
  else
    return QUEUED;
}

const Glib::ustring Torrent::get_state_string() const
{
  if (!m_is_queued)
  {
    if (m_handle.is_valid())
    {
      unsigned int state = m_handle.status().state;
      switch (state)
      {
        case torrent_status::queued_for_checking:
          return "Queued";
        case torrent_status::checking_files:
          return "Checking";
        case torrent_status::connecting_to_tracker:
          return "Announcing";
        case torrent_status::downloading:
          return "Downloading";
        case torrent_status::finished:
          return "Finished";
        case torrent_status::seeding:
          return "Seeding";
        case torrent_status::allocating:
          return "Allocating";
      }
    }
    else
      return "Stoped";
  }
  else
    return "Queued";
}

const Glib::ustring Torrent::get_state_string(State state) const
{
  if (state & QUEUED)
    return "Queued";
  else if (state & STOPED)
    return "Stoped";
  else if (state & CHECK_QUEUE)
    return "Queued";
  else if (state & CHECKING)
    return "Checking";
  else if (state & ANNOUNCING)
    return "Announcing";
  else if (state & DOWNLOADING)
    return "Downloading";
  else if (state & FINISHED)
    return "Finished";
  else if (state & SEEDING)
    return "Seeding";
  else if (state & ALLOCATING)
    return "Allocating";
}

const torrent_info Torrent::get_info() const
{
  return m_handle.get_torrent_info();
}

const torrent_status Torrent::get_status() const
{
  if (m_handle.is_valid())
    return m_handle.status();
  else
    return torrent_status();
}

const std::vector<partial_piece_info> Torrent::get_download_queue() const
{
  std::vector<partial_piece_info> queue;
  if (m_handle.is_valid())
    m_handle.get_download_queue(queue);
  
  return queue;
}

const std::vector<float> Torrent::get_file_progress()
{
  std::vector<float> fp;
  if (m_handle.is_valid())
    m_handle.file_progress(fp);
  
  return fp;
}
  
void Torrent::set_handle(const torrent_handle& handle)
{
  m_handle = handle;
}

void Torrent::set_group(const Glib::ustring& group)
{
  m_group = group;
  Engine::instance()->get_torrent_manager()->signal_group_changed().emit(m_hash, m_group);
}

void Torrent::set_tracker_reply(const Glib::ustring& reply)
{
  m_tracker_reply = reply;
}

void Torrent::set_position(unsigned int position)
{
  Direction direction = (position < m_position) ? DIR_UP : DIR_DOWN;
  m_position = position;
  Engine::instance()->get_torrent_manager()->set_torrent_position(m_hash, direction);
}

void Torrent::set_filter(std::vector<bool>& filter)
{
  if (filter != m_filter)
    m_filter.assign(filter.begin(), filter.end());
    
  /* TODO: Thread this? It completly freezes UI on large files.. */
  if (m_handle.is_valid())
    m_handle.filter_files(m_filter);
}

void Torrent::filter_file(const Glib::ustring& name, bool filter)
{
  torrent_info info = m_handle.get_torrent_info();
  
  /* Get the file index to i */
  unsigned int i = 0;
  for (i = 0; i < info.num_files(); i++)
    if (name == info.file_at(i).path.string())
      break;

  m_filter[i] = filter;
  
  set_filter(m_filter);
}

void Torrent::set_up_limit(int limit)
{
  m_up_limit = limit;
  if (m_handle.is_valid())
    m_handle.set_upload_limit(m_up_limit);
}

void Torrent::set_down_limit(int limit)
{
  m_down_limit = limit;
  if (m_handle.is_valid())
    m_handle.set_download_limit(m_down_limit);
}

void Torrent::start()
{
  m_time_val.assign_current_time();
  
  set_filter(m_filter);
}

void Torrent::stop()
{
  if (m_handle.is_valid())
  {
    m_handle.pause();
    m_resume.time = get_time_active();
    m_resume.downloaded += m_handle.status().total_download;
    m_resume.uploaded += m_handle.status().total_upload;
  }
}

void Torrent::queue()
{
  if (m_handle.is_valid())
    m_handle.pause();
}

void Torrent::unqueue()
{
  if (m_handle.is_valid())
    m_handle.resume();
}

bool Torrent::is_queued()
{
  return m_handle.is_paused();
}

bool Torrent::is_running()
{
  m_handle.is_valid();
}

bool Torrent::is_valid()
{
  return (m_hash != INVALID_HASH);
}

const entry Torrent::get_resume_entry(bool running)
{
  entry::dictionary_type resume_entry;
  
  /* This shouldn't happen */
  if (!m_handle.is_valid())
  	return entry();
  	
  if (!m_handle.is_paused())
    stop();

  try 
  {
    resume_entry = m_handle.write_resume_data().dict();
  }
  catch (std::exception& e)
  {
    //FIXME: std::cerr undeclared, wtf?
    g_warning(e.what());
  }
  
  resume_entry["path"] = m_save_path;
  resume_entry["position"] = m_position;
  resume_entry["running"] = running;
  resume_entry["downloaded"] = m_resume.downloaded;
  resume_entry["uploaded"] = m_resume.uploaded;
  resume_entry["time"] = m_resume.time;
  resume_entry["download-limit"] = m_down_limit;
  resume_entry["upload-limit"] = m_up_limit;
  entry::list_type el;
  el.push_back(m_handle.get_torrent_info().num_files());
  for (unsigned int i = 0; i < m_filter.size(); i++)
    if (m_filter[i])
      el.push_back(i);
  resume_entry["filter"] = el;
  resume_entry["group"] = m_group;
  
  return resume_entry;
}
