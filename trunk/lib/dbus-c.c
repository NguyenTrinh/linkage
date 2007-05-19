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

#include "linkage/dbus-c.h"

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
"      <arg direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"Quit\">\n"
"    </method>\n"
"    <method name=\"ToggleVisible\">\n"
"    </method>\n"
"  </interface>\n"
"</node>\n";

DBusGConnection* init(gboolean* p, void (*cb_func)(unsigned int, const char*))
{
	GError error;
	DBusGConnection* connection;

	dbus_error_init(&error);
	connection = (DBusGConnection*)dbus_bus_get(DBUS_BUS_SESSION, &error);
	if (!connection) 
	{
		g_warning("Failed to connect to the D-BUS daemon: %s", error.message);
		dbus_error_free(&error);
		return 0;
	}
	dbus_connection_setup_with_g_main(connection, NULL);

	*p = !dbus_bus_name_has_owner(connection, "org.linkage", &error);
	if (dbus_error_is_set(&error))
	{
		g_warning("Error querying D-BUS: (%s)", error.message);
		dbus_error_free(&error);
	}

	if (*p)
	{
		dbus_bus_request_name(connection, "org.linkage", DBUS_NAME_FLAG_DO_NOT_QUEUE, &error);
		if (dbus_error_is_set(&error))
		{
			g_warning("Error requesting D-BUS name: (%s)", error.message);
			dbus_error_free(&error);
		}
		
		DBusObjectPathVTable vtable  = {
			NULL,
			message_handler,
			NULL,
			NULL,
			NULL,
			NULL 
		};
		dbus_connection_register_object_path(connection, "/org/linkage", &vtable, cb_func);
	}

	return connection;
}

DBusHandlerResult message_handler(DBusGConnection* connection, DBusGMessage* message, void* user_data)
{
	void (*cb_func)() = user_data;
	
	if (dbus_message_is_signal(message, DBUS_INTERFACE_DBUS, "Disconnected")) 
	{
		(*cb_func)(ACTION_QUIT, NULL);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_method_call(message, DBUS_INTERFACE_INTROSPECTABLE, "Introspect"))
	{
		DBusGMessage* reply = (DBusGMessage*)dbus_message_new_method_return(message);
		dbus_message_append_args(reply, DBUS_TYPE_STRING, &xml_introspect, DBUS_TYPE_INVALID);
		dbus_connection_send(connection, reply, NULL);
		dbus_message_unref(reply);
	}
	else if (dbus_message_is_method_call(message, "org.linkage", "Open")) 
	{
		GError error;
		char *file;
		dbus_error_init (&error);
		if (dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &file, DBUS_TYPE_INVALID))
		{
			(*cb_func)(ACTION_OPEN, file);
			DBusGMessage* reply = (DBusGMessage*)dbus_message_new_method_return(message);
			dbus_message_append_args(reply, DBUS_TYPE_INVALID);
			dbus_connection_send(connection, reply, NULL);
			dbus_message_unref(reply);
		} 
		else 
		{
			g_warning("Error recieved from D-BUS: %s", error.message);
			dbus_error_free (&error);
		}
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_method_call(message, "org.linkage", "Quit")) 
	{
		GError error;
		dbus_error_init (&error);
		if (dbus_message_get_args(message, &error, DBUS_TYPE_INVALID))
		{
			(*cb_func)(ACTION_QUIT, NULL);
			DBusGMessage* reply = (DBusGMessage*)dbus_message_new_method_return(message);
			dbus_message_append_args(reply, DBUS_TYPE_INVALID);
			dbus_connection_send(connection, reply, NULL);
			dbus_message_unref(reply);
		} 
		else 
		{
			g_warning("Error recieved from D-BUS: %s", error.message);
			dbus_error_free (&error);
		}
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_method_call(message, "org.linkage", "ToggleVisible")) 
	{
		GError error;
		dbus_error_init (&error);
		if (dbus_message_get_args(message, &error, DBUS_TYPE_INVALID))
		{
			(*cb_func)(ACTION_TOGGLE_VISIBLE, NULL);
			DBusGMessage* reply = (DBusGMessage*)dbus_message_new_method_return(message);
			dbus_message_append_args(reply, DBUS_TYPE_INVALID);
			dbus_connection_send(connection, reply, NULL);
			dbus_message_unref(reply);
		} 
		else 
		{
			g_warning("Error recieved from D-BUS: %s", error.message);
			dbus_error_free (&error);
		}
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void c_send(DBusGConnection* connection, const char* method, const char* data)
{
	DBusGMessage *message;
	GError error;
	
	message = (DBusGMessage*)dbus_message_new_method_call("org.linkage", "/org/linkage", "org.linkage", method);
	if (data != NULL)
		dbus_message_append_args(message, DBUS_TYPE_STRING, &data, DBUS_TYPE_INVALID);
	else
		dbus_message_append_args(message, DBUS_TYPE_INVALID);
	dbus_connection_send(connection, message, NULL);
	dbus_message_unref(message);
}
