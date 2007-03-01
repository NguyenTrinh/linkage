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

#include <gtkmm/menuitem.h>
#include <gtkmm/separatormenuitem.h>

#include "linkage/Engine.hh"
#include "linkage/SettingsManager.hh"

#include "TorrentMenu.hh"
#include "AlignedLabel.hh"

TorrentMenu::TorrentMenu()
{
	/* FIXME: add pretty icons */
	Gtk::MenuItem* item = Gtk::manage(new Gtk::MenuItem("Open location"));
	item->signal_activate().connect(sigc::mem_fun(&m_signal_open, &sigc::signal<void>::emit));
	append(*item);
	item = Gtk::manage(new Gtk::MenuItem("Information"));
	item->signal_activate().connect(sigc::mem_fun(&m_signal_info, &sigc::signal<void>::emit));
	append(*item);
	append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
	item = Gtk::manage(new Gtk::MenuItem("Move up"));
	item->signal_activate().connect(sigc::mem_fun(&m_signal_up, &sigc::signal<void>::emit));
	append(*item);
	item = Gtk::manage(new Gtk::MenuItem("Move down"));
	item->signal_activate().connect(sigc::mem_fun(&m_signal_down, &sigc::signal<void>::emit));
	append(*item);
	append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
	item = Gtk::manage(new Gtk::MenuItem("Start"));
	item->signal_activate().connect(sigc::mem_fun(&m_signal_start, &sigc::signal<void>::emit));
	append(*item);
	item = Gtk::manage(new Gtk::MenuItem("Stop"));
	item->signal_activate().connect(sigc::mem_fun(&m_signal_stop, &sigc::signal<void>::emit));
	append(*item);
	append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
	submenu_groups = Gtk::manage(new Gtk::Menu());
	item = Gtk::manage(new Gtk::MenuItem("Group"));
	item->set_submenu(*submenu_groups);
	append(*item);
	append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
	Gtk::Menu* submenu_remove = Gtk::manage(new Gtk::Menu());
	item = Gtk::manage(new Gtk::MenuItem("Remove torrent"));
	item->signal_activate().connect(sigc::bind(sigc::mem_fun(&m_signal_remove, &sigc::signal<void, bool>::emit), false));
	submenu_remove->append(*item);
	item = Gtk::manage(new Gtk::MenuItem("Remove torrent + content"));
	item->signal_activate().connect(sigc::bind(sigc::mem_fun(&m_signal_remove, &sigc::signal<void, bool>::emit), true));
	submenu_remove->append(*item);
	item = Gtk::manage(new Gtk::MenuItem("Remove"));
	item->set_submenu(*submenu_remove);
	append(*item);
	append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
	item = Gtk::manage(new Gtk::MenuItem("Check files"));
	item->signal_activate().connect(sigc::mem_fun(&m_signal_check, &sigc::signal<void>::emit));
	append(*item);
	
	on_settings();

	Engine::get_settings_manager()->signal_update_settings().connect(sigc::mem_fun(this, &TorrentMenu::on_settings));

	show_all_children();
}

TorrentMenu::~TorrentMenu()
{
}

void TorrentMenu::on_settings()
{
	std::list<Gtk::Widget*> children = submenu_groups->get_children();
	for (std::list<Gtk::Widget*>::iterator iter = children.begin();
				iter != children.end(); ++iter)
	{
		Gtk::Widget* widget = *iter;
		submenu_groups->remove(*widget);
		delete widget;
	}
	
	AlignedLabel* label = Gtk::manage(new AlignedLabel());
	label->set_markup("<i>None</i>");
	Gtk::MenuItem* item = Gtk::manage(new Gtk::MenuItem(*label));
	item->signal_activate().connect(sigc::bind(sigc::mem_fun(&m_signal_group, &sigc::signal<void, const Glib::ustring&>::emit), ""));
	submenu_groups->append(*item);
	submenu_groups->append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
	
	std::list<Glib::ustring> groups = Engine::get_settings_manager()->get_keys("Groups");
	for (std::list<Glib::ustring>::iterator iter = groups.begin();
				iter != groups.end(); ++iter)
	{
		item = Gtk::manage(new Gtk::MenuItem(*iter));
		item->signal_activate().connect(sigc::bind(sigc::mem_fun(&m_signal_group, &sigc::signal<void, const Glib::ustring&>::emit), *iter));
		submenu_groups->append(*item);
	}
}

sigc::signal<void> TorrentMenu::signal_open()
{
	return m_signal_open;
}

sigc::signal<void> TorrentMenu::signal_info()
{
	return m_signal_info;
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
