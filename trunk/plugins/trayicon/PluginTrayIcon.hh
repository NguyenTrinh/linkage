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

#include <gdkmm/types.h>
#include <gtkmm/image.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/tooltips.h>
#include <gtkmm/menu.h>
#include <gtkmm/stock.h>

#include "eggtrayicon.h"
#include "linkage/Plugin.hh"

class TrayPlugin : public Plugin
{
	EggTrayIcon *tray_icon;
	Gtk::Widget* widget;
	Gtk::EventBox* eventbox;
	Gtk::Tooltips* tooltips;
	Gtk::Image* image;
	Gtk::Menu* menu;

	bool on_button_released(GdkEventButton* event);
	bool on_update_tooltip(GdkEventCrossing* event);
	void on_quit();
	void on_torrents_stop();
	void on_torrents_start();
	
public:
	Glib::ustring get_name();
	Glib::ustring get_description();
	
	PluginParent get_parent() { return Plugin::PARENT_NONE; };
	Gtk::Widget* get_widget() { return NULL; };
	
	void on_load();
	
	TrayPlugin();
	~TrayPlugin();
};

#endif /* PLUGIN_TRAYICON_HH */
