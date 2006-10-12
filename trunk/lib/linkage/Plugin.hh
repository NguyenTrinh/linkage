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

#ifndef PLUGIN_HH
#define PLUGIN_HH

#include <list>
#include <gtkmm/widget.h>
#include "linkage/Torrent.hh"

enum NotifyType { NOTIFY_INFO, NOTIFY_WARNING, NOTIFY_ERROR };

class Plugin
{
  sigc::signal<HashList> signal_get_hashlist_;
  sigc::signal<Torrent*, sha1_hash> signal_get_torrent_;

protected:  
  Glib::ustring name_, description_;
  int version_;
  
  sigc::signal<bool, Plugin*> signal_unloading_;
  
  sigc::signal<void, Torrent*> signal_stop_torrent_;
  sigc::signal<void, Torrent*> signal_start_torrent_;
  sigc::signal<void, Glib::ustring> signal_add_torrent_;
  sigc::signal<void, Torrent*> signal_remove_torrent_;
  sigc::signal<bool> signal_ui_toggle_visible_;
  sigc::signal<void> signal_quit_;
  //FIXME: Implement these, shouldn't be virtuals
  //std::list<Torrent*> list_torrents();
  //Torrent* get_torrent(sha1_hash hash);
  
public:
  virtual sigc::signal<bool, Plugin*> signal_unloading();
  virtual sigc::signal<void, Torrent*> signal_stop_torrent();
  virtual sigc::signal<void, Torrent*> signal_start_torrent();
  virtual sigc::signal<void, Glib::ustring> signal_add_torrent();
  virtual sigc::signal<void, Torrent*> signal_remove_torrent();
  virtual sigc::signal<bool> signal_ui_toggle_visible();
  virtual sigc::signal<void> signal_quit();
  
  enum PluginParent { PARENT_NONE, PARENT_MAIN, PARENT_DETAILS, PARENT_MENU, PARENT_TOOLBAR };
  
  /*Returns the name of the plugin that will be displayed in the UI*/
  virtual Glib::ustring get_name();
  /*Returns a description of the plugin that will be displayed in the settings UI*/
  virtual Glib::ustring get_description();
  /*Returns the version of the plugin that will be displayed in the settings UI*/
  virtual int get_version();
  
  /*This is called immedeately after the plugin is loaded, 
    put initialization stuff here not in the constructor*/
  virtual void on_load();
  
  /*Returns the parent if plugin has a widget to pack in to main win*/
  // FIXME: Do a signal_add_widget() instead, in case the plugin wants more then one widget
  virtual PluginParent get_parent();
  virtual Gtk::Widget* get_widget();
  
  /*Returns true if widget was updated, this is called every n second,
    where n is Settings::update_interval, if the widget is visible*/
  virtual bool update(Torrent* torrent);
  
  /*Returns true if the notifications was displayed, if false the notification
    will be displayed using the standard method*/
  virtual bool on_notify(const Glib::ustring& title,
                         const Glib::ustring& message,
                         NotifyType type,
                         Torrent* torrent);
                         
  Plugin();
  Plugin(const Glib::ustring& name, const Glib::ustring& description, int version);
  virtual ~Plugin();
};

/* Glib::module needs standard C names */
#ifdef __cplusplus
extern "C" {
#endif

/* This is the plugin function each plugin overrides for the main app to
   create an instance of the plugin child class. */
extern Plugin* CreatePlugin();

#ifdef __cplusplus
}
#endif

#endif /* PLUGIN_HH */
