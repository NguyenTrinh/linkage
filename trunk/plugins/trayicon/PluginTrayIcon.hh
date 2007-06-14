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

#ifndef PLUGIN_TRAYICON_HH
#define PLUGIN_TRAYICON_HH

#include <gtkmm/statusicon.h>
#include <gtkmm/menu.h>

#include "linkage/Plugin.hh"

class TrayPlugin : public Plugin
{
	Glib::RefPtr<Gtk::StatusIcon> icon;
	Gtk::Menu* menu;

	static void on_activate(GtkStatusIcon* status_icon, gpointer data);
	static void on_popup(GtkStatusIcon* status_icon, guint button, guint time, gpointer data);

	void on_tick();
	void on_quit();
	void on_torrents_stop();
	void on_torrents_start();
	
public:
	gpointer get_user_data(gpointer arg = NULL) { return icon->gobj(); }
	
	Plugin::Info get_info();
	
	TrayPlugin();
	~TrayPlugin();
};

#endif /* PLUGIN_TRAYICON_HH */
