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

#include <glibmm/main.h>

#include "linkage/Engine.hh"

Glib::RefPtr<Engine> Engine::self					= Glib::RefPtr<Engine>();

Glib::RefPtr<SettingsManager> Engine::ssm	= Glib::RefPtr<SettingsManager>();
Glib::RefPtr<TorrentManager> Engine::tm		= Glib::RefPtr<TorrentManager>();
Glib::RefPtr<SessionManager> Engine::sm		= Glib::RefPtr<SessionManager>();
Glib::RefPtr<AlertManager> Engine::am			= Glib::RefPtr<AlertManager>();
Glib::RefPtr<PluginManager> Engine::pm		= Glib::RefPtr<PluginManager>();
Glib::RefPtr<DbusManager> Engine::dbm			= Glib::RefPtr<DbusManager>();

sigc::signal<void> Engine::m_signal_tick	= sigc::signal<void>();

Engine::Engine() : RefCounter<Engine>::RefCounter(this)
{
	ssm = SettingsManager::create();
	sm = SessionManager::create();
	am = AlertManager::create();
	tm = TorrentManager::create();
	pm = PluginManager::create();

	/* FIXME: update interval on_settings() */
	int interval = ssm->get_int("UI", "Interval")*1000;
	Glib::signal_timeout().connect(sigc::mem_fun(this, &Engine::on_timeout), interval);
}

Engine::~Engine()
{
}

void Engine::init()
{
	static bool creating = false;
	if (!self && is_primary() && !creating)
	{
		creating = true;
		self = Glib::RefPtr<Engine>(new Engine());
		creating = false;
	}
}

bool Engine::is_primary()
{
	if (!dbm)
		dbm = DbusManager::create();
	
	return dbm->is_primary();
}

bool Engine::on_timeout()
{
	m_signal_tick.emit();
	return true;
}

sigc::signal<void> Engine::signal_tick()
{
	if (!self)
		init();

	return m_signal_tick;
}

Glib::RefPtr<AlertManager> Engine::get_alert_manager()
{
	if (!am)
		init();

	return am;
}

Glib::RefPtr<PluginManager> Engine::get_plugin_manager()
{
	if (!pm)
		init();

	return pm;
}

Glib::RefPtr<SessionManager> Engine::get_session_manager()
{
	if (!sm)
		init();

	return sm;
}

Glib::RefPtr<SettingsManager> Engine::get_settings_manager()
{
	if (!ssm)
		init();

	return ssm;
}

Glib::RefPtr<TorrentManager> Engine::get_torrent_manager()
{
	if (!tm)
		init();

	return tm;
}

Glib::RefPtr<DbusManager> Engine::get_dbus_manager()
{
	if (!dbm)
		init();

	return dbm;
}
