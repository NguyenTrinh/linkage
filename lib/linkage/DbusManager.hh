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

#ifndef DBUS_MANAGER_HH
#define DBUS_MANAGER_HH

#include <sigc++/signal.h>
#include <glibmm/ustring.h>
#include <glibmm/refptr.h>

#include "linkage/RefCounter.hh"

extern "C" {
#include <dbus/dbus-glib.h>
}

class DbusManager : public RefCounter<DbusManager>
{	
	bool primary;
	DBusGConnection *m_connection;
	
	sigc::signal<void> m_signal_quit;
	sigc::signal<void, const Glib::ustring&> m_signal_open;
	sigc::signal<void> m_signal_toggle_visible;
	
	DbusManager();
	
public:
	sigc::signal<void> signal_quit();
	sigc::signal<void, const Glib::ustring&> signal_open();
	sigc::signal<void> signal_toggle_visible();
	
	bool is_primary();
	void send(const Glib::ustring& interface, const Glib::ustring& msg = Glib::ustring());
	
	static void callback_handler(unsigned int action, const char* data);
	
	static Glib::RefPtr<DbusManager> create();
	~DbusManager();
};

#endif /* DBUS_MANAGER_HH */
