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
#include "linkage/Interface.hh"

using namespace Linkage;

Engine* Engine::self = NULL;

SettingsManagerPtr Engine::ssm = SettingsManagerPtr();
TorrentManagerPtr Engine::tm = TorrentManagerPtr();
SessionManagerPtr Engine::sm = SessionManagerPtr();
AlertManagerPtr Engine::am = AlertManagerPtr();
PluginManagerPtr Engine::pm = PluginManagerPtr();

Interface* Engine::m_interface = NULL;

Engine::Engine(const DBus::Connection& connection)
: m_conn(connection)
{
	/* check if name is taken before we request it */
	m_primary = !m_conn.has_name("org.linkage");

	m_conn.request_name("org.linkage");

	/* FIXME: update interval on_settings() */
	int interval = get_settings_manager()->get_int("ui/interval")*1000;
	Glib::signal_timeout().connect(sigc::mem_fun(this, &Engine::on_timeout), interval);
}

Engine::~Engine()
{
}

void Engine::init(const DBus::Connection& connection)
{
	static bool creating = false;
	if (!self && !creating)
	{
		creating = true;
		self = new Engine(connection);
		creating = false;
	}
}

void Engine::uninit()
{
	g_assert(self);

	ssm->disconnect();

	// Kill them of in order due to depencies
	pm = PluginManagerPtr();
	tm = TorrentManagerPtr();
	sm = SessionManagerPtr();
	am = AlertManagerPtr();
	ssm = SettingsManagerPtr();

	delete self;
}

bool Engine::is_primary()
{
	g_assert(self);

	return self->m_primary;
}

DBus::Connection& Engine::get_bus()
{
	g_assert(self);

	return self->m_conn;
}

bool Engine::on_timeout()
{
	m_signal_tick.emit();
	return true;
}

sigc::signal<void> Engine::signal_tick()
{
	g_assert(self);

	return self->m_signal_tick;
}

AlertManagerPtr Engine::get_alert_manager()
{
	if (!am)
		am = AlertManager::create();

	return am;
}

PluginManagerPtr Engine::get_plugin_manager()
{
	if (!pm)
		pm = PluginManager::create();

	return pm;
}

SessionManagerPtr Engine::get_session_manager()
{
	if (!sm)
		sm = SessionManager::create();

	return sm;
}

SettingsManagerPtr Engine::get_settings_manager()
{
	if (!ssm)
		ssm = SettingsManager::create();

	return ssm;
}

TorrentManagerPtr Engine::get_torrent_manager()
{
	if (!tm)
		tm = TorrentManager::create();

	return tm;
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

