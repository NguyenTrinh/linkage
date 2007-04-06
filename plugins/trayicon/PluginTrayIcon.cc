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

#include <sstream>

#include <gtkmm/menuitem.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/separatormenuitem.h>

#include "PluginTrayIcon.hh"
#include "linkage/Engine.hh"

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
	Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file(PIXMAP_DIR "/linkage.svg", 24, 24, true);
	image = new Gtk::Image(pixbuf);
	
	eventbox = new Gtk::EventBox();;
	eventbox->add_events(Gdk::BUTTON_RELEASE_MASK | Gdk::ENTER_NOTIFY_MASK);
	eventbox->signal_button_release_event().connect(sigc::mem_fun(this, &TrayPlugin::on_button_released));
	eventbox->add(*image);
	eventbox->show_all_children();

	tooltips = new Gtk::Tooltips();

	Gtk::Widget* widget = dynamic_cast<Gtk::Widget*>(eventbox);

	tray_icon = egg_tray_icon_new("TrayPlugin");
	gtk_container_add(GTK_CONTAINER(tray_icon), widget->gobj());
	gtk_widget_show_all(GTK_WIDGET(tray_icon));
	
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

	eventbox->signal_enter_notify_event().connect(sigc::mem_fun(this, &TrayPlugin::on_update_tooltip));

	menu->show_all_children();
}

bool TrayPlugin::on_button_released(GdkEventButton* event)
{
	if (event->button == 1)
	{
		Engine::get_dbus_manager()->send("ToggleVisible");
	}
	else if (event->button == 3)
	{
		menu->popup(event->button, event->time);
	}
}

bool TrayPlugin::on_update_tooltip(GdkEventCrossing* event)
{
	TorrentManager::TorrentList torrents = Engine::get_torrent_manager()->get_torrents();
	unsigned int num_active = 0, num_queued = 0, num_seeds = 0;
	for (TorrentManager::TorrentList::iterator iter = torrents.begin(); iter != torrents.end(); ++iter)
	{
		WeakPtr<Torrent> torrent = *iter;
		Torrent::State state = torrent->get_state();

		if (state == Torrent::SEEDING)
			num_seeds++;
		else if (state != Torrent::STOPPED)
		{
			if (state == Torrent::DOWNLOADING || state == Torrent::ANNOUNCING || state == Torrent::FINISHED)
				num_active++;
			else
				num_queued++;
		}
	}
	
	session_status status = Engine::get_session_manager()->status();

	std::stringstream ss;
	ss << num_active << " (" << num_queued << ") downloads, "
		<< num_seeds << " seeds\nDL: " << suffix_value(status.payload_download_rate)
		<< "/s\tUL:" << suffix_value(status.payload_upload_rate) + "/s";

	tooltips->set_tip(*eventbox, ss.str());

	return false;
}

void TrayPlugin::on_quit()
{
	Engine::get_dbus_manager()->send("Quit");
}

void TrayPlugin::on_torrents_stop()
{
	TorrentManager::TorrentList torrents = Engine::get_torrent_manager()->get_torrents();
	for (TorrentManager::TorrentList::iterator iter = torrents.begin();
				iter != torrents.end(); ++iter)
	{
		WeakPtr<Torrent> torrent = *iter;
		if (!torrent->is_stopped())
			Engine::get_session_manager()->stop_torrent(torrent->get_hash());
	}
}

void TrayPlugin::on_torrents_start()
{
	TorrentManager::TorrentList torrents = Engine::get_torrent_manager()->get_torrents();
	for (TorrentManager::TorrentList::iterator iter = torrents.begin();
				iter != torrents.end(); ++iter)
	{
		WeakPtr<Torrent> torrent = *iter;
		if (torrent->is_stopped())
			Engine::get_session_manager()->resume_torrent(torrent->get_hash());
	}
}

Plugin* CreatePlugin()
{
	 return new TrayPlugin();
}
