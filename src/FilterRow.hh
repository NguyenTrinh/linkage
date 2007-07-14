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

#ifndef FILTER_ROW_HH
#define FILTER_ROW_HH

#include <gtkmm/box.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/entry.h>

#include "Group.hh"

class FilterRow : public Gtk::HBox
{
	Gtk::ComboBoxText *combo_tag, *combo_eval, *combo_operation;
	Gtk::Entry *entry_filter;

	void init(bool first);

public:
	bool has_focus();
	Group::Filter get_filter();

	FilterRow(bool first);
	FilterRow(bool first, const Group::Filter& filter);
	~FilterRow();
};

#endif /* FILTER_ROW_HH */

