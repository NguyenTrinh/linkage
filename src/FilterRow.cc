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
	combotxt_filter->set_active_text(filter.filter);
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
		combo_operation->append_text("If");
		combo_operation->append_text("If not");
	}
	else
	{
		combo_operation->append_text("Or");
		combo_operation->append_text("Or not");
		combo_operation->append_text("And");
		combo_operation->append_text("And not");
	}

	combo_tag = manage(new Gtk::ComboBoxText());
	combo_tag->signal_changed().connect(sigc::mem_fun(this, &FilterRow::on_tag_changed));
	combo_tag->append_text("Tracker");
	combo_tag->append_text("Name");
	combo_tag->append_text("Comment");
	combo_tag->append_text("State");

	combo_eval = manage(new Gtk::ComboBoxText());
	combo_eval->append_text("Equals");
	combo_eval->append_text("Contains");
	combo_eval->append_text("Starts with");
	combo_eval->append_text("Ends with");

	combotxt_filter = manage(new Gtk::ComboBoxEntryText());

	/* FIXME: has_focus() still returns false :( */
	combo_operation->set_flags(Gtk::CAN_FOCUS);
	combo_eval->set_flags(Gtk::CAN_FOCUS);
	combo_tag->set_flags(Gtk::CAN_FOCUS);
	combotxt_filter->set_flags(Gtk::CAN_FOCUS);

	pack_start(*combo_operation, true, true);
	pack_start(*combo_tag, false, false);
	pack_start(*combo_eval, false, false);
	pack_start(*combotxt_filter, false, false);
}

void FilterRow::on_tag_changed()
{
	if (combo_tag->get_active_row_number() == Group::TAG_STATE)
	{
		combo_eval->set_active(Group::EVAL_EQUALS);
		combo_eval->set_sensitive(false);
		combotxt_filter->get_entry()->set_editable(false);
		combotxt_filter->append_text("Queued");
		combotxt_filter->append_text("Checking");
		combotxt_filter->append_text("Announcing");
		combotxt_filter->append_text("Downloading");
		combotxt_filter->append_text("Finished");
		combotxt_filter->append_text("Seeding");
		combotxt_filter->append_text("Allocating");
		combotxt_filter->append_text("Stopped");
		combotxt_filter->append_text("Queued for checking");
		combotxt_filter->append_text("Error");
	}
	else
	{
		combo_eval->set_sensitive(true);
		combotxt_filter->get_entry()->set_editable(true);
		combotxt_filter->clear_items();
	}
}

bool FilterRow::has_focus()
{
	return combo_operation->has_focus() || combo_eval->has_focus() ||
		combo_tag->has_focus() || combotxt_filter->has_focus() || 
		combotxt_filter->get_entry()->has_focus();
}

Group::Filter FilterRow::get_filter()
{
	Group::OperationType operation = Group::OperationType(combo_operation->get_active_row_number());
	Group::TagType tag = Group::TagType(combo_tag->get_active_row_number());
	Group::EvalType eval = Group::EvalType(combo_eval->get_active_row_number());
	Glib::ustring filter = combotxt_filter->get_active_text();

	return Group::Filter(filter, tag, eval, operation);
}

