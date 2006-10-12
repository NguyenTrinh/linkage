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

#include "linkage/PluginManager.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/SessionManager.hh"

PluginInfo::PluginInfo(const Glib::ustring& name, const Glib::ustring& description, const Glib::ustring& file, bool loaded)
{
  name_ = name;
  description_ = description;
  file_ = file;
  loaded_ = loaded;
}

PluginInfo::~PluginInfo()
{
}

const Glib::ustring& PluginInfo::get_name()
{
  return name_;
}

const Glib::ustring& PluginInfo::get_description()
{
  return description_;
}

const Glib::ustring& PluginInfo::get_file()
{
  return file_;
}

bool PluginInfo::get_loaded()
{
  return loaded_;
}

PluginManager* PluginManager::smInstance = NULL;

PluginManager* PluginManager::instance()
{
  static bool running = false;
  if (smInstance == NULL && running == false)
  {
    running = true;
    smInstance = new PluginManager();
    running = false;
  }
  return smInstance;
}
                                                 
void PluginManager::goodnight()
{
  if (smInstance != NULL)
  {
    delete smInstance;
  }
}

PluginManager::PluginManager()
{
  /* TODO: load plugins from SettinsManger */
  SettingsManager::instance()->signal_update_settings().connect(sigc::mem_fun(this, &PluginManager::on_settings));
  
  on_settings();
}

PluginManager::~PluginManager()
{
  /* TODO: save plugins to SettinsManger */
}

sigc::signal<void, Plugin*> PluginManager::signal_plugin_load()
{
  return signal_plugin_load_;
}

sigc::signal<void, Plugin*> PluginManager::signal_plugin_unload()
{
  return signal_plugin_unload_;
}

sigc::signal<void, Plugin*, Gtk::Widget*, Plugin::PluginParent> PluginManager::signal_add_widget()
{
  return signal_add_widget_;
}

sigc::signal<void, const Glib::ustring&> PluginManager::signal_add_torrent()
{
  return signal_add_torrent_;
}

sigc::signal<bool> PluginManager::signal_ui_toggle_visible()
{
  return signal_ui_toggle_visible_;
}

sigc::signal<void> PluginManager::signal_quit()
{
  return signal_quit_;
}
  
PluginList PluginManager::get_plugins()
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
        signal_plugin_load_.emit(plugin);
        plugin->signal_unloading().connect(sigc::mem_fun(this, &PluginManager::on_plugin_unloading));
        plugin->signal_add_widget().connect(sigc::mem_fun(this, &PluginManager::on_add_widget));
        plugin->signal_add_torrent().connect(sigc::mem_fun(this, &PluginManager::on_add_torrent));
        plugin->signal_ui_toggle_visible().connect(sigc::mem_fun(this, &PluginManager::on_ui_toggle_visible));
        plugin->signal_quit().connect(sigc::mem_fun(this, &PluginManager::on_quit));
      }
    }
  }
}

void PluginManager::unload_plugin(Plugin* plugin)
{
  signal_plugin_unload_.emit(plugin);
  
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

bool PluginManager::on_plugin_unloading(Plugin* plugin)
{
  return !is_loaded(plugin->get_name());
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
  signal_add_widget_.emit(plugin, widget, parent);
}

void PluginManager::on_add_torrent(Glib::ustring file)
{
  signal_add_torrent_.emit(file);
}

bool PluginManager::on_ui_toggle_visible()
{
  bool ret = signal_ui_toggle_visible_.emit();
  return ret;
}

void PluginManager::on_quit()
{
  signal_quit_.emit();
}

void PluginManager::on_settings()
{
  std::list<Glib::ustring> plugins = SettingsManager::instance()->get<UStringArray>("UI", "Plugins");
  
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

