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
#include <iostream>
#include "PluginTrayIcon.hh"

#include <gtkmm/menuitem.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/separatormenuitem.h>

TrayPlugin::TrayPlugin()
{
}

TrayPlugin::~TrayPlugin()
{
  delete popup_menu;
  delete image;
  delete event_box;
}

Glib::ustring TrayPlugin::get_name()
{
  return "TrayPlugin";
}

Glib::ustring TrayPlugin::get_description()
{
  return "Displays a tray icon";
}
  
void TrayPlugin::on_load()
{
  Glib::ustring icon = PIXMAP_DIR;
  icon = icon + "/linkage.png";
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file(icon, 24, 24, true);
  image = new Gtk::Image(pixbuf);
  
  event_box = new Gtk::EventBox();;
  event_box->add_events(Gdk::BUTTON_RELEASE_MASK);
  event_box->signal_button_release_event().connect(sigc::mem_fun(this, &TrayPlugin::on_button_released));
  event_box->add(*image);
  event_box->show_all_children();
  
  Gtk::Widget* widget = dynamic_cast<Gtk::Widget*>(event_box);
  GtkWidget* gobj = widget->gobj();

  tray_icon = egg_tray_icon_new ("TrayPlugin");
  gtk_container_add (GTK_CONTAINER (tray_icon), gobj);
  gtk_widget_show_all (GTK_WIDGET (tray_icon));
  
  popup_menu = new Gtk::Menu();
  Gtk::MenuItem* item = Gtk::manage(new Gtk::MenuItem("Start torrents"));
  popup_menu->append(*item);
  item = Gtk::manage(new Gtk::MenuItem("Stop torrents"));
  popup_menu->append(*item);
  Gtk::SeparatorMenuItem* separator = Gtk::manage(new Gtk::SeparatorMenuItem());
  popup_menu->append(*separator);
  Gtk::ImageMenuItem* imageitem = Gtk::manage(new Gtk::ImageMenuItem(Gtk::Stock::QUIT));
  popup_menu->append(*imageitem);
  
  popup_menu->show_all_children();
}

Plugin::PluginParent TrayPlugin::get_parent()
{
  return Plugin::PARENT_NONE;
}

Gtk::Widget* TrayPlugin::get_widget()
{
  return 0;
}

bool TrayPlugin::update(Torrent* torrent)
{
  return false;
}

bool TrayPlugin::on_notify(const Glib::ustring& title,
                          const Glib::ustring& message,
                          NotifyType type,
                          Torrent* torrent)
{
  return false;
}

bool TrayPlugin::on_button_released(GdkEventButton* e)
{
  if (e->button == 1)
  {
    signal_ui_toggle_visible_.emit();
  }
  else if (e->button == 3)
  {
    popup_menu->popup(e->button, e->time);
  }
}

Plugin * CreatePlugin()
{
   return new TrayPlugin();
}
