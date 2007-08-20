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

#ifndef ENGINE_HH
#define ENGINE_HH

#include <glibmm/refptr.h>

#include <sigc++/signal.h>

#include "linkage/RefCounter.hh"
#include "linkage/WeakPtr.hh"

class SettingsManager;
class TorrentManager;
class SessionManager;
class AlertManager;
class PluginManager;
class DbusManager;
class Interface;

class Engine : public RefCounter<Engine>
{

	static Glib::RefPtr<SettingsManager> ssm;
	static Glib::RefPtr<TorrentManager> tm;
	static Glib::RefPtr<SessionManager> sm;
	static Glib::RefPtr<AlertManager> am;
	static Glib::RefPtr<PluginManager> pm;
	static Glib::RefPtr<DbusManager> dbm;
	static Interface* m_interface;

	static Glib::RefPtr<Engine> self;
	
	static sigc::signal<void> m_signal_tick;

	bool on_timeout();
	
	static void init();
	Engine();
	
public:
	static Glib::RefPtr<AlertManager> get_alert_manager();
	static Glib::RefPtr<PluginManager> get_plugin_manager();
	static Glib::RefPtr<SessionManager> get_session_manager();
	static Glib::RefPtr<SettingsManager>	get_settings_manager();
	static Glib::RefPtr<TorrentManager>	get_torrent_manager();
	static Glib::RefPtr<DbusManager>	get_dbus_manager();
	static WeakPtr<Interface> get_interface();
	static void register_interface(Interface* interface);

	static sigc::signal<void> signal_tick();

	static bool is_primary();
	static void destroy();
	
	~Engine();
};

#endif /* ENGINE_HH */

