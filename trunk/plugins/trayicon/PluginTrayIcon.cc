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

#include "config.h"

#include <sstream>

#include <gtkmm/stock.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/separatormenuitem.h>
#include <glibmm/i18n.h>

#include <gtk/gtkstatusicon.h>
#include <gtk/gtkmenu.h>

#include "PluginTrayIcon.hh"

#include "linkage/Engine.hh"
#include "linkage/SessionManager.hh"
#include "linkage/TorrentManager.hh"
#include "linkage/Interface.hh"
#include "linkage/Utils.hh"

using namespace Linkage;

#define PLUGIN_NAME		"TrayPlugin"
#define PLUGIN_DESC		_("Displays a tray icon")
#define PLUGIN_VER		PACKAGE_VERSION
#define PLUGIN_AUTHOR	"Christian Lundgren"
#define PLUGIN_WEB		"http://code.google.com/p/linkage"

TrayPlugin::TrayPlugin()
{
	menu = new Gtk::Menu();
	Gtk::ImageMenuItem* item = Gtk::manage(new Gtk::ImageMenuItem(_("Start torrents")));
	Gtk::Image* image = Gtk::manage(new Gtk::Image(Gtk::Stock::APPLY, Gtk::ICON_SIZE_MENU));
	item->set_image(*image);
	item->signal_activate().connect(sigc::mem_fun(this, &TrayPlugin::on_torrents_start));
	menu->append(*item);
	item = Gtk::manage(new Gtk::ImageMenuItem(_("Stop torrents")));
	image = Gtk::manage(new Gtk::Image(Gtk::Stock::STOP, Gtk::ICON_SIZE_MENU));
	item->set_image(*image);
	item->signal_activate().connect(sigc::mem_fun(this, &TrayPlugin::on_torrents_stop));
	menu->append(*item);
	Gtk::SeparatorMenuItem* separator = Gtk::manage(new Gtk::SeparatorMenuItem());
	menu->append(*separator);
	Gtk::ImageMenuItem* imageitem = Gtk::manage(new Gtk::ImageMenuItem(Gtk::Stock::QUIT));
	imageitem->signal_activate().connect(sigc::mem_fun(this, &TrayPlugin::on_quit));
	menu->append(*imageitem);

	menu->show_all_children();

	icon = Gtk::StatusIcon::create_from_file(PIXMAP_DIR "/linkage.svg");
	// FIXME: use gtkmm 2.12 for this
	GtkStatusIcon* gobj = icon->gobj();
	g_signal_connect(G_OBJECT(gobj), "activate", G_CALLBACK(TrayPlugin::on_activate), NULL);
	g_signal_connect(G_OBJECT(gobj), "popup-menu", G_CALLBACK(TrayPlugin::on_popup), menu);

	Engine::signal_tick().connect(sigc::mem_fun(this, &TrayPlugin::on_tick));
}

TrayPlugin::~TrayPlugin()
{
	delete menu;
}
	
Plugin::Info TrayPlugin::get_info()
{
	return Plugin::Info(PLUGIN_NAME,
		PLUGIN_DESC,
		PLUGIN_VER,
		PLUGIN_AUTHOR,
		PLUGIN_WEB,
		false,
		Plugin::PARENT_NONE);
}

void TrayPlugin::on_activate(GtkStatusIcon* status_icon, gpointer data)
{
	Interface& ui = Engine::get_interface();
	ui.set_visible(!ui.get_visible());
}

void TrayPlugin::on_popup(GtkStatusIcon* status_icon, guint button, guint time, gpointer data)
{
	Gtk::Menu* menu = static_cast<Gtk::Menu*>(data);
	// FIXME: use gtkmm 2.12 for this
	gtk_menu_popup(menu->gobj(), NULL, NULL, gtk_status_icon_position_menu, status_icon, button, time);
}

void TrayPlugin::on_tick()
{
	// GtkStatusIcon doesn't provide any kind of enter-notify event,
	// so we have to update the tooltip like this..
	TorrentManager::TorrentList torrents = Engine::get_torrent_manager()->get_torrents();
	unsigned int num_active = 0, num_queued = 0, num_seeds = 0;
	for (TorrentManager::TorrentList::iterator iter = torrents.begin(); iter != torrents.end(); ++iter)
	{
		Glib::RefPtr<Torrent> torrent = *iter;
		int state = torrent->get_state();

		if (state & Torrent::SEEDING)
			num_seeds++;
		else if (!(state & Torrent::STOPPED))
		{
			if (state & (Torrent::DOWNLOADING | Torrent::ANNOUNCING | Torrent::FINISHED))
				num_active++;
			else
				num_queued++;
		}
	}
	
	libtorrent::session_status status = Engine::get_session_manager()->status();

	Glib::ustring tip = String::ucompose(_(
		"%1 (%2) downloads, %3 seeds\n"
		"DL: %4/s\tUL: %5/s"),
		num_active, num_queued, num_seeds,
		suffix_value(status.payload_download_rate),
		suffix_value(status.payload_upload_rate));

	icon->set_tooltip(tip);
}

void TrayPlugin::on_quit()
{
	Engine::get_interface().quit();
}

void TrayPlugin::on_torrents_stop()
{
	TorrentManager::TorrentList torrents = Engine::get_torrent_manager()->get_torrents();
	for (TorrentManager::TorrentList::iterator iter = torrents.begin();
				iter != torrents.end(); ++iter)
	{
		Glib::RefPtr<Torrent> torrent = *iter;
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
		Glib::RefPtr<Torrent> torrent = *iter;
		if (torrent->is_stopped())
			Engine::get_session_manager()->resume_torrent(torrent->get_hash());
	}
}

Plugin* create_plugin()
{
	 return new TrayPlugin();
}

Plugin::Info plugin_info()
{
	return Plugin::Info(PLUGIN_NAME,
		PLUGIN_DESC,
		PLUGIN_VER,
		PLUGIN_AUTHOR,
		PLUGIN_WEB,
		false,
		Plugin::PARENT_NONE);
}

