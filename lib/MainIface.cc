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
  name_ = "Generic main plugin";
  description_ = "Generic main description";
  version_ = 0;
  
  title_ = name_;
}

MainPlugin::MainPlugin(const Glib::ustring& name, const Glib::ustring& description, int version, const Glib::ustring& title)
{
  name_ = name;
  description_ = description;
  version_ = version;
  
  title_ = title;
}

MainPlugin::~MainPlugin()
{
}

const Glib::ustring MainPlugin::get_title()
{
  return title_;
}

Plugin::PluginParent MainPlugin::get_parent()
{
  return PARENT_MAIN;
}