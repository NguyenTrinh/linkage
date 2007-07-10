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

#include <libtorrent/entry.hpp>

#include "linkage/Utils.hh"
#include "linkage/Engine.hh"
#include "linkage/SettingsManager.hh"

#include "GroupList.hh"

GroupList::GroupList(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::TreeView(cobject),
	glade_xml(refGlade)
{
	model = Gtk::ListStore::create(columns);

	set_model(model);

	append_column("Name", columns.name);
	append_column("#", columns.num);

	m_cur_state = Torrent::NONE;

	/* Add the "All" filter with an invalid group */
	Gtk::TreeRow row = *(model->append());
	row[columns.name] = "All";
	row[columns.group] = Group();

	get_selection()->signal_changed().connect(sigc::mem_fun(this, &GroupList::on_selection_changed));
	
	show_all_children();
}

GroupList::~GroupList()
{
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
}

void GroupList::on_groups_changed(const std::list<Group>& groups)
{
	Gtk::TreeIter iter = get_selection()->get_selected();
	Glib::ustring selected;
	if (iter)
	{
		Gtk::TreeRow row = *iter;
		Group group = row[columns.group];
		selected = group.get_name();
	}

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

	for (std::list<Group>::const_iterator i = groups.begin(); i != groups.end(); ++i)
	{
		Group group = *i;
		Gtk::TreeRow row = *(model->append());
		row[columns.name] = group.get_name();
		row[columns.group] = group;
	}
}

sigc::signal<void, const Group&> GroupList::signal_filter_set()
{
	return m_signal_filter_set;
}

void GroupList::update()
{
	/* FIXME: group.eval() is duplicated in TorrentList, possible performance hit */
	TorrentManager::TorrentList torrents = Engine::get_torrent_manager()->get_torrents();

	Gtk::TreeNodeChildren children = model->children();
	for (Gtk::TreeIter i = children.begin(); i != children.end(); ++i)
	{
		Gtk::TreeRow row = *i;
		Group group = row[columns.group];
		if (group.is_valid())
		{
			unsigned int n = 0;
			for (TorrentManager::TorrentList::iterator j = torrents.begin(); j != torrents.end(); ++j)
			{
				WeakPtr<Torrent> torrent = *j;
				if (m_cur_state && m_cur_state != torrent->get_state())
					continue;
				if (group.eval(torrent))
					n++;
			}
			row[columns.num] = n;
		}
		else
			row[columns.num] = torrents.size();
	}
}

