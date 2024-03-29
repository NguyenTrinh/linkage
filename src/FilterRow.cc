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

#include <glibmm/i18n.h>

#include "FilterRow.hh"

FilterRow::FilterRow(bool first)
{
	init(first);

	combo_operation->set_active(0);
	combo_tag->set_active(0);
	combo_eval->set_active(0);
}

FilterRow::FilterRow(bool first, const Group::Filter& filter)
{
	init(first);

	combo_operation->set_active(filter.operation);
	combo_tag->set_active(filter.tag);
	combo_eval->set_active(filter.eval);
	entry_filter->set_text(filter.filter);
}

FilterRow::~FilterRow()
{
}

void FilterRow::init(bool first)
{
	/* Note, the order is important, row number correspond to enums */
	combo_operation = manage(new Gtk::ComboBoxText());
	if (first)
	{
		combo_operation->append_text(_("If"));
		combo_operation->append_text(_("If not"));
	}
	else
	{
		combo_operation->append_text(_("Or"));
		combo_operation->append_text(_("Or not"));
		combo_operation->append_text(_("And"));
		combo_operation->append_text(_("And not"));
	}

	combo_tag = manage(new Gtk::ComboBoxText());
	combo_tag->append_text(_("Tracker"));
	combo_tag->append_text(_("Name"));
	combo_tag->append_text(_("Comment"));

	combo_eval = manage(new Gtk::ComboBoxText());
	combo_eval->append_text(_("Equals"));
	combo_eval->append_text(_("Contains"));
	combo_eval->append_text(_("Starts with"));
	combo_eval->append_text(_("Ends with"));

	entry_filter = manage(new Gtk::Entry());

	/* FIXME: has_focus() still returns false :( */
	combo_operation->set_flags(Gtk::CAN_FOCUS);
	combo_eval->set_flags(Gtk::CAN_FOCUS);
	combo_tag->set_flags(Gtk::CAN_FOCUS);

	pack_start(*combo_operation, true, true);
	pack_start(*combo_tag, false, false);
	pack_start(*combo_eval, false, false);
	pack_start(*entry_filter, false, false);
}

bool FilterRow::has_focus()
{
	return combo_operation->has_focus() || combo_eval->has_focus() ||
		combo_tag->has_focus() || entry_filter->has_focus();
}

Group::Filter FilterRow::get_filter()
{
	Group::OperationType operation = Group::OperationType(combo_operation->get_active_row_number());
	Group::TagType tag = Group::TagType(combo_tag->get_active_row_number());
	Group::EvalType eval = Group::EvalType(combo_eval->get_active_row_number());
	Glib::ustring filter = entry_filter->get_text();

	return Group::Filter(filter, tag, eval, operation);
}

