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

#include "GroupView.hh"
#include <iostream>
GroupView::GroupView()
{
}

GroupView::~GroupView()
{
	for (std::list<GroupRow*>::iterator iter = m_children.begin();
				iter != m_children.end(); ++iter)
	{
		delete *iter;
	}
	m_children.clear();
}

void GroupView::append(GroupRow* row)
{
	m_children.push_back(row);
	pack_start(*row, false, false);
	show_all_children();
}

void GroupView::erase(GroupRow* row)
{
	for (std::list<GroupRow*>::iterator iter = m_children.begin();
				iter != m_children.end(); ++iter)
	{
		if (row->get_name() == (*iter)->get_name())
		{
			m_children.erase(iter);
			remove(*row);
			delete row;
			break;
		}
	}
}

GroupRow* GroupView::get_row(const Glib::ustring& group)
{
	for (std::list<GroupRow*>::iterator iter = m_children.begin();
				iter != m_children.end(); ++iter)
	{
		if (group == (*iter)->get_name())
			return *iter;
	}

	return 0;
}

GroupRow* GroupView::get_selected()
{
	for (std::list<GroupRow*>::iterator iter = m_children.begin();
				iter != m_children.end(); ++iter)
	{
		if ((*iter)->has_focus())
			return *iter;
	}

	return 0;
}

const std::list<GroupRow*>& GroupView::children()
{
	return m_children;
}
