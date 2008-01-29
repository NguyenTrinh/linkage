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

#include <gtkmm/stock.h>
#include <gtkmm/image.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/label.h>
#include <glibmm/i18n.h>

#include <libtorrent/entry.hpp>

#include "linkage/Engine.hh"
#include "linkage/SettingsManager.hh"

#include "GroupsWin.hh"
#include "TorrentMenu.hh"

using namespace Linkage;

TorrentMenu::TorrentMenu(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::Menu(cobject),
	glade_xml(refGlade)
{
	submenu_groups = Gtk::manage(new Gtk::Menu());
	Gtk::MenuItem* item;
	glade_xml->get_widget("torrent_menu_groups", item);
	item->set_submenu(*submenu_groups);

	/* Connect update signal */
	GroupsWin* groups_win;
	glade_xml->get_widget_derived("groups_win", groups_win);
	groups_win->signal_groups_changed().connect(sigc::mem_fun(this, &TorrentMenu::on_groups_changed));

	glade_xml->connect_clicked
		("torrent_menu_open", sigc::mem_fun(&m_signal_open, &sigc::signal<void>::emit));
	glade_xml->connect_clicked
		("torrent_menu_up", sigc::mem_fun(&m_signal_up, &sigc::signal<void>::emit));
	glade_xml->connect_clicked
		("torrent_menu_down", sigc::mem_fun(&m_signal_down, &sigc::signal<void>::emit));
	glade_xml->connect_clicked
		("torrent_menu_start", sigc::mem_fun(&m_signal_start, &sigc::signal<void>::emit));
	glade_xml->connect_clicked
		("torrent_menu_stop", sigc::mem_fun(&m_signal_stop, &sigc::signal<void>::emit));
	glade_xml->connect_clicked
		("torrent_menu_remove", sigc::bind(sigc::mem_fun
		(&m_signal_remove, &sigc::signal<void, bool>::emit), false));
	glade_xml->connect_clicked
		("torrent_menu_remove_content", sigc::bind(sigc::mem_fun
		(&m_signal_remove, &sigc::signal<void, bool>::emit), true));
	glade_xml->connect_clicked
		("torrent_menu_check", sigc::mem_fun(&m_signal_check, &sigc::signal<void>::emit));
	glade_xml->connect_clicked
		("torrent_menu_edit_cols", sigc::mem_fun(&m_signal_edit_cols, &sigc::signal<void>::emit));

	show_all_children();
}

TorrentMenu::~TorrentMenu()
{
}

void TorrentMenu::on_groups_changed(const std::list<GroupPtr>& groups)
{
	std::list<Gtk::Widget*> children = submenu_groups->get_children();
	for (std::list<Gtk::Widget*>::iterator iter = children.begin();
		iter != children.end(); ++iter)
	{
		Gtk::Widget* widget = *iter;
		submenu_groups->remove(*widget);
		delete widget;
	}
	
	Gtk::Label* label = Gtk::manage(new Gtk::Label());
	label->set_markup(_("<i>None</i>"));
	Gtk::MenuItem* item = Gtk::manage(new Gtk::MenuItem(*label));
	item->signal_activate().connect(sigc::bind(sigc::mem_fun(&m_signal_group, &sigc::signal<void, const Glib::ustring&>::emit), ""));
	submenu_groups->append(*item);
	submenu_groups->append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
	
	for (std::list<GroupPtr>::const_iterator iter = groups.begin();
		iter != groups.end(); ++iter)
	{
		GroupPtr group = *iter;
		item = Gtk::manage(new Gtk::MenuItem(group->get_name()));
		item->signal_activate().connect(sigc::bind(sigc::mem_fun(&m_signal_group, &sigc::signal<void, const Glib::ustring&>::emit), group->get_name()));
		submenu_groups->append(*item);
	}

	submenu_groups->show_all_children();
}

sigc::signal<void> TorrentMenu::signal_open()
{
	return m_signal_open;
}

sigc::signal<void> TorrentMenu::signal_up()
{
	return m_signal_up;
}

sigc::signal<void> TorrentMenu::signal_down()
{
	return m_signal_down;
}

sigc::signal<void> TorrentMenu::signal_start()
{
	return m_signal_start;
}

sigc::signal<void> TorrentMenu::signal_stop()
{
	return m_signal_stop;
}

sigc::signal<void, const Glib::ustring&> TorrentMenu::signal_group()
{
	return m_signal_group;
}

sigc::signal<void, bool> TorrentMenu::signal_remove()
{
	return m_signal_remove;
}

sigc::signal<void> TorrentMenu::signal_check()
{
	return m_signal_check;
}
sigc::signal<void> TorrentMenu::signal_edit_columns()
{
	return m_signal_edit_cols;
}
