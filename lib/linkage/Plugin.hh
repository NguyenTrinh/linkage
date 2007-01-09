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
public:
  enum PluginParent { PARENT_NONE, PARENT_MAIN, PARENT_DETAILS, PARENT_MENU, PARENT_TOOLBAR };
  
  sigc::signal<bool, Plugin*> signal_unloading();
  
  sigc::signal<void, Glib::ustring> signal_add_torrent();
  sigc::signal<bool> signal_ui_toggle_visible();
  sigc::signal<void> signal_quit();
  
  sigc::signal<void, Plugin*, Gtk::Widget*, PluginParent> signal_add_widget();
  
private:  
  sigc::signal<bool, Plugin*> m_signal_unloading;
  
  sigc::signal<void, Plugin*, Gtk::Widget*, PluginParent> m_signal_add_widget;

protected:
	Glib::ustring m_name, m_description;
  int m_version;
  void add_widget(Gtk::Widget* widget, PluginParent parent);
  
public:
	/* FIXME: Does everything need to be pure virtual? */
	
  /*Returns the name of the plugin that will be displayed in the UI*/
  virtual Glib::ustring get_name();
  /*Returns a description of the plugin that will be displayed in the settings UI*/
  virtual Glib::ustring get_description();
  /*Returns the version of the plugin that will be displayed in the settings UI*/
  virtual int get_version();
  
  //FIXME: thing below has to be pure virtual
  
  /*This is called immedeately after the plugin is loaded, 
    put initialization stuff here not in the constructor*/
  virtual void on_load() = 0;
  
  /*Returns the parent if plugin has a widget to pack in to main win*/
  // FIXME: Do a signal_add_widget() instead, in case the plugin wants more then one widget
  virtual PluginParent get_parent() = 0;
  virtual Gtk::Widget* get_widget() = 0;
  
  /*Returns true if widget was updated, this is called every n second,
    where n is Settings::update_interval, if the widget is visible*/
  virtual bool update(Torrent& torrent) = 0;
  
  /* FIXME: on_notify should be removed, connect to individual signals from 
  					AlertManager instead and remove all plugin crap from UI */
  /*Returns true if the notifications was displayed, if false the notification
    will be displayed using the standard method*/
  virtual bool on_notify(const Glib::ustring& title,
                         const Glib::ustring& message,
                         NotifyType type,
                         Torrent& torrent) = 0;
                         
  Plugin();
  Plugin(const Glib::ustring& name, const Glib::ustring& description, int version);
  virtual ~Plugin();
};

/* Glib::module needs standard C names */
#ifdef __cplusplus
extern "C" {
#endif

/* This is the plugin function each plugin implements for the main app to
   create an instance of the plugin child class. */
extern Plugin* CreatePlugin();

#ifdef __cplusplus
}
#endif

#endif /* PLUGIN_HH */
