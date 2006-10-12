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

#ifndef PLUGIN_MANAGER_HH
#define PLUGIN_MANAGER_HH

#include <list>

#include "linkage/Plugin.hh"

typedef std::list<Plugin*> PluginList;

class PluginInfo
{
  Glib::ustring name_, description_, file_;
  bool loaded_;
  
public:
  const Glib::ustring& get_name();
  const Glib::ustring& get_description();
  const Glib::ustring& get_file();
  bool get_loaded();
  
  PluginInfo(const Glib::ustring& name, const Glib::ustring& description, const Glib::ustring& file, bool loaded);
  ~PluginInfo();
};

class PluginManager
{
  static PluginManager* smInstance;
  
  PluginList loaded_plugins;
  
  void load_plugin(const Glib::ustring& file);
  void unload_plugin(Plugin* plugin);
  
  bool is_loaded(const Glib::ustring& name);
  
  Glib::ustring get_module(const Glib::ustring& name);
  
  sigc::signal<void, Plugin*> signal_plugin_load_;
  sigc::signal<void, Plugin*> signal_plugin_unload_;
  
  bool on_plugin_unloading(Plugin* plugin);
  void on_stop_torrent(Torrent* torrent);
  void on_start_torrent(Torrent* torrent);
  void on_add_torrent(Glib::ustring file);
  void on_remove_torrent(Torrent* torrent);
  bool on_ui_toggle_visible();
  void on_quit();
  
  void on_settings();
  
  sigc::signal<void, const Glib::ustring&> signal_add_torrent_;
  sigc::signal<bool> signal_ui_toggle_visible_;
  sigc::signal<void> signal_quit_;
  
public:
  static PluginManager* instance();
  static void goodnight();

  PluginList get_plugins();
  
  sigc::signal<void, Plugin*> signal_plugin_load();
  sigc::signal<void, Plugin*> signal_plugin_unload();
  
  std::list<PluginInfo> list_plugins();
  
  sigc::signal<void, const Glib::ustring&> signal_add_torrent();
  sigc::signal<bool> signal_ui_toggle_visible();
  sigc::signal<void> signal_quit();
  
  PluginManager();
  ~PluginManager();
};
  
#endif /* PLUGIN_MANAGER_HH */
