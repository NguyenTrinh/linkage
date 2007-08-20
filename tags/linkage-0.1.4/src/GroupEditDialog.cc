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

#include "GroupEditDialog.hh"

GroupEditDialog::GroupEditDialog(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::Dialog(cobject),
	glade_xml(refGlade)
{
	glade_xml->get_widget("filters_vbox", filters_box);
	glade_xml->get_widget("group_name", name_entry);

	Gtk::Button* add_button = NULL;
	glade_xml->get_widget("filter_add", add_button);
	add_button->signal_clicked().connect(sigc::mem_fun(this, &GroupEditDialog::on_button_add));
	add_button->unset_flags(Gtk::CAN_FOCUS);

	Gtk::Button* remove_button = NULL;
	glade_xml->get_widget("filter_remove", remove_button);
	remove_button->signal_clicked().connect(sigc::mem_fun(this, &GroupEditDialog::on_button_remove));
	remove_button->unset_flags(Gtk::CAN_FOCUS);
}

GroupEditDialog::~GroupEditDialog()
{
	clear();
}

void GroupEditDialog::clear()
{
	for (std::list<FilterRow*>::iterator iter = m_filters.begin();
		iter != m_filters.end();)
	{
		FilterRow* row = *iter;
		filters_box->remove(*row);
		delete row;
		iter = m_filters.erase(iter);
	}
}

int GroupEditDialog::run(Glib::ustring& name, std::list<Group::Filter>& filters)
{
	name_entry->set_text(name);
	for (std::list<Group::Filter>::iterator iter = filters.begin();
		iter != filters.end(); ++iter)
	{
		FilterRow* row = new FilterRow(m_filters.empty(), *iter);
		filters_box->pack_start(*row, false, false);
		m_filters.push_back(row);
	}
	show_all_children();

	int ret = Gtk::Dialog::run();
	hide();

	name = name_entry->get_text();
	filters.clear();
	for (std::list<FilterRow*>::iterator iter = m_filters.begin();
		iter != m_filters.end(); ++iter)
	{
		FilterRow* row = *iter;
		filters.push_back(row->get_filter());
	}

	clear();

	return ret;
}

void GroupEditDialog::on_button_add()
{
	FilterRow* row = new FilterRow(m_filters.empty());
	filters_box->pack_start(*row, false, false);
	m_filters.push_back(row);

	show_all_children();
}

void GroupEditDialog::on_button_remove()
{
	for (std::list<FilterRow*>::iterator iter = m_filters.begin();
		iter != m_filters.end(); ++iter)
	{
		FilterRow* row = *iter;

		if (row->has_focus())
		{
			filters_box->remove(*row);
			delete row;
			m_filters.erase(iter);
			return;
		}
	}
}

