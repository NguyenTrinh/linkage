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

#include "linkage/Plugin.hh"
#include "linkage/Utils.hh"

Plugin::Plugin()
{
	m_name = "Generic plugin";
	m_description = "Generic plugin description";
	m_version = 0;
}

Plugin::Plugin(const Glib::ustring& name, const Glib::ustring& description, unsigned int version)
{
	m_name = name;
	m_description = description;
	m_version = version;
}

Plugin::~Plugin()
{
	m_signal_unloading.emit(this);
}

sigc::signal<void, Plugin*, Gtk::Widget*, Plugin::PluginParent> Plugin::signal_add_widget()
{
	return m_signal_add_widget;
}

void Plugin::add_widget(Gtk::Widget* widget, Plugin::PluginParent parent)
{
	m_signal_add_widget.emit(this, widget, parent);
}

sigc::signal<void, Plugin*> Plugin::signal_unloading()
{
	return m_signal_unloading;
}

Glib::ustring Plugin::get_name()
{
	return m_name;
}

Glib::ustring Plugin::get_description()
{
	return m_description;
}

unsigned int Plugin::get_version()
{
	return m_version;
}
