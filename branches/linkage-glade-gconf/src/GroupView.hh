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

#ifndef GROUP_VIEW_HH
#define GROUP_VIEW_HH

#include <list>

#include <gtkmm/radiobuttongroup.h>
#include <gtkmm/box.h>

#include "GroupRow.hh"

class GroupView : public Gtk::VBox
{
	std::list<GroupRow*> m_children;

public:
	void append(GroupRow* row);
	void erase(GroupRow* row);
	GroupRow* get_row(const Glib::ustring& group);
	const std::list<GroupRow*>& children();
	GroupRow* get_selected();

	GroupView();
	~GroupView();
};

#endif /* GROUP_VIEW_HH */
