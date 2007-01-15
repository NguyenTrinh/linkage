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
#include "linkage/RefCounter.hh"

typedef std::list<Plugin*> PluginList;

class PluginInfo
{
  Glib::ustring m_name, m_description, m_file;
  bool m_loaded;
  
public:
  const Glib::ustring& get_name();
  const Glib::ustring& get_description();
  const Glib::ustring& get_file();
  bool get_loaded();
  
  PluginInfo(const Glib::ustring& name, const Glib::ustring& description, const Glib::ustring& file, bool loaded);
  ~PluginInfo();
};

class PluginManager : public RefCounter<PluginManager>
{ 
  PluginList loaded_plugins;
  
  void load_plugin(const Glib::ustring& file);
  void unload_plugin(Plugin* plugin);
  
  bool is_loaded(const Glib::ustring& name);
  
  Glib::ustring get_module(const Glib::ustring& name);
  
  sigc::signal<void, Plugin*> m_signal_plugin_load;
  sigc::signal<void, Plugin*> m_signal_plugin_unload;
  
  void on_plugin_unloading(Plugin* plugin);
  
  void on_add_widget(Plugin* plugin, Gtk::Widget* widget, Plugin::PluginParent parent);

  void on_settings();
  
  sigc::signal<void, Plugin*, Gtk::Widget*, Plugin::PluginParent> m_signal_add_widget;
  
  PluginManager();
  
public:
  const PluginList& get_plugins();
  
  sigc::signal<void, Plugin*> signal_plugin_load();
  sigc::signal<void, Plugin*> signal_plugin_unload();
  
  std::list<PluginInfo> list_plugins();
  
  sigc::signal<void, Plugin*, Gtk::Widget*, Plugin::PluginParent> signal_add_widget();
  
	static Glib::RefPtr<PluginManager> create();
  ~PluginManager();
};
  
#endif /* PLUGIN_MANAGER_HH */
