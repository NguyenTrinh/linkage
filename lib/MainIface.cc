/*
Copyright (C) 2006  Christian Lundgren

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more Main.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "linkage/MainIface.hh"

MainPlugin::MainPlugin()
{
  m_name = "Generic main plugin";
  m_description = "Generic main description";
  m_version = 0;
  
  m_title = m_name;
}

MainPlugin::MainPlugin(const Glib::ustring& name, const Glib::ustring& description, unsigned int version, const Glib::ustring& title)
{
  m_name = name;
  m_description = description;
  m_version = version;
  
  m_title = title;
}

MainPlugin::~MainPlugin()
{
}

const Glib::ustring MainPlugin::get_title()
{
  return m_title;
}

Plugin::PluginParent MainPlugin::get_parent()
{
  return PARENT_MAIN;
}
