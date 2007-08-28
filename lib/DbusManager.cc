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

#include <dbus/dbus-glib-bindings.h>
#include <dbus/dbus-glib-lowlevel.h>

#include <glibmm/i18n.h>

#include "linkage/DbusManager.hh"
#include "linkage/Engine.hh"
#include "linkage/Interface.hh"
#include "linkage/Utils.hh"

#define LINKAGE_BUS_NAME "org.linkage"
#define LINKAGE_INTERFACE_INTERFACE "org.linkage.Interface"
#define LINKAGE_INTERFACE_TORRENT "org.linkage.Torrent"
#define LINKAGE_PATH_INTERFACE "/org/linkage/Interface"

const char* introspect_interface = 
"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD DBUS Object Introspection 1.0//EN\"\n"
"\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
"<node>\n"
"  <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
"    <method name=\"Introspect\">\n"
"      <arg name=\"data\" direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"  </interface>\n"
"  <interface name=\"org.linkage.Interface\">\n"
"    <method name=\"Open\">\n"
"      <arg name=\"file\" direction=\"in\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"GetVisible\">\n"
"      <arg name=\"visible\" direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"SetVisible\">\n"
"      <arg name=\"visible\" direction=\"in\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"Quit\"/>\n"
"  </interface>\n"
"</node>\n";

const char* introspect_torrent = 
"<!DOCTYPE node PUBLIC \"-//freedesktop//DTD DBUS Object Introspection 1.0//EN\"\n"
"\"http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd\">\n"
"<node>\n"
"  <interface name=\"org.freedesktop.DBus.Introspectable\">\n"
"    <method name=\"Introspect\">\n"
"      <arg name=\"data\" direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"  </interface>\n"
"  <interface name=\"org.linkage.Torrent\">\n"
"    <method name=\"GetName\">\n"
"      <arg name=\"name\" direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"GetState\">\n"
"      <arg name=\"state\" direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"  </interface>\n"
"</node>\n";

bool DbusManager::handler_common(DBusConnection* connection,
	DBusMessage* message,
	const char* introspect,
	DbusManager* self)
{
	if (dbus_message_is_signal(message, DBUS_INTERFACE_DBUS, "Disconnected")) 
	{
		if (self)
			self->m_signal_disconnect.emit();
	}
	else if (dbus_message_is_method_call(message, DBUS_INTERFACE_INTROSPECTABLE, "Introspect"))
	{
		DBusMessage* reply = dbus_message_new_method_return(message);
		dbus_message_append_args(reply, DBUS_TYPE_STRING, &introspect, DBUS_TYPE_INVALID);
		dbus_connection_send(connection, reply, NULL);
		dbus_message_unref(reply);
	}
	else
		return false;

	return true;
}

DBusHandlerResult DbusManager::handler_interface(DBusConnection* connection,
	DBusMessage* message,
	DbusManager* self)
{
	// Using some dbus c++ bindings would make this much easier,
	// seems pointless to create wrapping GObjects just to use dbus-glib..

	if (handler_common(connection, message, introspect_interface, self))
		return DBUS_HANDLER_RESULT_HANDLED;

	if (dbus_message_is_method_call(message, LINKAGE_INTERFACE_INTERFACE, "Open")) 
	{
		DBusError error;
		dbus_error_init(&error);
		char *file;
		if (dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &file, DBUS_TYPE_INVALID))
		{
			Engine::get_interface()->open(file);
			DBusMessage* reply = dbus_message_new_method_return(message);
			dbus_message_append_args(reply, DBUS_TYPE_INVALID);
			dbus_connection_send(connection, reply, NULL);
			dbus_message_unref(reply);
		} 
		else 
		{
			g_warning("Error recieved from DBus: %s", error.message);
			dbus_error_free(&error);
		}
	}
	else if (dbus_message_is_method_call(message, LINKAGE_INTERFACE_INTERFACE, "GetVisible")) 
	{
		gboolean visible = Engine::get_interface()->get_visible();
		DBusMessage* reply = dbus_message_new_method_return(message);
		dbus_message_append_args(reply, DBUS_TYPE_BOOLEAN, &visible, DBUS_TYPE_INVALID);
		dbus_connection_send(connection, reply, NULL);
		dbus_message_unref(reply);
	}
	else if (dbus_message_is_method_call(message, LINKAGE_INTERFACE_INTERFACE, "SetVisible")) 
	{
		DBusError error;
		dbus_error_init(&error);
		gboolean visible;
		if (dbus_message_get_args(message, &error, DBUS_TYPE_BOOLEAN, &visible, DBUS_TYPE_INVALID))
		{
			Engine::get_interface()->set_visible(visible);
			DBusMessage* reply = dbus_message_new_method_return(message);
			dbus_message_append_args(reply, DBUS_TYPE_INVALID);
			dbus_connection_send(connection, reply, NULL);
			dbus_message_unref(reply);
		} 
		else 
		{
			g_warning("Error recieved from DBus: %s", error.message);
			dbus_error_free(&error);
		}
	}
	else if (dbus_message_is_method_call(message, LINKAGE_INTERFACE_INTERFACE, "Quit")) 
	{
		Engine::get_interface()->quit();
		DBusMessage* reply = dbus_message_new_method_return(message);
		dbus_message_append_args(reply, DBUS_TYPE_INVALID);
		dbus_connection_send(connection, reply, NULL);
		dbus_message_unref(reply);
	}
	else
	{
		DBusMessage* reply = dbus_message_new_error(message, DBUS_ERROR_FAILED, NULL);
		dbus_connection_send(connection, reply, NULL);
		dbus_message_unref(reply);
	}

	return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult DbusManager::handler_torrent(DBusConnection* connection,
	DBusMessage* message,
	Torrent* torrent)
{
	// FIXME: don't pass NULL as self
	if (handler_common(connection, message, introspect_torrent, NULL))
		return DBUS_HANDLER_RESULT_HANDLED;
	
	if (dbus_message_is_method_call(message, LINKAGE_INTERFACE_TORRENT, "GetName")) 
	{
		const char* name = torrent->get_name().c_str();
		DBusMessage* reply = dbus_message_new_method_return(message);
		dbus_message_append_args(reply, DBUS_TYPE_STRING, &name, DBUS_TYPE_INVALID);
		dbus_connection_send(connection, reply, NULL);
		dbus_message_unref(reply);
	}
	if (dbus_message_is_method_call(message, LINKAGE_INTERFACE_TORRENT, "GetState")) 
	{
		char* state = g_strdup(torrent->get_state_string().c_str());
		DBusMessage* reply = dbus_message_new_method_return(message);
		dbus_message_append_args(reply, DBUS_TYPE_STRING, &state, DBUS_TYPE_INVALID);
		dbus_connection_send(connection, reply, NULL);
		dbus_message_unref(reply);
		g_free(state);
	}
	else
	{
		DBusMessage* reply = dbus_message_new_error(message, DBUS_ERROR_FAILED, NULL);
		dbus_connection_send(connection, reply, NULL);
		dbus_message_unref(reply);
	}

	return DBUS_HANDLER_RESULT_HANDLED;
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
		g_warning("Failed to connect to Dbus session: %s", error.message);
		dbus_error_free(&error);
	}
	dbus_connection_setup_with_g_main(m_connection, NULL);

	primary = !dbus_bus_name_has_owner(m_connection, LINKAGE_BUS_NAME, &error);
	if (dbus_error_is_set(&error))
	{
		g_warning("Error querying DBus: (%s)", error.message);
		dbus_error_free(&error);
	}

	if (primary)
	{
		dbus_bus_request_name(m_connection, LINKAGE_BUS_NAME, DBUS_NAME_FLAG_DO_NOT_QUEUE, &error);
		if (dbus_error_is_set(&error))
		{
			g_warning("Error requesting Dbus name: (%s)", error.message);
			dbus_error_free(&error);
		}
		
		DBusObjectPathVTable vtable = {
			NULL,
			(DBusObjectPathMessageFunction)DbusManager::handler_interface,
			NULL,
			NULL,
			NULL,
			NULL 
		};
		dbus_connection_register_object_path(m_connection, LINKAGE_PATH_INTERFACE, &vtable, this);
	}
}


DbusManager::~DbusManager()
{
	dbus_connection_unref(m_connection);
}

void DbusManager::unregister_torrent(Torrent* torrent)
{
	Glib::ustring path = "/org/linkage/torrents/" + str(torrent->get_hash());
	dbus_connection_unregister_object_path(m_connection, path.c_str());
}

void DbusManager::register_torrent(Torrent* torrent)
{
	DBusObjectPathVTable vtable = {
		NULL,
		(DBusObjectPathMessageFunction)DbusManager::handler_torrent,
		NULL,
		NULL,
		NULL,
		NULL 
	};
	Glib::ustring path = "/org/linkage/torrents/" + str(torrent->get_hash());
	dbus_connection_register_object_path(m_connection, path.c_str(), &vtable, torrent);
}

void DbusManager::send(const Glib::ustring& interface,
	const Glib::ustring& member,
	const Glib::ustring& path,
	const Glib::ustring& msg)
{
	// ignore messages past to self, should use Engine::get_interface()/Plugin::get_data() for those
	if (!is_primary())
	{
		DBusMessage *message;
		DBusError error;

		message = dbus_message_new_method_call(LINKAGE_BUS_NAME, path.c_str(), interface.c_str(), member.c_str());
		if (!msg.empty())
		{
			const char* c_msg = msg.c_str();
			dbus_message_append_args(message, DBUS_TYPE_STRING, &c_msg, DBUS_TYPE_INVALID);
		}
		else
			dbus_message_append_args(message, DBUS_TYPE_INVALID);
		dbus_connection_send(m_connection, message, NULL);
		dbus_message_unref(message);
	}
	else
		g_warning(_("Ignoring attempt to send message to self (%s: %s)"), member.c_str(), msg.c_str());
}

bool DbusManager::is_primary()
{
	return primary;
}

sigc::signal<void> DbusManager::signal_disconnect()
{
	return m_signal_disconnect;
}

