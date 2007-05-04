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

#include <gtkmm/stock.h>

#include "GroupRow.hh"

GroupRow::FilterRow::FilterRow()
{
	init();
}

GroupRow::FilterRow::FilterRow(const Glib::ustring& filter, int tag, int eval)
{
	init();

	/* Offset due to separator */
	if (tag > 0)
		tag++;

	m_tag->set_active(tag);
	m_eval->set_active(eval);
	m_filter->get_entry()->set_text(filter);
}

GroupRow::FilterRow::~FilterRow()
{
}

void GroupRow::FilterRow::init()
{
	m_eval = manage(new Gtk::ComboBoxText());
	m_eval->append_text("Equals");
	m_eval->append_text("Contains");
	m_eval->append_text("Starts with");
	m_eval->append_text("Ends with");
	m_tag = manage(new Gtk::ComboBoxText());
	m_tag->set_row_separator_func(sigc::mem_fun(this, &GroupRow::FilterRow::is_separator));
	m_tag->signal_changed().connect(sigc::mem_fun(this, &GroupRow::FilterRow::on_tag_changed));
	m_tag->append_text("None");
	m_tag->append_text("-");
	m_tag->append_text("Tracker");
	m_tag->append_text("Name");
	m_tag->append_text("Comment");
	m_tag->append_text("State");
	m_filter = manage(new Gtk::ComboBoxEntryText());

	Gtk::Button* remove_button = manage(new Gtk::Button());
	remove_button->signal_clicked().connect(sigc::mem_fun(this, &GroupRow::FilterRow::on_remove));
	Gtk::Image* image = manage(new Gtk::Image(Gtk::Stock::REMOVE, Gtk::ICON_SIZE_BUTTON));
	remove_button->set_image(*image);

	pack_start(*m_tag, false, false);
	pack_start(*m_eval, false, false);
	pack_start(*m_filter, false, false);
	pack_start(*remove_button, false, false);
}

sigc::signal<void, GroupRow::FilterRow*> GroupRow::FilterRow::signal_removed()
{
	return m_signal_removed;
}

void GroupRow::FilterRow::on_remove()
{
	m_signal_removed.emit(this);
}

void GroupRow::FilterRow::on_tag_changed()
{
	int tag = m_tag->get_active_row_number();
	if ((tag - 1) == Group::TAG_STATE)
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

Glib::ustring GroupRow::FilterRow::get_filter()
{
	return m_filter->get_entry()->get_text();
}

int GroupRow::FilterRow::get_tag()
{
	int tag = m_tag->get_active_row_number();
	if (tag > 0)
		tag--;

	return tag;
}

int GroupRow::FilterRow::get_eval()
{
	return m_eval->get_active_row_number();
}

bool GroupRow::FilterRow::is_separator(const Glib::RefPtr<Gtk::TreeModel>& model,
																						 const Gtk::TreeIter& iter)
{
	Gtk::TreeRow row = *iter;
	Glib::ustring data;
	row.get_value(0, data);
	return (data == "-");
}


GroupRow::GroupRow(const Glib::ustring& name)
{
	init();

	m_name->set_text(name);
	m_exp->set_sensitive(false);
}

GroupRow::GroupRow(const Glib::ustring& name, const std::list<Group::Filter>& filters)
{
	init();

	m_name->set_text(name);

	for (std::list<Group::Filter>::const_iterator iter = filters.begin();
				iter != filters.end(); ++iter)
	{
		Group::Filter f = *iter;
		FilterRow* row = manage(new FilterRow(f.filter, f.tag, f.eval));
		row->signal_removed().connect(sigc::mem_fun(this, &GroupRow::on_removed));
		if (!m_exp->get_expanded())
			m_exp->set_expanded(true);
		m_vbox->pack_start(*row);
		row->show_all_children();
	}
	if (filters.empty())
		m_exp->set_sensitive(false);

	show_all_children();
}

void GroupRow::init()
{
	m_exp = manage(new ExpanderButton());
	m_name = manage(new Gtk::Entry());
	Gtk::Button* add_button = manage(new Gtk::Button());
	add_button->signal_clicked().connect(sigc::mem_fun(this, &GroupRow::on_add));
	Gtk::Image* image = manage(new Gtk::Image(Gtk::Stock::ADD,Gtk::ICON_SIZE_BUTTON));
	add_button->set_image(*image);

	Gtk::HBox* box = manage(new Gtk::HBox());
	box->pack_start(*m_exp, false, false);
	box->pack_start(*m_name, true, true);
	box->pack_start(*add_button, false, false);
	pack_start(*box, true, true);

	m_vbox = manage(new Gtk::VBox());

	m_exp->add(m_vbox);
	pack_start(*m_vbox, false, false);
}

GroupRow::~GroupRow()
{
}

void GroupRow::on_add()
{
	FilterRow* row = manage(new FilterRow());
	row->signal_removed().connect(sigc::mem_fun(this, &GroupRow::on_removed));
	if (!m_exp->get_expanded())
		m_exp->set_expanded(true);
	m_exp->set_sensitive(true);
	m_vbox->pack_start(*row, false, false);
	row->show_all_children();
	m_vbox->show_all_children();
}

void GroupRow::on_removed(FilterRow* row)
{
	m_vbox->remove(*row);
	delete row;
	if (m_vbox->get_children().empty())
	{
		m_exp->set_expanded(false);
		m_exp->set_sensitive(false);
	}
}

std::list<Group::Filter> GroupRow::get_filters()
{
	std::list<Group::Filter> filters;
	std::list<Gtk::Widget*> children = m_vbox->get_children();
	for (std::list<Gtk::Widget*>::iterator iter = children.begin();
				iter != children.end(); ++iter)
	{
		FilterRow* row = dynamic_cast<FilterRow*>(*iter);
		int tag = row->get_tag();
		int eval = row->get_eval();
		Glib::ustring filter = row->get_filter();
		filters.push_back(Group::Filter(filter, Group::TagType(tag), Group::EvalType(eval)));
	}

	return filters;
}

Glib::ustring GroupRow::get_name()
{
	return m_name->get_text();
}

bool GroupRow::has_focus()
{
	return (m_name->has_focus());
}
