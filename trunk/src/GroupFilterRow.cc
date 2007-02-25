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

#include "GroupFilterRow.hh"
#include "GroupFilter.hh"

GroupFilterRow::GroupFilterRow()
{
	init();
}

GroupFilterRow::GroupFilterRow(const Glib::ustring& name,
																int tag,
																int eval,
																const Glib::ustring& filter)
{
	init();

	m_name->set_text(name);
	m_eval->set_active(eval);
	m_tag->set_active(tag);
	m_filter->get_entry()->set_text(filter);
}

void GroupFilterRow::init()
{
	m_radio = new Gtk::RadioButton();
	m_radio->signal_toggled().connect(sigc::mem_fun(this, &GroupFilterRow::on_radio_changed));
	m_name = new Gtk::Entry();
	m_eval = new Gtk::ComboBoxText();
	m_eval->append_text("Equals");
	m_eval->append_text("Contains");
	m_eval->append_text("Starts with");
	m_eval->append_text("Ends with");
	m_tag = new Gtk::ComboBoxText();
	m_tag->set_row_separator_func(sigc::mem_fun(this, &GroupFilterRow::is_separator));
	m_tag->signal_changed().connect(sigc::mem_fun(this, &GroupFilterRow::on_tag_changed));
	m_tag->append_text("None");
	m_tag->append_text("-");
	m_tag->append_text("Tracker");
	m_tag->append_text("Name");
	m_tag->append_text("Comment");
	m_tag->append_text("State");
	m_filter = new Gtk::ComboBoxEntryText();

	pack_start(*m_radio, false, false);
	pack_start(*m_name, false, false);
	pack_start(*m_tag, false, false);
	pack_start(*m_eval, false, false);
	pack_start(*m_filter, false, false);

	show_all_children();
}

GroupFilterRow::~GroupFilterRow()
{
	delete m_radio;
	delete m_name;
	delete m_tag;
	delete m_eval;
	delete m_filter;
}

void GroupFilterRow::on_tag_changed()
{
	int tag = m_tag->get_active_row_number();
	if ((tag - 2) == GroupFilter::TAG_STATE)
	{
		m_eval->set_sensitive(true);
		m_filter->set_sensitive(true);
		m_filter->get_entry()->set_editable(false);
		m_filter->append_text("Queued");
		m_filter->append_text("Checking");
		m_filter->append_text("Announcing");
		m_filter->append_text("Downloading");
		m_filter->append_text("Finished");
		m_filter->append_text("Seeding");
		m_filter->append_text("Allocating");
		m_filter->append_text("Stopped");
	}
	else if (tag == 0)
	{
		m_eval->set_sensitive(false);
		m_filter->set_sensitive(false);
	}
	else
	{
		m_eval->set_sensitive(true);
		m_filter->set_sensitive(true);
		m_filter->get_entry()->set_editable(true);
		m_filter->clear_items();
	}
}

void GroupFilterRow::on_radio_changed()
{
	if (m_radio->get_active())
	{
		m_tag->set_active(0);
		m_tag->set_sensitive(false);
	}
	else
	{
		m_tag->set_sensitive(true);
	}
}

Glib::ustring GroupFilterRow::get_filter()
{
	return m_filter->get_entry()->get_text();
}

int GroupFilterRow::get_tag()
{
	return m_tag->get_active_row_number();
}

int GroupFilterRow::get_eval()
{
	return m_eval->get_active_row_number();
}

Glib::ustring GroupFilterRow::get_name()
{
	return m_name->get_text();
}

bool GroupFilterRow::has_focus()
{
	return (m_filter->has_focus() || m_tag->has_focus() || m_eval->has_focus() || m_name->has_focus() || m_radio->has_focus());
}

bool GroupFilterRow::is_default()
{
	return m_radio->get_active();
}

void GroupFilterRow::set_group(Gtk::RadioButtonGroup& group)
{
	m_radio->set_group(group);
}

Gtk::RadioButtonGroup GroupFilterRow::get_group()
{
	return m_radio->get_group();
}

void GroupFilterRow::set_default()
{
	m_radio->set_active(true);
}

bool GroupFilterRow::is_separator(const Glib::RefPtr<Gtk::TreeModel>& model,
																	 const Gtk::TreeIter& iter)
{
	Gtk::TreeRow row = *iter;
	Glib::ustring data;
	row.get_value(0, data);
	return (data == "-");
}
