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

#include "linkage/Plugin.hh"
#include "linkage/Utils.hh"

Plugin::Plugin()
{
  name_ = "Generic plugin";
  description_ = "Generic plugin description";
  version_ = 0;
}

Plugin::Plugin(const Glib::ustring& name, const Glib::ustring& description, int version)
{
  name_ = name;
  description_ = description;
  version_ = version;
}

Plugin::~Plugin()
{
}

sigc::signal<void, Plugin*, Gtk::Widget*, Plugin::PluginParent> Plugin::signal_add_widget()
{
  return signal_add_widget_;
}

void Plugin::add_widget(Gtk::Widget* widget, Plugin::PluginParent parent)
{
  signal_add_widget_.emit(this, widget, parent);
}

bool Plugin::ui_toggle_visible()
{
  return signal_ui_toggle_visible_.emit();
}

void Plugin::add_torrent(const Glib::ustring& file)
{
  signal_add_torrent_.emit(file);
}

sigc::signal<bool, Plugin*> Plugin::signal_unloading()
{
  return signal_unloading_;
}

sigc::signal<void, Glib::ustring> Plugin::signal_add_torrent()
{
  return signal_add_torrent_;
}

sigc::signal<bool> Plugin::signal_ui_toggle_visible()
{
  return signal_ui_toggle_visible_;
}

sigc::signal<void> Plugin::signal_quit()
{
  return signal_quit_;
}

Glib::ustring Plugin::get_name()
{
  return name_;
}

Glib::ustring Plugin::get_description()
{
  return description_;
}

int Plugin::get_version()
{
  return version_;
}

Plugin::PluginParent Plugin::get_parent()
{
  return PARENT_NONE;
}

Gtk::Widget* Plugin::get_widget()
{
  return 0;
}

void Plugin::on_load()
{
  /* If this method wasn't overridden try to unload */
  if (signal_unloading_.emit(this))
    delete this;
}

bool Plugin::update(Torrent* torrent)
{
  std::cout << str(torrent->get_hash());
  return false;
}

bool Plugin::on_notify(const Glib::ustring& title,
                          const Glib::ustring& message,
                          NotifyType type,
                          Torrent* torrent)
{
  return false;
}
