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
#include "linkage/AlertManager.hh"
#include "linkage/PluginManager.hh"
#include "linkage/SessionManager.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/TorrentManager.hh"
#include "linkage/DbusManager.hh"
#include "linkage/Interface.hh"

using namespace Linkage;

Engine* Engine::self = NULL;

Glib::RefPtr<SettingsManager> Engine::ssm	= Glib::RefPtr<SettingsManager>();
Glib::RefPtr<TorrentManager> Engine::tm		= Glib::RefPtr<TorrentManager>();
Glib::RefPtr<SessionManager> Engine::sm		= Glib::RefPtr<SessionManager>();
Glib::RefPtr<AlertManager> Engine::am			= Glib::RefPtr<AlertManager>();
Glib::RefPtr<PluginManager> Engine::pm		= Glib::RefPtr<PluginManager>();
Glib::RefPtr<DbusManager> Engine::dbm			= Glib::RefPtr<DbusManager>();

Interface* Engine::m_interface = NULL;

Engine::Engine()
{
	/* FIXME: update interval on_settings() */
	int interval = get_settings_manager()->get_int("ui/interval")*1000;
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
		self = new Engine();
		creating = false;
	}
}

void Engine::uninit()
{
	// Kill them of in order due to depencies
	pm.clear();
	tm.clear();
	dbm.clear();
	sm.clear();
	am.clear();
	ssm.clear();

	delete self;
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

	return self->m_signal_tick;
}

Glib::RefPtr<AlertManager> Engine::get_alert_manager()
{
	if (!am)
		am = AlertManager::create();

	return am;
}

Glib::RefPtr<PluginManager> Engine::get_plugin_manager()
{
	if (!pm)
		pm = PluginManager::create();

	return pm;
}

Glib::RefPtr<SessionManager> Engine::get_session_manager()
{
	if (!sm)
		sm = SessionManager::create();

	return sm;
}

Glib::RefPtr<SettingsManager> Engine::get_settings_manager()
{
	if (!ssm)
		ssm = SettingsManager::create();

	return ssm;
}

Glib::RefPtr<TorrentManager> Engine::get_torrent_manager()
{
	if (!tm)
		tm = TorrentManager::create();

	return tm;
}

Glib::RefPtr<DbusManager> Engine::get_dbus_manager()
{
	if (!dbm)
		dbm = DbusManager::create();

	return dbm;
}

Interface& Engine::get_interface() throw()
{
	if (m_interface)
		return *m_interface;

	throw InterfaceException();
}

void Engine::register_interface(Interface* interface)
{
	if (!m_interface)
		m_interface = interface;
}

