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

#ifndef GROUP_FILTER_ROW_HH
#define GROUP_FILTER_ROW_HH

#include <gtkmm/comboboxtext.h>
#include <gtkmm/comboboxentrytext.h>
#include <gtkmm/entry.h>
#include <gtkmm/box.h>

class GroupFilterRow : public Gtk::HBox
{
	Gtk::Entry *m_name;
	Gtk::ComboBoxText *m_eval, *m_tag;
	Gtk::ComboBoxEntryText *m_filter;

	void on_tag_changed();
	void init();
	bool is_separator(const Glib::RefPtr<Gtk::TreeModel>& model,
										const Gtk::TreeIter& iter);
public:
	Glib::ustring get_filter();
	int get_tag();
	int get_eval();
	Glib::ustring get_name();
	bool has_focus();

	GroupFilterRow();
	GroupFilterRow(const Glib::ustring& filter,
									int tag,
									int eval,
									const Glib::ustring& name);
	~GroupFilterRow();
};

#endif /* GROUP_ROW_FILTER_HH */
