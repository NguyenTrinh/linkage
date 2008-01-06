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

#ifndef GROUP_LIST_HH
#define GROUP_LIST_HH

#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <libglademm/xml.h>

#include "Group.hh"
#include "linkage/Torrent.hh"

class GroupList : public Gtk::TreeView
{
	Glib::RefPtr<Gnome::Glade::Xml> glade_xml;

	class ModelColumns : public Gtk::TreeModelColumnRecord
	{
	public:
		ModelColumns()
		{
			add(name);
			add(num);
			add(group);
		}
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<unsigned int> num;
		Gtk::TreeModelColumn<GroupPtr> group;
	};
	ModelColumns columns;
	Glib::RefPtr<Gtk::ListStore> model;

	sigc::signal<void, const GroupPtr&> m_signal_filter_set;

	Linkage::Torrent::State m_cur_state;

	void format_name(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter);

	void on_selection_changed();

	void on_state_filter_changed(Linkage::Torrent::State state);
	void on_groups_changed(const std::list<GroupPtr>& groups);

	friend class UI;

public:
	sigc::signal<void, const GroupPtr&> signal_filter_set();

	void update();

	GroupList(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	~GroupList();
};

#endif /* GROUP_LIST_HH */

