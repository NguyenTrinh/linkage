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

#include "linkage/PluginManager.hh"
#include "linkage/Engine.hh"

PluginInfo::PluginInfo(const Glib::ustring& name, const Glib::ustring& description, const Glib::ustring& file, bool loaded)
{
	m_name = name;
	m_description = description;
	m_file = file;
	m_loaded = loaded;
}

PluginInfo::~PluginInfo()
{
}

const Glib::ustring& PluginInfo::get_name()
{
	return m_name;
}

const Glib::ustring& PluginInfo::get_description()
{
	return m_description;
}

const Glib::ustring& PluginInfo::get_file()
{
	return m_file;
}

bool PluginInfo::get_loaded()
{
	return m_loaded;
}

Glib::RefPtr<PluginManager> PluginManager::create()
{
	return Glib::RefPtr<PluginManager>(new PluginManager());
}

PluginManager::PluginManager() : RefCounter<PluginManager>::RefCounter(this)
{
	/* TODO: load plugins from SettinsManger */
	Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();
	sm->signal_update_settings().connect(sigc::mem_fun(this, &PluginManager::on_settings));
	
	on_settings();
}

PluginManager::~PluginManager()
{
	/* TODO: save plugins to SettinsManger */
}

sigc::signal<void, Plugin*> PluginManager::signal_plugin_load()
{
	return m_signal_plugin_load;
}

sigc::signal<void, Plugin*> PluginManager::signal_plugin_unload()
{
	return m_signal_plugin_unload;
}

sigc::signal<void, Plugin*, Gtk::Widget*, Plugin::PluginParent> PluginManager::signal_add_widget()
{
	return m_signal_add_widget;
}
	
const PluginList& PluginManager::get_plugins()
{
	return loaded_plugins;
}

std::list<PluginInfo> PluginManager::list_plugins()
{
	std::list<PluginInfo> list;
	
	Glib::Dir dir(PLUGIN_DIR);
	for (Glib::DirIterator iter = dir.begin(); iter != dir.end(); ++iter)
	{
		Glib::ustring file = Glib::build_filename(PLUGIN_DIR, *iter);
		if (file.find(".so", file.size()-3) == Glib::ustring::npos)
			continue;
			
		Plugin* (*CreatePlugin)();
		
		Glib::Module module(file);
		if (module)
		{
			if (module.get_symbol("CreatePlugin", (void*&)CreatePlugin))
			{
				Plugin* plugin = CreatePlugin();
				
				bool loaded = is_loaded(plugin->get_name());
				PluginInfo info = PluginInfo(plugin->get_name(), plugin->get_description(), file, loaded);
				//delete plugin; needed?
				list.push_back(info);
			}
		}
		else
			std::cerr << module.get_last_error() << std::endl;
	}
	return list;
}

Glib::ustring PluginManager::get_module(const Glib::ustring& name)
{
	std::list<PluginInfo> info_list = list_plugins();
	for (std::list<PluginInfo>::iterator iter = info_list.begin();
				iter != info_list.end(); ++iter)
	{
		PluginInfo info = *iter;
		if (name == info.get_name())
			return info.get_file();
	}
}

void PluginManager::load_plugin(const Glib::ustring& file)
{
	Plugin* (*CreatePlugin)();
	
	Glib::Module module(file);
	if (module)
	{
		if (module.get_symbol("CreatePlugin", (void*&)CreatePlugin))
		{
			Plugin* plugin = CreatePlugin();
			
			if (!is_loaded(plugin->get_name()))
			{
				module.make_resident();
				loaded_plugins.push_back(plugin);
				plugin->on_load();
				m_signal_plugin_load.emit(plugin);
				plugin->signal_unloading().connect(sigc::mem_fun(this, &PluginManager::on_plugin_unloading));
				plugin->signal_add_widget().connect(sigc::mem_fun(this, &PluginManager::on_add_widget));
			}
		}
	}
}

void PluginManager::unload_plugin(Plugin* plugin)
{
	m_signal_plugin_unload.emit(plugin);
	
	for (PluginList::iterator iter = loaded_plugins.begin(); 
					iter != loaded_plugins.end(); ++iter)
	{
		Plugin* loaded_plugin = *iter;
		if (plugin->get_name() == loaded_plugin->get_name())
		{
			loaded_plugins.erase(iter);
			break;
		}
	}
	delete plugin;
}

/* This method is used to catch plugins that remove themselfs/crash/whatever */
void PluginManager::on_plugin_unloading(Plugin* plugin)
{
	if (std::find(loaded_plugins.begin(), loaded_plugins.end(), plugin) != loaded_plugins.end())
		m_signal_plugin_unload.emit(plugin);
}

bool PluginManager::is_loaded(const Glib::ustring& name)
{
	for (PluginList::iterator iter = loaded_plugins.begin(); 
					iter != loaded_plugins.end(); ++iter)
	{
		Plugin* loaded_plugin = *iter;
		if (name == loaded_plugin->get_name())
			return true;
	}
	return false;
}

void PluginManager::on_add_widget(Plugin* plugin, Gtk::Widget* widget, Plugin::PluginParent parent)
{
	m_signal_add_widget.emit(plugin, widget, parent);
}

void PluginManager::on_settings()
{
	std::list<Glib::ustring> plugins;

 	plugins = Engine::get_settings_manager()->get_string_list("UI", "Plugins");

	/* Load all new */
	for (std::list<Glib::ustring>::iterator iter = plugins.begin();
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
	for (PluginList::iterator piter = loaded_plugins.begin(); 
					piter != loaded_plugins.end(); ++piter)
	{
		Plugin* loaded_plugin = *piter;
		Plugin* unload = loaded_plugin;
		for (std::list<Glib::ustring>::iterator iter = plugins.begin();
					iter != plugins.end(); ++iter)
		{
			Glib::ustring name = *iter;
			if (name == loaded_plugin->get_name())
				unload = 0;
		}
		if (unload)
			unload_list.push_back(unload);
	}
	
	for (PluginList::iterator piter = unload_list.begin(); 
					piter != unload_list.end(); ++piter)
	{
		unload_plugin(*piter);
	}
}

