/*
Copyright (C) 2006	Christian Lundgren

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

#include <glibmm/stringutils.h>

#include "Group.hh"


Group::Group(const Glib::ustring& name, const std::list<Filter>& filters)
{
	m_filters.assign(filters.begin(), filters.end());
	m_name = name;
}

Group::Group()
{
	/* Invalid group */
}

Group::~Group()
{
}

bool Group::eval(const WeakPtr<Torrent>& torrent) const
{
	torrent_info info = torrent->get_info();
	
	if (m_name.empty())
		return false;
	
	if (torrent->get_group() == m_name)
		return true;

	bool ret = false;
	for (std::list<Filter>::const_iterator iter = m_filters.begin();
				iter != m_filters.end() && !ret; ++iter)
	{
		Filter f = *iter;
		switch (f.eval)
		{
			case EVAL_EQUALS:
				switch (f.tag)
				{
					case TAG_TRACKER:
						for (unsigned int i = 0; i < info.trackers().size() && !ret; i++)
						{
							Glib::ustring tracker = info.trackers()[i].url;
							ret = (f.filter == tracker);
						}
						break;
					case TAG_NAME:
						ret = (f.filter == info.name());
						break;
					case TAG_COMMENT:
						ret = (f.filter == info.comment());
						break;
					case TAG_STATE:
						ret = (f.filter == torrent->get_state_string());
						break;
				}
				break;
			case EVAL_CONTAINS:
				switch (f.tag)
				{
					case TAG_TRACKER:
						for (unsigned int i = 0; i < info.trackers().size() && !ret; i++)
						{
							Glib::ustring tracker = info.trackers()[i].url;
							ret = (tracker.find(f.filter) != Glib::ustring::npos);
						}
						break;
					case TAG_NAME:
						ret = (info.name().find(f.filter) != Glib::ustring::npos);
						break;
					case TAG_COMMENT:
						ret = (info.comment().find(f.filter) != Glib::ustring::npos);
						break;
					case TAG_STATE:
						ret = (torrent->get_state_string().find(f.filter) != Glib::ustring::npos);
						break;
				}
				break;
			case EVAL_STARTS:
				switch (f.tag)
				{
					case TAG_TRACKER:
						for (unsigned int i = 0; i < info.trackers().size() && !ret; i++)
						{
							Glib::ustring tracker = info.trackers()[i].url;
							ret = Glib::str_has_prefix(tracker, f.filter);
						}
						break;
					case TAG_NAME:
						ret = Glib::str_has_prefix(info.name(), f.filter);
						break;
					case TAG_COMMENT:
						ret = Glib::str_has_prefix(info.comment(), f.filter);
						break;
					case TAG_STATE:
						ret = Glib::str_has_prefix(torrent->get_state_string(), f.filter);
						break;
				}
				break;
			case EVAL_ENDS:
				switch (f.tag)
				{
					case TAG_TRACKER:
						for (unsigned int i = 0; i < info.trackers().size() && !ret; i++)
						{
							Glib::ustring tracker = info.trackers()[i].url;
							ret = Glib::str_has_suffix(tracker, f.filter);
						}
						break;
					case TAG_NAME:
						ret = Glib::str_has_suffix(info.name(), f.filter);
						break;
					case TAG_COMMENT:
						ret = Glib::str_has_suffix(info.comment(), f.filter);
						break;
					case TAG_STATE:
						ret = Glib::str_has_suffix(torrent->get_state_string(), f.filter);
						break;
				}
				break;
		}
	}
	return ret;
}

const Glib::ustring& Group::get_name()
{
	return m_name;
}

const std::list<Group::Filter>& Group::get_filters()
{
	return m_filters;
}
