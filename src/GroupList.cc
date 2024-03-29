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

#include <libtorrent/entry.hpp>

#include "StateFilter.hh"
#include "GroupList.hh"
#include "GroupsWin.hh"

#include "linkage/Engine.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/TorrentManager.hh"
#include "linkage/Utils.hh"

using namespace Linkage;

GroupList::GroupList(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::TreeView(cobject),
	glade_xml(refGlade)
{
	model = Gtk::ListStore::create(columns);

	set_model(model);

	append_column(_("Name"), columns.name);
	Gtk::TreeViewColumn* column = get_column(0);
	Gtk::CellRenderer* renderer = column->get_first_cell_renderer();
	column->set_cell_data_func(*renderer, sigc::mem_fun(this, &GroupList::format_name));

	m_cur_state = Torrent::NONE;

	/* Add the "All" filter with an invalid group */
	Gtk::TreeRow row = *(model->append());
	row[columns.group] = GroupPtr(NULL);

	get_selection()->signal_changed().connect(sigc::mem_fun(this, &GroupList::on_selection_changed));

	/* Connect update signals */
	GroupsWin* groups_win;
	glade_xml->get_widget_derived("groups_win", groups_win);
	groups_win->signal_groups_changed().connect(sigc::mem_fun(this, &GroupList::on_groups_changed));
	StateFilter* state_filter;
	glade_xml->get_widget_derived("state_combobox", state_filter);
	state_filter->signal_state_filter_changed().connect(sigc::mem_fun(this, &GroupList::on_state_filter_changed));

	show_all_children();
}

GroupList::~GroupList()
{
	Glib::ustring selected;
	Gtk::TreeIter iter = get_selection()->get_selected();
	if (iter)
	{
		Gtk::TreeRow row = *iter;
		GroupPtr group = row[columns.group];
		if (group)
			selected = group->get_name();
	}
	Engine::get_settings_manager()->set("ui/active_group", selected);
}

void GroupList::format_name(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter)
{
	Gtk::TreeRow row = *iter;
	Gtk::CellRendererText* cell_t = dynamic_cast<Gtk::CellRendererText*>(cell);

	GroupPtr group = row[columns.group];
	Glib::ustring name = group ? group->get_name() : _("All");
	cell_t->property_text() = String::ucompose("%1 (%2)", name, row[columns.num]);
}


void GroupList::on_selection_changed()
{
	Gtk::TreeIter iter = get_selection()->get_selected();
	if (iter)
	{
		Gtk::TreeRow row = *iter;
		m_signal_filter_set.emit(row[columns.group]);
	}
}

void GroupList::on_state_filter_changed(Torrent::State state)
{
	m_cur_state = state;

	update();
}

void GroupList::on_groups_changed(const std::list<GroupPtr>& groups)
{
	// find selected name
	Gtk::TreeIter iter = get_selection()->get_selected();
	GroupPtr selected;
	if (iter)
	{
		Gtk::TreeRow row = *iter;
		selected = row[columns.group];
	}

	// clear list
	Gtk::TreeNodeChildren children = model->children();
	for (Gtk::TreeIter i = children.begin(); i != children.end();)
	{
		Gtk::TreeRow row = *i;
		GroupPtr group = row[columns.group];
		/* Don't delete "All" */
		if (group)
			i = model->erase(i);
		else
			i++;
	}

	// add new groups
	for (std::list<GroupPtr>::const_iterator i = groups.begin(); i != groups.end(); ++i)
	{
		GroupPtr group = *i;
		Gtk::TreeRow row = *(model->append());
		row[columns.group] = group;
	}

	// select previously selected group if it still exists
	children = model->children();
	for (Gtk::TreeIter i = children.begin(); i != children.end(); ++i)
	{
		Gtk::TreeRow row = *i;
		GroupPtr group = row[columns.group];
		if (group == selected)
		{
			get_selection()->select(i);
			return;
		}
	}


	Glib::ustring active = Engine::get_settings_manager()->get_string("ui/active_group");
	if (!active.empty())
	{
		Gtk::TreeNodeChildren children = model->children();
		for (Gtk::TreeIter i = children.begin(); i != children.end(); ++i)
		{
			Gtk::TreeRow row = *i;
			GroupPtr group = row[columns.group];
			if (group && group->get_name() == active)
				get_selection()->select(i);
		}
	}
}

sigc::signal<void, const GroupPtr&> GroupList::signal_filter_set()
{
	return m_signal_filter_set;
}

void GroupList::update()
{
	TorrentManager::TorrentList torrents = Engine::get_torrent_manager()->get_torrents();

	Gtk::TreeRow all;

	Gtk::TreeNodeChildren children = model->children();
	for (Gtk::TreeIter i = children.begin(); i != children.end(); ++i)
	{
		Gtk::TreeRow row = *i;
		GroupPtr group = row[columns.group];

		unsigned int n = 0;
		if (!m_cur_state && !group) // "All" row without any state filter
			n = torrents.size();

		for (TorrentManager::TorrentList::iterator j = torrents.begin();
			j != torrents.end() && n < torrents.size(); ++j)
		{
			TorrentPtr& torrent = *j;
			if (m_cur_state && !(m_cur_state & torrent->get_state()))
				continue;

			if (group) // not "All" row
			{
				if (group->eval(torrent))
					n++;
			}
			else if (m_cur_state) // "All" row with state filter set
			{
				if (m_cur_state & torrent->get_state())
					n++;
			}
		}
		row[columns.num] = n;
	}
}

