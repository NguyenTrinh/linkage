/*
Copyright (C) 2006-2007   Christian Lundgren
Copyright (C) 2007        Dave Moore

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

#ifndef GROUPS_WIN_HH
#define GROUPS_WIN_HH

#include <gtkmm/window.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>

#include <libglademm.h>

#include <libtorrent/entry.hpp>

#include "GroupEditDialog.hh"

class GroupsWin : public Gtk::Window
{
	Glib::RefPtr<Gnome::Glade::Xml> glade_xml;
	
	class ModelColumns : public Gtk::TreeModelColumnRecord
	{
	public:
		ModelColumns()
			{
				add(name);
				add(group);
			}
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<GroupPtr> group;
	};

	Gtk::TreeView* groups_view;
	ModelColumns columns;
	Glib::RefPtr<Gtk::ListStore> model;

	GroupEditDialog* group_edit;

	sigc::signal<void, const std::list<GroupPtr>& > m_signal_groups_changed;

	void on_button_remove();
	void on_button_edit();
	void on_button_new();

	bool on_delete_event(GdkEventAny*);
	void on_button_close();
	void on_hide();

	void format_name(Gtk::CellRenderer* renderer, const Gtk::TreeIter& iter);

public:
	sigc::signal<void, const std::list<GroupPtr>& > signal_groups_changed();

	void notify();

	GroupsWin(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	virtual ~GroupsWin();
};

#endif /* GROUPS_WIN_HH */
