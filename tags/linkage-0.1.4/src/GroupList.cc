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

#include "GroupList.hh"

#include "linkage/Engine.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/TorrentManager.hh"
#include "linkage/Utils.hh"

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
	row[columns.name] = _("All");
	row[columns.group] = Group();

	get_selection()->signal_changed().connect(sigc::mem_fun(this, &GroupList::on_selection_changed));

	show_all_children();
}

GroupList::~GroupList()
{
	Glib::ustring selected;
	Gtk::TreeIter iter = get_selection()->get_selected();
	if (iter)
	{
		Gtk::TreeRow row = *iter;
		selected = row[columns.name];
	}
	Engine::get_settings_manager()->set("ui/active_group", selected);
}

void GroupList::format_name(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter)
{
	Gtk::TreeRow row = *iter;
	Gtk::CellRendererText* cell_t = dynamic_cast<Gtk::CellRendererText*>(cell);

	cell_t->property_text() = row[columns.name] + " (" + str(row[columns.num]) + ")";
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

void GroupList::on_groups_changed(const std::list<Group>& groups)
{
	// find selected name
	Gtk::TreeIter iter = get_selection()->get_selected();
	Glib::ustring selected;
	if (iter)
	{
		Gtk::TreeRow row = *iter;
		selected = row[columns.name];
	}

	// clear list
	Gtk::TreeNodeChildren children = model->children();
	for (Gtk::TreeIter i = children.begin(); i != children.end();)
	{
		Gtk::TreeRow row = *i;
		Group group = row[columns.group];
		/* Don't delete "All" */
		if (group.is_valid())
			i = model->erase(i);
		else
			i++;
	}

	// add new groups
	for (std::list<Group>::const_iterator i = groups.begin(); i != groups.end(); ++i)
	{
		Group group = *i;
		Gtk::TreeRow row = *(model->append());
		row[columns.name] = group.get_name();
		row[columns.group] = group;
	}

	// select previously selected group if it still exists
	if (!selected.empty())
	{
		children = model->children();
		for (Gtk::TreeIter i = children.begin(); i != children.end(); ++i)
		{
			Gtk::TreeRow row = *i;
			Glib::ustring name = row[columns.name];
			if (name == selected)
				get_selection()->select(i);
		}
	}
	else
	{
		Glib::ustring active = Engine::get_settings_manager()->get_string("ui/active_group");
		if (!active.empty())
		{
			Gtk::TreeNodeChildren children = model->children();
			for (Gtk::TreeIter i = children.begin(); i != children.end(); ++i)
			{
				Gtk::TreeRow row = *i;
				Glib::ustring name = row[columns.name];
				if (name == active)
					get_selection()->select(i);
			}
		}
	}
}

sigc::signal<void, const Group&> GroupList::signal_filter_set()
{
	return m_signal_filter_set;
}

void GroupList::update()
{
	TorrentManager::TorrentList torrents = Engine::get_torrent_manager()->get_torrents();

	Gtk::TreeRow all;
	unsigned int n_all = 0;

	Gtk::TreeNodeChildren children = model->children();
	for (Gtk::TreeIter i = children.begin(); i != children.end(); ++i)
	{
		Gtk::TreeRow row = *i;
		Group group = row[columns.group];

		unsigned int n = 0;
		for (TorrentManager::TorrentList::iterator j = torrents.begin(); j != torrents.end(); ++j)
		{
			WeakPtr<Torrent> torrent = *j;
			if (m_cur_state && m_cur_state != torrent->get_state())
				continue;

			if (group.is_valid()) // not "All" row
			{
				if (group.eval(torrent))
					n++;
			}
			else if (m_cur_state) // "All" row with state filter set
			{
				if (m_cur_state == torrent->get_state())
					n++;
			}
			else // "All" row without any state filter
				n++;
		}
		row[columns.num] = n;
	}
}
