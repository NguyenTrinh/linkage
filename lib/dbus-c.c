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

DBusGConnection* init(int* p, void (*cb_func)(unsigned int, const char*))
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
		int ret = dbus_bus_request_name(connection, "org.linkage", DBUS_NAME_FLAG_DO_NOT_QUEUE, &error);
		if (dbus_error_is_set(&error))
		{
			g_warning("Error requesting D-BUS name: (%s)", error.message);
			dbus_error_free(&error);
		}
		dbus_bus_add_match(connection, "type='signal',interface='org.linkage'", &error);
		dbus_connection_add_filter(connection, signal_filter, cb_func, NULL);
	}

	return connection;
}

DBusHandlerResult signal_filter(DBusGConnection* connection, DBusGMessage* message, void* user_data)
{
	void (*cb_func)() = user_data;
	
	if (dbus_message_is_signal(message, DBUS_INTERFACE_DBUS, "Disconnected")) 
	{
		(*cb_func)(ACTION_QUIT, NULL);
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_signal(message, "org.linkage", "Open")) 
	{
		GError error;
		char *file;
		dbus_error_init (&error);
		if (dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &file, DBUS_TYPE_INVALID))
		{
			(*cb_func)(ACTION_OPEN, file);
		} 
		else 
		{
			g_warning("Error recieved from D-BUS: %s", error.message);
			dbus_error_free (&error);
		}
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_signal(message, "org.linkage", "Quit")) 
	{
		GError error;
		dbus_error_init (&error);
		if (dbus_message_get_args(message, &error, DBUS_TYPE_INVALID))
		{
			(*cb_func)(ACTION_QUIT, NULL);
		} 
		else 
		{
			g_warning("Error recieved from D-BUS: %s", error.message);
			dbus_error_free (&error);
		}
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if (dbus_message_is_signal(message, "org.linkage", "ToggleVisible")) 
	{
		GError error;
		dbus_error_init (&error);
		if (dbus_message_get_args(message, &error, DBUS_TYPE_INVALID))
		{
			(*cb_func)(ACTION_TOGGLE_VISIBLE, NULL);
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

void c_send(DBusGConnection* connection,const char* interface, const char* msg)
{
	DBusGMessage *message;
	GError error;
	
	message = (DBusGMessage*)dbus_message_new_signal("/org/linkage/Open", "org.linkage", interface);
	if (msg != NULL)
		dbus_message_append_args(message, DBUS_TYPE_STRING, &msg, DBUS_TYPE_INVALID);
	else
		dbus_message_append_args(message, DBUS_TYPE_INVALID);
	dbus_connection_send(connection, message, NULL);
	dbus_message_unref(message);
}
