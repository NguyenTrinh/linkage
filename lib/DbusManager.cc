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

#include "linkage/DbusManager.hh"

/* FIXME: This is sooo ugly... */
#include "linkage/Engine.hh"
#include <iostream>
extern "C" {
#include "linkage/dbus-c.h"
}

void DbusManager::callback_handler(unsigned int action, const char* data)
{
	switch (action)
	{
		case ACTION_QUIT:
			Engine::get_dbus_manager()->signal_quit().emit();
			break;
		case ACTION_OPEN:
			Engine::get_dbus_manager()->signal_open().emit(data);
			break;
		case ACTION_TOGGLE_VISIBLE:
			Engine::get_dbus_manager()->signal_toggle_visible().emit();
			break;
	}
}

Glib::RefPtr<DbusManager> DbusManager::create()
{
	return Glib::RefPtr<DbusManager>(new DbusManager());
}

DbusManager::DbusManager() : RefCounter<DbusManager>::RefCounter(this)
{
	gboolean p = FALSE;
	m_connection = init(&p, &DbusManager::callback_handler);
	primary = p;
	std::cout << m_connection << std::endl;
}


DbusManager::~DbusManager()
{
}

void DbusManager::send(const Glib::ustring& interface, const Glib::ustring& msg)
{
	c_send(m_connection, interface.c_str(), msg.c_str());
}

void DbusManager::send(const Glib::ustring& interface)
{
	c_send(m_connection, interface.c_str(), NULL);
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
