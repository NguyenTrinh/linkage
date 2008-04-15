/*
Copyright (C) 2007   Christian Lundgren

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

#ifndef MENU_HH
#define MENU_HH

#include <gtkmm/expander.h>
#include <gtkmm/notebook.h>
#include <gtkmm/menu.h>
#include <gtkmm/paned.h>
#include <gtkmm/checkmenuitem.h>

#include <libglademm/xml.h>

#include "AddDialog.hh"
#include "TorrentCreator.hh"
#include "GroupsWin.hh"
#include "SettingsWin.hh"

class Menu : public Gtk::Menu
{
	AddDialog* add_dialog;
	TorrentCreator* new_dialog;
	GroupsWin* groups_win;
	SettingsWin* settings_win;

	Gtk::CheckMenuItem* checkitem_groups;
	Gtk::HPaned* main_hpane;
	Gtk::Expander* expander_details;
	Gtk::Notebook* notebook_details;

	Glib::RefPtr<Gnome::Glade::Xml> glade_xml;

	void on_info();
	void on_about();
	void on_view_groups();
	void on_main_hpane_changed();

public:
	Menu(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& xml);
	~Menu();
};

#endif /* MENU_HH */
