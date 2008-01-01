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

#include <exception>

#include <sigc++/signal.h>

#include <glibmm/exception.h>
#include <glibmm/object.h>
#include <glibmm/refptr.h>

#include <dbus-c++/dbus.h>

#include "linkage/AlertManager.hh"
#include "linkage/PluginManager.hh"
#include "linkage/SessionManager.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/TorrentManager.hh"

namespace Linkage
{

class Interface;

class InterfaceException : public Glib::Exception
{
public:
  virtual Glib::ustring what() const throw()
  {
    return "No interface previously set";
  }
  virtual ~InterfaceException() throw() {}
};

class Engine
{

	static SettingsManagerPtr ssm;
	static TorrentManagerPtr tm;
	static SessionManagerPtr sm;
	static AlertManagerPtr am;
	static PluginManagerPtr pm;
	static Interface* m_interface;

	static Engine* self;

	DBus::Connection m_conn;
	bool m_primary;

	sigc::signal<void> m_signal_tick;

	bool on_timeout();

	Engine(const DBus::Connection& connection);
	
public:
	static AlertManagerPtr get_alert_manager();
	static PluginManagerPtr get_plugin_manager();
	static SessionManagerPtr get_session_manager();
	static SettingsManagerPtr get_settings_manager();
	static TorrentManagerPtr get_torrent_manager();

	static Interface& get_interface() throw();
	static void register_interface(Interface* interface);

	static sigc::signal<void> signal_tick();

	static void init(const DBus::Connection& connection);
	static void uninit();

	static bool is_primary();
	static DBus::Connection& get_bus();

	~Engine();
};

}; /* namespace */

#endif /* ENGINE_HH */

