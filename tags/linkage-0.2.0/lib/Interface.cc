/*
Copyright (C) 2007	Christian Lundgren

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

#include "linkage/Interface.hh"
#include "linkage/Engine.hh"

using namespace Linkage;

Interface::Interface()
:
  DBus::ObjectAdaptor(Engine::get_bus(), "/org/linkage/interface")
{
}

Interface::~Interface()
{
}

SelectionList Interface::get_selected() const
{
	return SelectionList();
}

bool Interface::get_visible() const
{
	return false;
}

Gtk::Container* Interface::get_container(Plugin::PluginParent parent)
{
	return NULL;
}

void Interface::set_visible(bool visible = true)
{
}

void Interface::open(const Glib::ustring& uri)
{
}

void Interface::quit()
{
}

