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

/*g++ -shared notifications.cpp -o notify_plugin.so `pkg-config gtkmm-2.4 --libs --cflags` `pkg-config libtorrent --libs --cflags` `pkg-config libnotify --libs --cflags`*/

#ifndef PLUGIN_NOTIFICATION_HH
#define PLUGIN_NOTIFICATION_HH

#include "linkage/Plugin.hh"

#include <libnotify/notify.h>
#include <glib.h>

int notify_send(const char *summary, const char *body);

class NotifyPlugin : public Plugin
{
public:
  Glib::ustring get_name();
  Glib::ustring get_description();

  void on_load();
  
  bool update(Torrent* torrent);
  
  bool on_notify(const Glib::ustring& title,
                const Glib::ustring& message,
                NotifyType type,
                Torrent* torrent);

  NotifyPlugin();
  ~NotifyPlugin();
};

#endif /* PLUGIN_NOTIFICATION_HH */
