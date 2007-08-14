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

#include <glibmm/i18n.h>

#include "linkage/DbusManager.hh"

const char* xml_introspect = 
"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD D-BUS Object Introspection 1.0//EN\"\n"
"\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
"<node>\n"
"  <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
"    <method name=\"Introspect\">\n"
"      <arg name=\"data\" direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"  </interface>\n"
"  <interface name=\"org.linkage\">\n"
"    <method name=\"Open\">\n"
"      <arg name=\"file\" direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"Quit\">\n"
"    </method>\n"
"    <method name=\"ToggleVisible\">\n"
"    </method>\n"
"  </interface>\n"
"</node>\n";

DBusHandlerResult DbusManager::message_handler(DBusConnection* connection, DBusMessage* message, gpointer data)
{
	DbusManager* self = static_cast<DbusManager*>(data);

	if (dbus_message_is_signal(message, DBUS_INTERFACE_DBUS, "Disconnected")) 
	{
		self->m_signal_quit.emit();
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_method_call(message, DBUS_INTERFACE_INTROSPECTABLE, "Introspect"))
	{
		DBusMessage* reply = dbus_message_new_method_return(message);
		dbus_message_append_args(reply, DBUS_TYPE_STRING, &xml_introspect, DBUS_TYPE_INVALID);
		dbus_connection_send(connection, reply, NULL);
		dbus_message_unref(reply);
	}
	else if (dbus_message_is_method_call(message, "org.linkage", "Open")) 
	{
		DBusError error;
		char *file;
		dbus_error_init(&error);
		if (dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &file, DBUS_TYPE_INVALID))
		{
			self->m_signal_open.emit(file);
			DBusMessage* reply = dbus_message_new_method_return(message);
			dbus_message_append_args(reply, DBUS_TYPE_INVALID);
			dbus_connection_send(connection, reply, NULL);
			dbus_message_unref(reply);
		} 
		else 
		{
			g_warning("Error recieved from D-BUS: %s", error.message);
			dbus_error_free(&error);
		}
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_method_call(message, "org.linkage", "Quit")) 
	{
		DBusError error;
		dbus_error_init(&error);
		if (dbus_message_get_args(message, &error, DBUS_TYPE_INVALID))
		{
			self->m_signal_quit.emit();
			DBusMessage* reply = dbus_message_new_method_return(message);
			dbus_message_append_args(reply, DBUS_TYPE_INVALID);
			dbus_connection_send(connection, reply, NULL);
			dbus_message_unref(reply);
		} 
		else 
		{
			g_warning("Error recieved from D-BUS: %s", error.message);
			dbus_error_free(&error);
		}
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_method_call(message, "org.linkage", "ToggleVisible")) 
	{
		DBusError error;
		dbus_error_init(&error);
		if (dbus_message_get_args(message, &error, DBUS_TYPE_INVALID))
		{
			self->m_signal_toggle_visible.emit();
			DBusMessage* reply = dbus_message_new_method_return(message);
			dbus_message_append_args(reply, DBUS_TYPE_INVALID);
			dbus_connection_send(connection, reply, NULL);
			dbus_message_unref(reply);
		} 
		else 
		{
			g_warning("Error recieved from D-BUS: %s", error.message);
			dbus_error_free(&error);
		}
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

Glib::RefPtr<DbusManager> DbusManager::create()
{
	return Glib::RefPtr<DbusManager>(new DbusManager());
}

DbusManager::DbusManager() : RefCounter<DbusManager>::RefCounter(this)
{
	DBusError error;

	dbus_error_init(&error);
	m_connection = dbus_bus_get(DBUS_BUS_SESSION, &error);
	if (!m_connection) 
	{
		g_warning("Failed to connect to the D-BUS daemon: %s", error.message);
		dbus_error_free(&error);
	}
	dbus_connection_setup_with_g_main(m_connection, NULL);

	primary = !dbus_bus_name_has_owner(m_connection, "org.linkage", &error);
	if (dbus_error_is_set(&error))
	{
		g_warning("Error querying D-BUS: (%s)", error.message);
		dbus_error_free(&error);
	}

	if (primary)
	{
		dbus_bus_request_name(m_connection, "org.linkage", DBUS_NAME_FLAG_DO_NOT_QUEUE, &error);
		if (dbus_error_is_set(&error))
		{
			g_warning("Error requesting D-BUS name: (%s)", error.message);
			dbus_error_free(&error);
		}
		
		DBusObjectPathVTable vtable = {
			NULL,
			DbusManager::message_handler,
			NULL,
			NULL,
			NULL,
			NULL 
		};
		dbus_connection_register_object_path(m_connection, "/org/linkage", &vtable, this);
	}
}


DbusManager::~DbusManager()
{
}

void DbusManager::send(const Glib::ustring& interface, const Glib::ustring& msg)
{
	// ignore messages past to self, should use Engine::get_interface() for those
	if (!is_primary())
	{
		DBusMessage *message;
		DBusError error;
		
		message = dbus_message_new_method_call("org.linkage", "/org/linkage", "org.linkage", interface.c_str());
		if (!msg.empty())
			dbus_message_append_args(message, DBUS_TYPE_STRING, msg.c_str(), DBUS_TYPE_INVALID);
		else
			dbus_message_append_args(message, DBUS_TYPE_INVALID);
		dbus_connection_send(m_connection, message, NULL);
		dbus_message_unref(message);
	}
	else
		g_warning(_("Ignoring attempt to send message to self (%s: %s)"), interface.c_str(), msg.c_str());
}

bool DbusManager::is_primary()
{
	return primary;
}

sigc::signal<void> DbusManager::signal_quit()
{
	return m_signal_quit;
}

sigc::signal<void, const Glib::ustring&> DbusManager::signal_open()
{
	return m_signal_open;
}

sigc::signal<void> DbusManager::signal_toggle_visible()
{
	return m_signal_toggle_visible;
}
