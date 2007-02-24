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

#include "linkage/Engine.hh"
#include "linkage/SettingsManager.hh"
#include "FilterManager.hh"
#include <iostream>

FilterManager::FilterManager()
{
	on_settings();

	Engine::instance()->get_settings_manager()->signal_update_settings().connect(sigc::mem_fun(this, &FilterManager::on_settings));
}

FilterManager::~FilterManager()
{
	Glib::RefPtr<SettingsManager> sm = Engine::instance()->get_settings_manager();
	for (std::list<GroupFilter*>::iterator iter = m_filters.begin();
				iter != m_filters.end(); ++iter)
	{
		GroupFilter* group = *iter;

		std::list<Glib::ustring> info;
		info.push_back(group->get_filter());
		info.push_back(str((int)group->get_eval()));
		info.push_back(str((int)group->get_tag()));

		sm->set("Filters", group->get_name(), UStringArray(info));

		delete group;
	}
	m_filters.clear();
}

void FilterManager::on_settings()
{
	for (std::list<GroupFilter*>::iterator iter = m_filters.begin();
				iter != m_filters.end(); ++iter)
	{
		delete *iter;
	}
	m_filters.clear();

	Glib::RefPtr<SettingsManager> sm = Engine::instance()->get_settings_manager();
	std::list<Glib::ustring> keys = sm->get_keys("Filters");
	for (std::list<Glib::ustring>::iterator iter = keys.begin();
				iter != keys.end(); ++iter)
	{
		std::vector<Glib::ustring> info = sm->get_string_list("Filters", *iter);

		if (info.size() != 3)
			continue;

		Glib::ustring filter = info[0];
		GroupFilter::EvalType eval = GroupFilter::EvalType(std::atoi(info[1].c_str()));
		GroupFilter::TagType tag = GroupFilter::TagType(std::atoi(info[2].c_str()));

		m_filters.push_back(new GroupFilter(filter, tag, eval, *iter));
	}
}

void FilterManager::check_filters(const WeakPtr<Torrent>& torrent)
{
	for (std::list<GroupFilter*>::iterator iter = m_filters.begin();
				iter != m_filters.end(); ++iter)
	{
		GroupFilter* group = *iter;
		if (group->eval(torrent) && torrent->get_group() != group->get_name())
		{
			torrent->set_group(group->get_name());
			return;
		}
	}

	for (std::list<GroupFilter*>::iterator iter = m_filters.begin();
				iter != m_filters.end(); ++iter)
	{
		GroupFilter* group = *iter;
		if (!group->eval(torrent) && torrent->get_group() == group->get_name())
		{
			Glib::ustring group = Engine::instance()->get_settings_manager()->get_string("Files", "DefGroup");
			torrent->set_group(group);
		}
	}
}
