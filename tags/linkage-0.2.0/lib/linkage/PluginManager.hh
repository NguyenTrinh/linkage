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

#include <boost/smart_ptr.hpp>

#include "libtorrent/intrusive_ptr_base.hpp"

#include "linkage/Plugin.hh"

namespace Linkage
{
class PluginManager;
typedef boost::intrusive_ptr<PluginManager> PluginManagerPtr;

class Value;

class PluginManager : public libtorrent::intrusive_ptr_base<PluginManager>
{
public:
	struct PluginInfo : public Plugin::Info
	{
		Glib::ustring file;
		bool loaded;
	};
	typedef std::list<PluginInfo> PluginInfoList;
	typedef std::list<PluginPtr > PluginList;

private:
	PluginList m_plugins;
	PluginInfoList m_info;

	void refresh_info();

	void load_plugin(const Glib::ustring& file);
	void unload_plugin(const PluginPtr& plugin);
	void on_plugin_destroyed(Plugin* plugin);
	
	Glib::ustring get_module(const Glib::ustring& name);
	
	sigc::signal<void, PluginPtr> m_signal_plugin_load;
	sigc::signal<void, PluginPtr> m_signal_plugin_unload;

	void on_plugins_changed(const Value& value);
	void update_plugins(const std::list<Glib::ustring>& plugins);

	PluginManager();
	
public:
	const PluginList& get_plugins();
	
	sigc::signal<void, PluginPtr> signal_plugin_load();
	sigc::signal<void, PluginPtr> signal_plugin_unload();

	bool is_loaded(const Glib::ustring& name);
	PluginPtr get_plugin(const Glib::ustring& name);

	PluginInfoList list_plugins();

	static PluginManagerPtr create();
	~PluginManager();
};

} /* namespace */
	
#endif /* PLUGIN_MANAGER_HH */
