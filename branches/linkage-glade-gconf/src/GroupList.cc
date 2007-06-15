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
	: Gtk::VBox(cobject),
	glade_xml(refGlade)
{
	m_all = manage(new Gtk::RadioButton("All"));
	m_all->signal_toggled().connect(sigc::mem_fun(this, &GroupList::on_all_toggled));
	pack_start(*m_all, false, false);

	on_settings();

	Engine::get_settings_manager()->signal_update_settings().connect(sigc::mem_fun(this, &GroupList::on_settings));
	
	show_all_children();
}

GroupList::~GroupList()
{
	Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();
	entry::dictionary_type groups;
	for (GroupMap::iterator iter = m_map.begin(); iter != m_map.end(); ++iter)
	{
		Group* group = iter->first;
		
		entry::list_type efilters;
		std::list<Group::Filter> filters = group->get_filters();

		for (std::list<Group::Filter>::iterator giter = filters.begin();
					giter != filters.end(); ++giter)
		{
			Group::Filter f = *giter;
			entry::dictionary_type filter;
			
			filter["filter"] = entry(f.filter);
			filter["eval"] = entry(f.eval);
			filter["tag"] = entry(f.tag);
			efilters.push_back(entry(filter));
		}
		groups[group->get_name()] = entry(efilters);
		
		Gtk::RadioButton* radio = iter->second;
		remove(*radio);
		delete radio;
		delete group;
	}
	sm->write_groups_data(entry(groups));
	m_map.clear();
}

void GroupList::on_group_toggled(Group* group)
{
	m_signal_filter_set.emit(*group);
}

void GroupList::on_all_toggled()
{
	m_signal_filter_unset.emit();
}

void GroupList::on_settings()
{
	Glib::ustring active_group;
	for (GroupMap::iterator iter = m_map.begin(); iter != m_map.end(); ++iter)
	{
		Gtk::RadioButton* radio = iter->second;
		Group* group = iter->first;
		if (radio->get_active())
			active_group = group->get_name();
		remove(*radio);
		delete radio;
		delete group;
	}
	m_map.clear();

	bool all_active = true;
	// build the group list from the bencoded file
	Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();

	using namespace libtorrent;
	entry e;
	sm->get_groups_data(e);

	for (entry::dictionary_type::iterator iter = e.dict().begin();
			iter != e.dict().end(); ++iter)
	{
		std::list<Group::Filter> filters;

		for (entry::list_type::const_iterator giter = (*iter).second.list().begin();
				giter != (*iter).second.list().end(); ++giter)
		{
			Glib::ustring filter = (*giter)["filter"].string();
			Group::EvalType eval = Group::EvalType((*giter)["eval"].integer());
			Group::TagType tag = Group::TagType((*giter)["tag"].integer());
			filters.push_back(Group::Filter(filter, tag, eval));
		}
		
		Group* group = new Group((*iter).first, filters);
		Gtk::RadioButtonGroup radio_group = m_all->get_group();
		Gtk::RadioButton* radio = new Gtk::RadioButton(radio_group, group->get_name());
		m_map[group] = radio;
		radio->signal_toggled().connect(sigc::bind(sigc::mem_fun(this, &GroupList::on_group_toggled), group));
		if (active_group == group->get_name())
		{
			radio->set_active(true);
			all_active = false;
		}
		pack_start(*radio, false, false);
		radio->show();
	}
	if (all_active)
		m_all->set_active(true);
}

sigc::signal<void, const Group&> GroupList::signal_filter_set()
{
	return m_signal_filter_set;
}

sigc::signal<void> GroupList::signal_filter_unset()
{
	return m_signal_filter_unset;
}

void GroupList::update()
{
	TorrentManager::TorrentList torrents = Engine::get_torrent_manager()->get_torrents();

	for (GroupMap::iterator iter = m_map.begin(); iter != m_map.end(); ++iter)
	{
		Group* group = iter->first;
		int n = 0;
		for (TorrentManager::TorrentList::iterator titer = torrents.begin();
					titer != torrents.end(); ++titer)
		{
			if (group->eval(*titer))
				n++;
		}
		Gtk::RadioButton* radio = iter->second;

		radio->set_label(group->get_name() + " (" + str(n) + ")");
	}
	
	m_all->set_label("All (" + str(torrents.size()) + ")");
}
