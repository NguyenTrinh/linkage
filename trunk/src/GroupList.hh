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

#ifndef GROUP_LIST_HH
#define GROUP_LIST_HH

#include <map>

#include <gtkmm/box.h>
#include <gtkmm/radiobutton.h>

#include "linkage/WeakPtr.hh"
#include "linkage/Torrent.hh"

#include "GroupFilter.hh"

class GroupList : public Gtk::VBox
{
	typedef std::map<GroupFilter*, Gtk::RadioButton*> GroupMap;

	Gtk::RadioButton* m_all;

	GroupMap m_map;
	
	sigc::signal<void, const GroupFilter&> m_signal_filter_set;
	sigc::signal<void> m_signal_filter_unset;

	void on_all_toggled();
	void on_group_toggled(GroupFilter* group);

	void on_settings();

public:
	sigc::signal<void, const GroupFilter&> signal_filter_set();
	sigc::signal<void> signal_filter_unset();

	void update();

	GroupList();
	~GroupList();
};

#endif /* GROUP_LIST_HH */
