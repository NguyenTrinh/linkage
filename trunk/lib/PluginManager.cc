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

#include <glibmm/i18n.h>

#include "config.h"

#include "linkage/PluginManager.hh"
#include "linkage/Engine.hh"
#include "linkage/SettingsManager.hh"

Glib::RefPtr<PluginManager> PluginManager::create()
{
	return Glib::RefPtr<PluginManager>(new PluginManager());
}

PluginManager::PluginManager()
{
	refresh_info();

	Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();
	sm->signal("ui/plugins").connect(sigc::mem_fun(this, &PluginManager::on_plugins_changed));

	std::list<Glib::ustring> plugins = sm->get_string_list("ui/plugins");
	update_plugins(plugins);
}

PluginManager::~PluginManager()
{
	//m_plugins.clear();
}

sigc::signal<void, Glib::RefPtr<Plugin> > PluginManager::signal_plugin_load()
{
	return m_signal_plugin_load;
}

sigc::signal<void, Glib::RefPtr<Plugin> > PluginManager::signal_plugin_unload()
{
	return m_signal_plugin_unload;
}

void PluginManager::refresh_info()
{
	m_info.clear();

	Glib::Dir dir(PLUGIN_DIR);
	for (Glib::DirIterator iter = dir.begin(); iter != dir.end(); ++iter)
	{
		Glib::ustring file = Glib::build_filename(PLUGIN_DIR, *iter);
		if (!Glib::str_has_suffix(file, ".so"))
			continue;

		Plugin::Info (*plugin_info)();
		
		Glib::Module module(file);
		if (module)
		{
			if (module.get_symbol("plugin_info", (void*&)plugin_info))
			{
				Plugin::Info p_info = plugin_info();
				PluginInfo info = *static_cast<PluginInfo*>(&p_info);

				info.loaded = is_loaded(info.name);
				info.file = file;

				m_info.push_back(info);
			}
		}
		else
			g_warning(_("Failed to open plugin %s"), module.get_last_error().c_str());
	}
}

const PluginManager::PluginList& PluginManager::get_plugins()
{
	return m_plugins;
}

PluginManager::PluginInfoList PluginManager::list_plugins()
{
	return m_info;
}

Glib::ustring PluginManager::get_module(const Glib::ustring& name)
{
	for (PluginInfoList::iterator iter = m_info.begin(); iter != m_info.end(); ++iter)
	{
		PluginInfo info = *iter;
		if (name == info.name)
			return info.file;
	}
	return "";
}

void PluginManager::load_plugin(const Glib::ustring& file)
{
	Plugin* (*create_plugin)();

	Glib::Module module(file);
	if (module)
	{
		if (module.get_symbol("create_plugin", (void*&)create_plugin))
		{
			Glib::RefPtr<Plugin> plugin(create_plugin());

			Glib::ustring name = plugin->get_info().name;
			if (!is_loaded(name))
			{
				module.make_resident();
				m_plugins.push_back(plugin);
				for (PluginInfoList::iterator info_iter = m_info.begin(); info_iter != m_info.end(); ++info_iter)
				{
					PluginInfo& info = *info_iter;
					if (name == info.name)
						info.loaded = true;
				}
				m_signal_plugin_load.emit(plugin);
			}
		}
	}
}

void PluginManager::unload_plugin(const Glib::RefPtr<Plugin>& plugin)
{
	PluginList::iterator iter = std::find(m_plugins.begin(), m_plugins.end(), plugin);
	if (iter != m_plugins.end())
	{
		Glib::ustring name = plugin->get_info().name;

		m_signal_plugin_unload.emit(plugin);
		m_plugins.erase(iter);

		for (PluginInfoList::iterator info_iter = m_info.begin(); info_iter != m_info.end(); ++info_iter)
		{
			PluginInfo& info = *info_iter;
			if (name == info.name)
				info.loaded = false;
		}
	}
}

bool PluginManager::is_loaded(const Glib::ustring& name)
{
	for (PluginInfoList::iterator info_iter = m_info.begin(); info_iter != m_info.end(); ++info_iter)
	{
		PluginInfo info = *info_iter;
		if (name == info.name)
			return info.loaded;
	}
	return false;
}

Glib::RefPtr<Plugin> PluginManager::get_plugin(const Glib::ustring& name)
{
	for (PluginList::iterator iter = m_plugins.begin(); iter != m_plugins.end(); ++iter)
	{
		Glib::RefPtr<Plugin> plugin = *iter;
		if (name == plugin->get_info().name)
			return plugin;
	}
	return Glib::RefPtr<Plugin>();
}

void PluginManager::on_plugins_changed(const Value& value)
{
	std::list<Glib::ustring> plugins = value.get_string_list();
	update_plugins(plugins);
}

void PluginManager::update_plugins(const std::list<Glib::ustring>& plugins)
{
	/* Load all new */
	for (std::list<Glib::ustring>::const_iterator iter = plugins.begin();
		iter != plugins.end(); ++iter)
	{
		Glib::ustring name = *iter;
		if (!is_loaded(name))
		{
			Glib::ustring file = get_module(name);
			load_plugin(file);
		}
	}

	/* Unload all old */
	PluginList unload_list;
	for (PluginList::iterator iter = m_plugins.begin(); iter != m_plugins.end(); ++iter)
	{
		Glib::RefPtr<Plugin> plugin = *iter;
		std::list<Glib::ustring>::const_iterator search;
		search = std::find(plugins.begin(), plugins.end(), plugin->get_info().name);
		if (search == plugins.end())
			unload_list.push_back(plugin);
	}
	
	for (PluginList::iterator iter = unload_list.begin(); iter != unload_list.end(); ++iter)
	{
		unload_plugin(*iter);
	}
}

