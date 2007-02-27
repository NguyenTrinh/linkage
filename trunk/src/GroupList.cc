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

#include "linkage/Utils.hh"
#include "linkage/Engine.hh"
#include "linkage/SettingsManager.hh"
#include "GroupList.hh"

GroupList::GroupList()
{
	m_all = manage(new Gtk::RadioButton("All"));
	m_all->signal_toggled().connect(sigc::mem_fun(this, &GroupList::on_all_toggled));
	pack_start(*m_all, false, false);

	on_settings();

	Engine::instance()->get_settings_manager()->signal_update_settings().connect(sigc::mem_fun(this, &GroupList::on_settings));
	
	show_all_children();
}

GroupList::~GroupList()
{
	Glib::RefPtr<SettingsManager> sm = Engine::instance()->get_settings_manager();
	for (GroupMap::iterator iter = m_map.begin(); iter != m_map.end(); ++iter)
	{
		GroupFilter* group = iter->first;
		
		std::list<Glib::ustring> info;
		info.push_back(group->get_filter());
		info.push_back(str((int)group->get_eval()));
		info.push_back(str((int)group->get_tag()));

		sm->set("Groups", group->get_name(), UStringArray(info));
		
		Gtk::RadioButton* radio = iter->second;
		remove(*radio);
		delete radio;
		delete group;
	}
	m_map.clear();
}

void GroupList::on_group_toggled(GroupFilter* group)
{
	for (GroupMap::iterator iter = m_map.begin(); iter != m_map.end(); ++iter)
	{
		Gtk::RadioButton* radio = iter->second;
		if (radio->get_active())
			radio->set_active(false);
	}
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
		GroupFilter* group = iter->first;
		if (radio->get_active())
			active_group = group->get_name();
		remove(*radio);
		delete radio;
		delete group;
	}
	m_map.clear();

	Glib::RefPtr<SettingsManager> sm = Engine::instance()->get_settings_manager();
	std::list<Glib::ustring> keys = sm->get_keys("Groups");
	for (std::list<Glib::ustring>::iterator iter = keys.begin();
				iter != keys.end(); ++iter)
	{
		std::vector<Glib::ustring> info = sm->get_string_list("Groups", *iter);

		if (info.size() != 3)
			continue;

		Glib::ustring filter = info[0];
		GroupFilter::EvalType eval = GroupFilter::EvalType(std::atoi(info[1].c_str()));
		GroupFilter::TagType tag = GroupFilter::TagType(std::atoi(info[2].c_str()));
		
		GroupFilter* group = new GroupFilter(filter, tag, eval, *iter);
		Gtk::RadioButtonGroup radio_group = m_all->get_group();
		Gtk::RadioButton* radio = new Gtk::RadioButton(radio_group, group->get_name());
		m_map[group] = radio;
		radio->signal_toggled().connect(sigc::bind(sigc::mem_fun(this, &GroupList::on_group_toggled), group));
		radio->set_active(active_group == group->get_name());
		pack_start(*radio, false, false);
		radio->show();
	}
}

sigc::signal<void, const GroupFilter&> GroupList::signal_filter_set()
{
	return m_signal_filter_set;
}

sigc::signal<void> GroupList::signal_filter_unset()
{
	return m_signal_filter_unset;
}

void GroupList::update()
{
	TorrentManager::TorrentList torrents = Engine::instance()->get_torrent_manager()->get_torrents();

	for (GroupMap::iterator iter = m_map.begin(); iter != m_map.end(); ++iter)
	{
		GroupFilter* group = iter->first;
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
}
