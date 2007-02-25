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

#ifndef GROUP_FILTER_VIEW_HH
#define GROUP_FILTER_VIEW_HH

#include <list>

#include <gtkmm/radiobuttongroup.h>
#include <gtkmm/box.h>

#include "GroupFilterRow.hh"

class GroupFilterView : public Gtk::VBox
{
	Gtk::RadioButtonGroup m_group;
	std::list<GroupFilterRow*> m_children;

public:
	void append(GroupFilterRow* row);
	void remove(GroupFilterRow* row);
	GroupFilterRow* get_row(const Glib::ustring& group);
	const std::list<GroupFilterRow*>& children();
	GroupFilterRow* get_selected();

	GroupFilterView();
	~GroupFilterView();
};

#endif /* GROUP_FILTER_VIEW_HH */
