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

#include <dbus/dbus-protocol.h>
#include <dbus/dbus.h>

class DbusManager : public RefCounter<DbusManager>
{	
	bool primary;
	DBusConnection* m_connection;
	
	sigc::signal<void> m_signal_disconnect;
	
	DbusManager();
	
public:
	sigc::signal<void> signal_disconnect();
	
	bool is_primary();
	void send(const Glib::ustring& member, const Glib::ustring& object, const Glib::ustring& msg = Glib::ustring());

	static DBusHandlerResult message_handler(DBusConnection* connection, DBusMessage* message, gpointer data);
	
	static Glib::RefPtr<DbusManager> create();
	~DbusManager();
};

#endif /* DBUS_MANAGER_HH */
