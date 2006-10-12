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

#include "PluginNotification.hh"


int notify_send(const char *summary, const char *body)
{
  NotifyNotification *notify;
  static const gchar *type = NULL;
  static gchar *icon_str = NULL;
  static glong expire_timeout = NOTIFY_EXPIRES_DEFAULT;
  static NotifyUrgency urgency = NOTIFY_URGENCY_NORMAL;
  
  if (!notify_init("linkage"))
    return 0;

  notify = notify_notification_new(summary, body, icon_str, NULL);
  notify_notification_set_category(notify, type);
  notify_notification_set_urgency(notify, urgency);
  notify_notification_set_timeout(notify, expire_timeout);
  
  notify_notification_show(notify, NULL);
  
  g_object_unref(G_OBJECT(notify));

  notify_uninit();
  
  return 1;
}

NotifyPlugin::NotifyPlugin()
{
}

NotifyPlugin::~NotifyPlugin()
{
}

Glib::ustring NotifyPlugin::get_name()
{
  return "NotifyPlugin";
}

Glib::ustring NotifyPlugin::get_description()
{
  return "Displays notifications trough libnotify";
}
  
void NotifyPlugin::on_load()
{
}

Plugin::PluginParent NotifyPlugin::get_parent()
{
  return Plugin::PARENT_NONE;
}

Gtk::Widget* NotifyPlugin::get_widget()
{
  return 0;
}

bool NotifyPlugin::update(Torrent* torrent)
{
  return false;
}

bool NotifyPlugin::on_notify(const Glib::ustring& title,
                          const Glib::ustring& message,
                          NotifyType type,
                          Torrent* torrent)
{
  //FIXME: add NotifyType support
  return notify_send(title.c_str(), message.c_str());
}

Plugin * CreatePlugin()
{
   return new NotifyPlugin();
}

