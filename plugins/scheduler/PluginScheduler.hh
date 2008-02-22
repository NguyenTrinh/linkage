/*
Copyright (C) 2008	Dave Moore

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

#ifndef PLUGIN_SCHEDULER_HH
#define PLUGIN_SCHEDULER_HH

#include <gtkmm/dialog.h>

#include "linkage/Plugin.hh"

#include "SchedulerConfig.hh"
#include "ScheduleWin.hh"


class SchedulerPlugin : public Linkage::Plugin
{
	SchedulerConfig m_config;
	Glib::ustring m_config_file;
	
	sigc::slot<bool> m_next_hour_slot;
	sigc::connection m_next_hour_conn;
	sigc::slot<bool> m_hourly_slot;
	sigc::connection m_hourly_conn;
	
	ScheduleWin* m_dialog;
	
	bool on_next_hour();
	bool on_every_hour();
	
	void apply_limits();
	void save_config(const SchedulerConfig & cfg);
	
	void on_dialog_response(int response);
public:
	Linkage::Plugin::Info get_info();
	//void configure();
	Gtk::Dialog* get_config_dialog();
	SchedulerPlugin();
	~SchedulerPlugin();
};

//typedef std::pair<int, int>

#endif // PLUGIN_SCHEDULER_HH
