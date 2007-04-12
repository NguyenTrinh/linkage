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

PluginInfo::PluginInfo(const Glib::ustring& name,
												const Glib::ustring& description,
												const Glib::ustring& author,
												const Glib::ustring& website,
												const Glib::ustring& version,
												const Glib::ustring& file,
												bool loaded)
{
	m_name = name;
	m_description = description;
	m_author = author;
	m_website = website;
	m_version = version;
	m_file = file;
	m_loaded = loaded;
}

PluginInfo::~PluginInfo()
{
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
	for (std::list<Plugin*>::iterator iter = loaded_plugins.begin(); 
				iter != loaded_plugins.end(); ++iter)
	{
		delete *iter;
	}
	loaded_plugins.clear();
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

PluginManager::PluginList PluginManager::get_plugins()
{
	PluginList list;
	for (std::list<Plugin*>::iterator iter = loaded_plugins.begin(); 
				iter != loaded_plugins.end(); ++iter)
	{
		list.push_back(WeakPtr<Plugin>(*iter));
	}
	return list;
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
				PluginInfo info = PluginInfo(plugin->get_name(),
																			plugin->get_description(),
																			plugin->get_author(),
																			plugin->get_website(),
																			plugin->get_version(),
																			file,
																			loaded);
				delete plugin;
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
	return "";
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
	std::list<Plugin*>::iterator iter = std::find(loaded_plugins.begin(), loaded_plugins.end(), plugin);
	if (iter != loaded_plugins.end())
	{
		m_signal_plugin_unload.emit(plugin);
		loaded_plugins.erase(iter);
		delete plugin;
	}
}

/* This method is used to catch plugins that remove themselfs/crash/whatever */
void PluginManager::on_plugin_unloading(Plugin* plugin)
{
	if (std::find(loaded_plugins.begin(), loaded_plugins.end(), plugin) != loaded_plugins.end())
		m_signal_plugin_unload.emit(plugin);
}

bool PluginManager::is_loaded(const Glib::ustring& name)
{
	for (std::list<Plugin*>::iterator iter = loaded_plugins.begin(); 
					iter != loaded_plugins.end(); ++iter)
	{
		Plugin* plugin = *iter;
		if (name == plugin->get_name())
			return true;
	}
	return false;
}

WeakPtr<Plugin> PluginManager::get_plugin(const Glib::ustring& name)
{
	for (std::list<Plugin*>::iterator iter = loaded_plugins.begin(); 
					iter != loaded_plugins.end(); ++iter)
	{
		Plugin* plugin = *iter;
		if (name == plugin->get_name())
			return WeakPtr<Plugin>(plugin);
	}
	return  WeakPtr<Plugin>();
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
	std::list<Plugin*> unload_list;
	for (std::list<Plugin*>::iterator piter = loaded_plugins.begin(); 
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
	
	for (std::list<Plugin*>::iterator piter = unload_list.begin(); 
					piter != unload_list.end(); ++piter)
	{
		unload_plugin(*piter);
	}
}

