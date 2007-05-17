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

#ifndef TORRENT_MENU_HH
#define TORRENT_MENU_HH

#include <gtkmm/menu.h>
#include <libglademm.h>
class TorrentMenu : public Gtk::Menu
{
	Gtk::Menu* submenu_groups;

	sigc::signal<void> m_signal_open;
	sigc::signal<void> m_signal_info;
	sigc::signal<void> m_signal_up;
	sigc::signal<void> m_signal_down;
	sigc::signal<void> m_signal_start;
	sigc::signal<void> m_signal_stop;
	sigc::signal<void, const Glib::ustring&> m_signal_group;
	sigc::signal<void, bool> m_signal_remove;
	sigc::signal<void> m_signal_check;

	void on_settings();

public:
	sigc::signal<void> signal_open();
	sigc::signal<void> signal_info();
	sigc::signal<void> signal_up();
	sigc::signal<void> signal_down();
	sigc::signal<void> signal_start();
	sigc::signal<void> signal_stop();
	sigc::signal<void, const Glib::ustring&> signal_group();
	sigc::signal<void, bool> signal_remove();
	sigc::signal<void> signal_check();
	
	TorrentMenu(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	~TorrentMenu();
};

#endif /* TORRENT_MENU_HH */
