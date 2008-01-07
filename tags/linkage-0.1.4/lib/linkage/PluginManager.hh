/*
Copyright (C) 2006	Christian Lundgren

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA	02110-1301, USA.
*/

#ifndef PLUGIN_MANAGER_HH
#define PLUGIN_MANAGER_HH

#include <list>

#include "linkage/Plugin.hh"
#include "linkage/WeakPtr.hh"
#include "linkage/RefCounter.hh"

class PluginManager : public RefCounter<PluginManager>
{
public:
	struct PluginInfo : public Plugin::Info
	{
		Glib::ustring file;
		bool loaded;
	};
	typedef std::list<PluginInfo> PluginInfoList;

private:
	typedef std::list<Plugin*> PluginPtrList;

	PluginPtrList m_plugins;
	PluginInfoList m_info;

	void refresh_info();

	void load_plugin(const Glib::ustring& file);
	void unload_plugin(Plugin* plugin, bool destroy = true);
	
	Glib::ustring get_module(const Glib::ustring& name);
	
	sigc::signal<void, WeakPtr<Plugin> > m_signal_plugin_load;
	sigc::signal<void, WeakPtr<Plugin> > m_signal_plugin_unload;

	void on_settings();
	
	PluginManager();
	
public:
	typedef std::list<WeakPtr<Plugin> > PluginList;
	PluginList get_plugins();
	
	sigc::signal<void, WeakPtr<Plugin> > signal_plugin_load();
	sigc::signal<void, WeakPtr<Plugin> > signal_plugin_unload();

	bool is_loaded(const Glib::ustring& name);
	WeakPtr<Plugin> get_plugin(const Glib::ustring& name);

	PluginInfoList list_plugins();

	static Glib::RefPtr<PluginManager> create();
	~PluginManager();
};
	
#endif /* PLUGIN_MANAGER_HH */