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

#ifndef PLUGIN_TRAYICON_HH
#define PLUGIN_TRAYICON_HH

#include <glibmm/object.h>
#include <gdkmm/types.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/image.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/menu.h>
#include <gtkmm/stock.h>

#include "eggtrayicon.h"
#include "linkage/Plugin.hh"

class TrayPlugin : public Plugin
{
  EggTrayIcon *tray_icon;
  Gtk::Widget* widget;
  GtkWidget* gobj;
  Gtk::EventBox* event_box;
  Glib::RefPtr<Gdk::Pixbuf> pixbuf;
  Gtk::Image* image;
  Gtk::Menu* popup_menu;
  
  bool on_button_released(GdkEventButton* e);
  
public:
  Glib::ustring get_name();
  Glib::ustring get_description();

  void on_load();
  
  bool update(Torrent* torrent);
  
  bool on_notify(const Glib::ustring& title,
                const Glib::ustring& message,
                NotifyType type,
                Torrent* torrent);
  
  TrayPlugin();
  ~TrayPlugin();
};

#endif /* PLUGIN_TRAYICON_HH */
