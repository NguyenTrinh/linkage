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

#include "linkage/Engine.hh"

Glib::RefPtr<Engine> Engine::self					= Glib::RefPtr<Engine>();

Glib::RefPtr<SettingsManager> Engine::ssm	= Glib::RefPtr<SettingsManager>();
Glib::RefPtr<TorrentManager> Engine::tm		= Glib::RefPtr<TorrentManager>();
Glib::RefPtr<SessionManager> Engine::sm		= Glib::RefPtr<SessionManager>();
Glib::RefPtr<AlertManager> Engine::am			= Glib::RefPtr<AlertManager>();
Glib::RefPtr<PluginManager> Engine::pm		 = Glib::RefPtr<PluginManager>();
Glib::RefPtr<DbusManager> Engine::dbm			= Glib::RefPtr<DbusManager>();

Glib::RefPtr<Engine> Engine::instance()
{
	static bool creating = false;
	if (!self && is_primary() && !creating)
	{
		creating = true;
		self = Glib::RefPtr<Engine>(new Engine());
		creating = false;
	}

	return self;
}

bool Engine::is_primary()
{
	if (!dbm)
		dbm = DbusManager::create();
	
	return dbm->is_primary();
}

Engine::Engine() : RefCounter<Engine>::RefCounter(this)
{
	ssm = SettingsManager::create();
	sm = SessionManager::create();
	am = AlertManager::create();
	tm = TorrentManager::create();
	pm = PluginManager::create();
}

Engine::~Engine()
{
}

Glib::RefPtr<AlertManager> Engine::get_alert_manager()
{
	return am;
}

Glib::RefPtr<PluginManager> Engine::get_plugin_manager()
{
	return pm;
}

Glib::RefPtr<SessionManager> Engine::get_session_manager()
{
	return sm;
}

Glib::RefPtr<SettingsManager> Engine::get_settings_manager()
{
	return ssm;
}

Glib::RefPtr<TorrentManager> Engine::get_torrent_manager()
{
	return tm;
}

Glib::RefPtr<DbusManager> Engine::get_dbus_manager()
{
	return dbm;
}
