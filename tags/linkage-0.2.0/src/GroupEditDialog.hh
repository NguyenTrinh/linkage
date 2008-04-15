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

#ifndef GROUP_EDIT_DIALOG_HH
#define GROUP_EDIT_DIALOG_HH

#include <list>

#include <gtkmm/box.h>
#include <gtkmm/entry.h>
#include <gtkmm/dialog.h>

#include <libglademm.h>

#include "Group.hh"
#include "FilterRow.hh"

class GroupEditDialog : public Gtk::Dialog
{
	Glib::RefPtr<Gnome::Glade::Xml> glade_xml;

	std::list<FilterRow*> m_filters;

	Gtk::VBox* filters_box;
	Gtk::Entry* name_entry;

	void on_button_add();
	void on_button_remove();

	void clear();

public:
	int run(Glib::ustring& name, std::list<Group::Filter>& filters);

	GroupEditDialog(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	virtual ~GroupEditDialog();
};

#endif /* GROUP_EDIT_DIALOG_HH */

