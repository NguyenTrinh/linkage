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

#ifndef TORRENT_MENU_HH
#define TORRENT_MENU_HH

#include <gtkmm/menu.h>
#include <libglademm.h>

#include "Group.hh"

class TorrentMenu : public Gtk::Menu
{
	Glib::RefPtr<Gnome::Glade::Xml> glade_xml;
	Gtk::Menu* submenu_groups;
	Gtk::MenuItem* group_item;
	sigc::signal<void> m_signal_open;
	sigc::signal<void> m_signal_up;
	sigc::signal<void> m_signal_down;
	sigc::signal<void> m_signal_start;
	sigc::signal<void> m_signal_stop;
	sigc::signal<void, const Glib::ustring&> m_signal_group;
	sigc::signal<void, bool> m_signal_remove;
	sigc::signal<void> m_signal_check;
	sigc::signal<void> m_signal_edit_cols;

	void on_groups_changed(const std::list<GroupPtr>& groups);

	friend class UI;

	
public:
	sigc::signal<void> signal_open();
	sigc::signal<void> signal_up();
	sigc::signal<void> signal_down();
	sigc::signal<void> signal_start();
	sigc::signal<void> signal_stop();
	sigc::signal<void, const Glib::ustring&> signal_group();
	sigc::signal<void, bool> signal_remove();
	sigc::signal<void> signal_check();
	sigc::signal<void> signal_edit_columns();
	
	// DIRTY HACK: see UI::on_torrent_list_right_clicked
	Gtk::MenuItem* get_group_menu_item() { return group_item; }
	
	TorrentMenu(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	~TorrentMenu();
};

#endif /* TORRENT_MENU_HH */
