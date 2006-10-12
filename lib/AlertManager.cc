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

#include <glibmm/main.h>

#include "libtorrent/alert_types.hpp"

#include "linkage/AlertManager.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/SessionManager.hh"

AlertManager* AlertManager::smInstance = NULL;

AlertManager* AlertManager::instance()
{
  static bool running = false;
  if (smInstance == NULL && running == false)
  {
    running = true;
    smInstance = new AlertManager();
    running = false;
  }
  return smInstance;
}

void AlertManager::goodnight()
{
  if (smInstance != NULL)
  {
    delete smInstance;
  }
}

AlertManager::AlertManager()
{
  int interval = SettingsManager::instance()->get<int>("UI", "Interval")*1000;
  Glib::signal_timeout().connect(sigc::mem_fun(this, &AlertManager::check_alerts), interval);
}

AlertManager::~AlertManager()
{
}

sigc::signal<void, const Glib::ustring&> 
AlertManager::signal_listen_failed()
{
  return signal_listen_failed_;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&, int, int> 
AlertManager::signal_tracker_failed()
{
  return signal_tracker_failed_;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&> 
AlertManager::signal_tracker_reply()
{
  return signal_tracker_reply_;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&> 
AlertManager::signal_tracker_warning()
{
  return signal_tracker_warning_;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&> 
AlertManager::signal_tracker_announce()
{
  return signal_tracker_announce_;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&> 
AlertManager::signal_torrent_finished()
{
  return signal_torrent_finished_;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&> 
AlertManager::signal_file_error()
{
  return signal_file_error_;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&> AlertManager::signal_fastresume_rejected()
{
  return signal_fastresume_rejected_;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&, int> 
AlertManager::signal_hash_failed()
{
  return signal_hash_failed_;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&, const Glib::ustring&> 
AlertManager::signal_peer_ban()
{
  return signal_peer_ban_;
}

  
bool AlertManager::check_alerts()
{
  std::auto_ptr<alert> a;
  a = SessionManager::instance()->pop_alert();
  while (a.get())
  {
    #ifdef DEBUG
      std::cerr << "Alert: " << a->msg() << std::endl;
    #endif
    if (listen_failed_alert* p = dynamic_cast<listen_failed_alert*>(a.get()))
    {
      signal_listen_failed_.emit(p->msg());
    }
    else if (tracker_alert* p = dynamic_cast<tracker_alert*>(a.get()))
    {
      signal_tracker_failed_.emit(p->handle.info_hash(), p->msg(), p->status_code, p->times_in_row);
    }
    else if (tracker_reply_alert* p = dynamic_cast<tracker_reply_alert*>(a.get()))
    {
      signal_tracker_reply_.emit(p->handle.info_hash(), p->msg());
    }
    else if (tracker_warning_alert* p = dynamic_cast<tracker_warning_alert*>(a.get()))
    {
      signal_tracker_warning_.emit(p->handle.info_hash(), p->msg());
    }
    else if (tracker_announce_alert* p = dynamic_cast<tracker_announce_alert*>(a.get()))
    {
      signal_tracker_announce_.emit(p->handle.info_hash(), p->msg());
    }
    else if (torrent_finished_alert* p = dynamic_cast<torrent_finished_alert*>(a.get()))
    {
      signal_torrent_finished_.emit(p->handle.info_hash(), p->msg());
    }
    else if (file_error_alert* p = dynamic_cast<file_error_alert*>(a.get()))
    {
      signal_file_error_.emit(p->handle.info_hash(), p->msg());
    }
    else if (fastresume_rejected_alert* p = dynamic_cast<fastresume_rejected_alert*>(a.get()))
    {
      signal_fastresume_rejected_.emit(p->handle.info_hash(), p->msg());
    }
    else if (hash_failed_alert* p = dynamic_cast<hash_failed_alert*>(a.get()))
    {
      signal_hash_failed_.emit(p->handle.info_hash(), p->msg(), p->piece_index);
    }
    else if (peer_ban_alert* p = dynamic_cast<peer_ban_alert*>(a.get()))
    {
      signal_peer_ban_.emit(p->handle.info_hash(), p->msg(), p->ip.address().to_string());
    }
    a = SessionManager::instance()->pop_alert();
  }
  return true;
}
