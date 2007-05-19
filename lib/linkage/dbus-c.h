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

#include <glib.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-protocol.h>
#include <dbus/dbus.h>

enum DbusAction { ACTION_QUIT, ACTION_OPEN, ACTION_TOGGLE_VISIBLE };

DBusGConnection* init(gboolean* p, void (*cb_func)(unsigned int, const char*));
DBusHandlerResult message_handler(DBusGConnection *connection, DBusGMessage *message, void *user_data);
void c_send(DBusGConnection *connection, const char* interface, const char* msg);
