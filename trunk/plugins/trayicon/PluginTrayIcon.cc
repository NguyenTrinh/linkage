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

#include "PluginTrayIcon.hh"
#include "linkage/Engine.hh"

#include <gtkmm/menuitem.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/separatormenuitem.h>

TrayPlugin::TrayPlugin()
{
}

TrayPlugin::~TrayPlugin()
{
	delete menu;
	delete image;
	delete eventbox;
	delete tooltips;
}

Glib::ustring TrayPlugin::get_name()
{
	return "TrayPlugin";
}

Glib::ustring TrayPlugin::get_description()
{
	return "Displays a tray icon";
}
	
void TrayPlugin::on_load()
{
	Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file(PIXMAP_DIR "/linkage.png", 24, 24, true);
	image = new Gtk::Image(pixbuf);
	
	eventbox = new Gtk::EventBox();;
	eventbox->add_events(Gdk::BUTTON_RELEASE_MASK);
	eventbox->signal_button_release_event().connect(sigc::mem_fun(this, &TrayPlugin::on_button_released));
	eventbox->add(*image);
	eventbox->show_all_children();

	tooltips = new Gtk::Tooltips();

	Gtk::Widget* widget = dynamic_cast<Gtk::Widget*>(eventbox);

	tray_icon = egg_tray_icon_new ("TrayPlugin");
	gtk_container_add (GTK_CONTAINER (tray_icon), widget->gobj());
	gtk_widget_show_all (GTK_WIDGET (tray_icon));
	
	menu = new Gtk::Menu();
	Gtk::MenuItem* item = Gtk::manage(new Gtk::MenuItem("Start torrents"));
	item->signal_activate().connect(sigc::mem_fun(this, &TrayPlugin::on_torrents_start));
	menu->append(*item);
	item = Gtk::manage(new Gtk::MenuItem("Stop torrents"));
	item->signal_activate().connect(sigc::mem_fun(this, &TrayPlugin::on_torrents_stop));
	menu->append(*item);
	Gtk::SeparatorMenuItem* separator = Gtk::manage(new Gtk::SeparatorMenuItem());
	menu->append(*separator);
	Gtk::ImageMenuItem* imageitem = Gtk::manage(new Gtk::ImageMenuItem(Gtk::Stock::QUIT));
	imageitem->signal_activate().connect(sigc::mem_fun(this, &TrayPlugin::on_quit));
	menu->append(*imageitem);

	Engine::signal_tick().connect(sigc::mem_fun(this, &TrayPlugin::on_tick));
	menu->show_all_children();
}

bool TrayPlugin::on_button_released(GdkEventButton* e)
{
	if (e->button == 1)
	{
		Engine::get_dbus_manager()->send("ToggleVisible");
	}
	else if (e->button == 3)
	{
		menu->popup(e->button, e->time);
	}
}

void TrayPlugin::on_tick()
{
	TorrentManager::TorrentList torrents = Engine::get_torrent_manager()->get_torrents();
	unsigned int num_active = 0, num_seeds = 0;
	for (TorrentManager::TorrentList::iterator iter = torrents.begin(); iter != torrents.end(); ++iter)
	{
		WeakPtr<Torrent> torrent = *iter;
		if (torrent->get_state() == Torrent::SEEDING)
			num_seeds++;
		else if (torrent->get_state() != Torrent::STOPPED)
			num_active++;
	}
	
	session_status status = Engine::get_session_manager()->status();
	
	tooltips->set_tip(*eventbox, str(num_active) + " downloads, " + str(num_seeds) + " seeds\nDL: " + suffix_value(status.payload_download_rate) + "/s\tUL:" + suffix_value(status.payload_upload_rate) + "/s");
}

void TrayPlugin::on_quit()
{
	Engine::get_dbus_manager()->send("Quit");
}

void TrayPlugin::on_torrents_stop()
{
	/* FIXME: Poll TorrentManager */
}

void TrayPlugin::on_torrents_start()
{
	/* FIXME: Poll TorrentManager */
}

Plugin * CreatePlugin()
{
	 return new TrayPlugin();
}
