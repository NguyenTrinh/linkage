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

#ifndef SIMPLE_FILELIST_HH
#define SIMPLE_FILELIST_HH

#include <vector>

#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <libglademm.h>

#include "libtorrent/torrent.hpp"

class SimpleFileList : public Gtk::TreeView
{
	class ModelColumns : public Gtk::TreeModelColumnRecord
	{
	public:
		ModelColumns()
		{
			add(filter);
			add(name);
			add(size);
			add(index);
		}
		Gtk::TreeModelColumn<bool> filter;
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<libtorrent::size_type> size;
		Gtk::TreeModelColumn<int> index;
	};
	
	ModelColumns columns;
	Glib::RefPtr<Gtk::ListStore> model;

	void on_filter_toggled(const Glib::ustring& path);

	void format_data(Gtk::CellRenderer* cell,
		const Gtk::TreeIter& iter,
		const Gtk::TreeModelColumn<libtorrent::size_type>& column);

public:
	std::vector<bool> get_filter();

	void populate(const boost::intrusive_ptr<libtorrent::torrent_info>& info);
	void clear();

	SimpleFileList(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	~SimpleFileList();
};

#endif /* SIMPLE_FILELIST_HH */
