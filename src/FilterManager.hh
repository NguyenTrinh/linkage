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

#ifndef FILTER_MANAGER_HH
#define FILTER_MANAGER_HH

#include "linkage/WeakPtr.hh"
#include "linkage/Torrent.hh"

#include "GroupFilter.hh"

class FilterManager
{
	std::list<GroupFilter*> m_filters;
	void on_settings();

public:
	void check_filters(const WeakPtr<Torrent>& torrent);

	FilterManager();
	~FilterManager();
};

#endif /* FILTER_MANAGER_HH */
