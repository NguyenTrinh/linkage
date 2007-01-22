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

#include "linkage/TorrentMenuIface.hh"

TorrentMenuPlugin::TorrentMenuPlugin()
{
  m_name = "Generic menu plugin";
  m_description = "Generic menu description";
  m_version = 0;
}

TorrentMenuPlugin::TorrentMenuPlugin(const Glib::ustring& name, const Glib::ustring& description, unsigned int version)
{
  m_name = name;
  m_description = description;
  m_version = version;
}

TorrentMenuPlugin::~TorrentMenuPlugin()
{
}

Gtk::MenuItem* TorrentMenuPlugin::get_menu()
{
  return 0;
}

Plugin::PluginParent TorrentMenuPlugin::get_parent()
{
  return PARENT_MENU;
}

Plugin* CreatePlugin()
{
  return new TorrentMenuPlugin();
}
