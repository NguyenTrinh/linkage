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

#ifndef GROUP_ROW_HH
#define GROUP_ROW_HH

#include <gtkmm/comboboxtext.h>
#include <gtkmm/comboboxentrytext.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/entry.h>
#include <gtkmm/box.h>

#include "Group.hh"
#include "ExpanderButton.hh"

class GroupRow : public Gtk::VBox
{
	class FilterRow : public Gtk::HBox
	{
		Gtk::ComboBoxText *m_tag, *m_eval;
		Gtk::ComboBoxEntryText *m_filter;

		sigc::signal<void, FilterRow*> m_signal_removed;

		void on_remove();
		void on_tag_changed();
		bool is_separator(const Glib::RefPtr<Gtk::TreeModel>& model,
											const Gtk::TreeIter& iter);
		void init();

	public:
		Glib::ustring get_filter();
		int get_tag();
		int get_eval();

		sigc::signal<void, FilterRow*> signal_removed();

		FilterRow();
		FilterRow(const Glib::ustring& filter, int tag, int eval);
		~FilterRow();
	};

	Gtk::Entry* m_name;
	Gtk::VBox* m_vbox;
	ExpanderButton* m_exp;

	void on_add();
	void on_removed(FilterRow* row);
	void init();

public:
	std::list<Group::Filter> get_filters();
	Glib::ustring get_name();
	bool has_focus();

	GroupRow(const Glib::ustring& name);
	GroupRow(const Glib::ustring& name, const std::list<Group::Filter>& filters);
	~GroupRow();
};

#endif /* GROUP_ROW_HH */
