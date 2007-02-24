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

#include <glibmm/refptr.h>

#include "linkage/AlertManager.hh"
#include "linkage/PluginManager.hh"
#include "linkage/SessionManager.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/TorrentManager.hh"
#include "linkage/DbusManager.hh"
#include "linkage/RefCounter.hh"

class Engine : public RefCounter<Engine>
{
	static Glib::RefPtr<SettingsManager> ssm;
	static Glib::RefPtr<TorrentManager> tm;
	static Glib::RefPtr<SessionManager> sm;
	static Glib::RefPtr<AlertManager> am;
	static Glib::RefPtr<PluginManager> pm;
	static Glib::RefPtr<DbusManager> dbm;
	
	static Glib::RefPtr<Engine> self;
	
	sigc::signal<void> m_signal_tick;
	sigc::connection m_conn_tick;

	bool on_timeout();
	
	Engine();
	
public:
	Glib::RefPtr<AlertManager> get_alert_manager();
	Glib::RefPtr<PluginManager> get_plugin_manager();
	Glib::RefPtr<SessionManager> get_session_manager();
	Glib::RefPtr<SettingsManager>	get_settings_manager();
	Glib::RefPtr<TorrentManager>	get_torrent_manager();
	Glib::RefPtr<DbusManager>	get_dbus_manager();
	
	sigc::signal<void> signal_tick();
	bool is_ticking();
	void start_tick();
	void stop_tick();

	static bool is_primary();
	static Glib::RefPtr<Engine> instance();
	static void destroy();
	
	~Engine();
};
