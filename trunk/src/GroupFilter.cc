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

#include "GroupFilter.hh"


GroupFilter::GroupFilter(const Glib::ustring& filter, TagType tag, EvalType eval, const Glib::ustring& name)
{
	m_filter = filter;
	m_tag = tag;
	m_eval = eval;
	m_name = name;
}

GroupFilter::~GroupFilter()
{
}

bool GroupFilter::eval(const WeakPtr<Torrent>& torrent)
{
	torrent_info info = torrent->get_info();

	switch (m_eval)
	{
		case EVAL_EQUALS:
			switch (m_tag)
			{
				case TAG_TRACKER:
					for (unsigned int i = 0; i < info.trackers().size(); i++)
					{
						Glib::ustring tracker = info.trackers()[i].url;
						if (m_filter == tracker)
							return true;
					}
					return false;
				case TAG_NAME:
					return (m_filter == info.name());
				case TAG_COMMENT:
					return (m_filter == info.comment());
				case TAG_STATE:
					return (m_filter == torrent->get_state_string());
			}
			break;
		case EVAL_CONTAINS:
			switch (m_tag)
			{
				case TAG_TRACKER:
					for (unsigned int i = 0; i < info.trackers().size(); i++)
					{
						Glib::ustring tracker = info.trackers()[i].url;
						if (tracker.find(m_filter, 0) != Glib::ustring::npos)
							return true;
					}
					return false;
				case TAG_NAME:
					return (info.name().find(m_filter, 0) != Glib::ustring::npos);
				case TAG_COMMENT:
					return (info.comment().find(m_filter, 0) != Glib::ustring::npos);
				case TAG_STATE:
					return (torrent->get_state_string().find(m_filter, 0) != Glib::ustring::npos);
			}
			break;
		case EVAL_STARTS:
			switch (m_tag)
			{
				case TAG_TRACKER:
					for (unsigned int i = 0; i < info.trackers().size(); i++)
					{
						Glib::ustring tracker = info.trackers()[i].url;
						if (tracker.substr(0, m_filter.size()) == m_filter)
							return true;
					}
					return false;
				case TAG_NAME:
					return (info.name().substr(0, m_filter.size()) == m_filter);
				case TAG_COMMENT:
					return (info.comment().substr(0, m_filter.size()) == m_filter);
				case TAG_STATE:
					return (torrent->get_state_string().substr(0, m_filter.size()) == m_filter);
			}
			break;
		case EVAL_ENDS:
			switch (m_tag)
			{
				case TAG_TRACKER:
					for (unsigned int i = 0; i < info.trackers().size(); i++)
					{
						Glib::ustring tracker = info.trackers()[i].url;
						if (tracker.substr(tracker.size()-m_filter.size(), tracker.size()) == m_filter)
							return true;
					}
					return false;
				case TAG_NAME:
					return (info.name().substr(info.name().size()-m_filter.size(), info.name().size()) == m_filter);
				case TAG_COMMENT:
					return (info.comment().substr(info.comment().size()-m_filter.size(), info.comment().size()) == m_filter);
				case TAG_STATE:
					Glib::ustring state = torrent->get_state_string();
					return (state.substr(state.size() - m_filter.size(), state.size()) == m_filter);
			}
			break;
	}
}

const Glib::ustring& GroupFilter::get_name()
{
	return m_name;
}

const Glib::ustring& GroupFilter::get_filter()
{
	return m_filter;
}

const GroupFilter::TagType GroupFilter::get_tag()
{
	return m_tag;
}

const GroupFilter::EvalType GroupFilter::get_eval()
{
	return m_eval;
}
