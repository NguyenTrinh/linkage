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

#ifndef INTERFACE_HH
#define INTERFACE_HH

#include <linkage/Torrent.hh>
#include <linkage/Plugin.hh>

#include <gtkmm/container.h>

class Interface
{
public:
	virtual HashList get_selected_list();

	virtual bool get_visible();
	virtual void set_visible(bool visible);

	virtual Gtk::Container* get_container(Plugin::PluginParent parent) = 0;

	virtual void open(const Glib::ustring& uri);

	virtual void quit();

	Interface();
	virtual ~Interface();
};

#endif /* INTERFACE_HH */

