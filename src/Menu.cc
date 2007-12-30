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

#include <gtkmm/main.h>
#include <glibmm/i18n.h>
#include <gtkmm/aboutdialog.h>

#include "linkage/Engine.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/Interface.hh"

#include "Menu.hh"

Menu::Menu(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& xml)
	: Gtk::Menu(cobject), glade_xml(xml)
{
	// get the widgets we need
	glade_xml->get_widget("notebook_details", notebook_details);
	glade_xml->get_widget("expander_details", expander_details);
	glade_xml->get_widget("mnu_view_groups", checkitem_groups);
	glade_xml->get_widget("main_hpane", main_hpane);
	glade_xml->get_widget_derived("new_dialog", new_dialog);
	glade_xml->get_widget_derived("add_dialog", add_dialog);
	glade_xml->get_widget_derived("groups_win", groups_win);
	glade_xml->get_widget_derived("settings_win", settings_win);

	// attach signal handlers
	main_hpane->property_position().signal_changed().connect(
		sigc::mem_fun(this, &Menu::on_main_hpane_changed));
 	glade_xml->connect_clicked
 		("mnu_file_new", sigc::mem_fun(new_dialog, &TorrentCreator::run));
 	glade_xml->connect_clicked
 		("mnu_file_open", sigc::bind(
 			sigc::mem_fun(Linkage::Engine::get_interface(),
		 		&Linkage::Interface::open),
		 	Glib::ustring()));
	glade_xml->connect_clicked
		("mnu_file_quit", sigc::ptr_fun(Gtk::Main::quit));
	glade_xml->connect_clicked
		("mnu_view_details", sigc::mem_fun(this, &Menu::on_info));
	glade_xml->connect_clicked
		("mnu_view_groups", sigc::mem_fun(this, &Menu::on_view_groups));
	glade_xml->connect_clicked
		("mnu_edit_groups", sigc::mem_fun(groups_win, &GroupsWin::show));
	glade_xml->connect_clicked
		("mnu_view_prefs", sigc::mem_fun(settings_win, &SettingsWin::show));
 	glade_xml->connect_clicked
 		("mnu_help_about", sigc::mem_fun(this, &Menu::on_about));
}

Menu::~Menu()
{
}

void Menu::on_info()
{
	if (expander_details->is_sensitive())
	{
		expander_details->set_expanded(true);
		notebook_details->set_current_page(0);
	}
}

void Menu::on_view_groups()
{
	if (checkitem_groups->get_active())
	{
		int pos = Linkage::Engine::get_settings_manager()->get_int("ui/groups_width");
		main_hpane->set_position(pos);
	}
	else
		main_hpane->set_position(0);
}

void Menu::on_about()
{
	Gtk::AboutDialog about;
	std::list<Glib::ustring> people;
	people.push_back("Christian Lundgren");
	people.push_back("Dave Moore");
	about.set_authors(people);
	people.clear();
	people.push_back("Brian William Davis");
	people.push_back("Ludvig Aleman");
	about.set_artists(people);
	people.clear();
	about.set_comments(_("A BitTorrent client"));
	try
	{
		about.set_logo(Gdk::Pixbuf::create_from_file(PIXMAP_DIR "/linkage.svg"));
	}
	catch (...) {}
	about.set_version(PACKAGE_VERSION);
	about.set_copyright("Copyright \u00A9 2006-2007 Christian Lundgren, Dave Moore");
	about.set_website("http://code.google.com/p/linkage");
	about.set_name("Linkage");
	about.run();
}

void Menu::on_main_hpane_changed()
{
	checkitem_groups->set_active(main_hpane->get_position());
}


