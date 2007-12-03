/*
Copyright (C) 2006-2007   Christian Lundgren
Copyright (C) 2007        Dave Moore

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

#include <fstream>

#include <glibmm/iochannel.h>
#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>
#include <glibmm/i18n.h>

#include "linkage/SettingsManager.hh"
#include "linkage/Utils.hh"

using namespace Linkage;

Glib::RefPtr<SettingsManager> SettingsManager::create()
{
	return Glib::RefPtr<SettingsManager>(new SettingsManager());
}

SettingsManager::SettingsManager()
{
	/* Create data dir if it doesn't exists */
	// FIXME: this belongs in SessionManager/TorrentManager
	if (g_mkdir_with_parents(get_data_dir().c_str(), 0755) == -1)
		g_warning(_("Could not create directory: %s"), get_data_dir().c_str());
	
	Gnome::Conf::init();
	gconf = Gnome::Conf::Client::get_default_client();

	notify_id = gconf->notify_add("/apps/linkage", sigc::mem_fun(this, &SettingsManager::on_changed));
	gconf->add_dir("/apps/linkage", Gnome::Conf::CLIENT_PRELOAD_NONE);
	/// TODO check gconf for the schema to make sure we have some defaults
}

SettingsManager::~SettingsManager()
{
	gconf->suggest_sync();
}

void SettingsManager::disconnect()
{
	gconf->remove_dir("/apps/linkage");
	gconf->notify_remove(notify_id);
}

void SettingsManager::on_changed(guint id, Gnome::Conf::Entry entry)
{
	Glib::ustring key = entry.get_key().substr(14); // remove /apps/linkage/
	Value value(entry.get_value());
	m_signal_key_changed.emit(key, value);
	m_signals[key].emit(value);
}

sigc::signal<void, const Value&>
SettingsManager::signal(const Glib::ustring& key)
{
	if (Glib::str_has_prefix(key, "/apps/linkage/"))
		return m_signals[key.substr(14)];

	return m_signals[key];
}

sigc::signal<void, const Glib::ustring&, const Value&>
SettingsManager::signal_key_changed()
{
	return m_signal_key_changed;
}

Glib::ustring SettingsManager::get_string(const Glib::ustring& path) const
{
	Glib::ustring prefix = (path[0] == '/' ? "" : "/apps/linkage/");

	try
	{
		return gconf->get_string(prefix + path);
	}
	catch (Gnome::Conf::Error err)
	{
		g_warning(err.what().c_str());
	}
}

int SettingsManager::get_int(const Glib::ustring& path) const
{
	Glib::ustring prefix = (path[0] == '/' ? "" : "/apps/linkage/");

	try
	{
		return gconf->get_int(prefix + path);
	}
	catch (Gnome::Conf::Error err)
	{
		g_warning(err.what().c_str());
	}
}

float SettingsManager::get_float(const Glib::ustring& path) const
{
	Glib::ustring prefix = (path[0] == '/' ? "" : "/apps/linkage/");

	try
	{
		return gconf->get_float(prefix + path);
	}
	catch (Gnome::Conf::Error err)
	{
		g_warning(err.what().c_str());
	}
}

bool SettingsManager::get_bool(const Glib::ustring& path) const
{
	Glib::ustring prefix = (path[0] == '/' ? "" : "/apps/linkage/");

	try
	{
		return gconf->get_bool(prefix + path);
	}
	catch (Gnome::Conf::Error err)
	{
		g_warning(err.what().c_str());
	}
}

UStringArray SettingsManager::get_string_list(const Glib::ustring& path) const
{
	Glib::ustring prefix = (path[0] == '/' ? "" : "/apps/linkage/");

	try
	{
		return gconf->get_string_list(prefix + path);
	}
	catch (Gnome::Conf::Error err)
	{
		g_warning(err.what().c_str());
	}
}

gint SettingsManager::get_enum(const Glib::ustring& path, GConfEnumStringPair* table) const
{
	Glib::ustring prefix = (path[0] == '/' ? "" : "/apps/linkage/");

	try
	{
		gint tmp = 0;
		Glib::ustring enum_str = gconf->get_string(prefix + path);
		gconf_string_to_enum(table, enum_str.c_str(), &tmp);
		return tmp;
	}
	catch (Gnome::Conf::Error err)
	{
		g_warning(err.what().c_str());
	}
}

IntArray SettingsManager::get_int_list(const Glib::ustring& path) const
{
	Glib::ustring prefix = (path[0] == '/' ? "" : "/apps/linkage/");

	try
	{
		return gconf->get_int_list(prefix + path);
	}
	catch (Gnome::Conf::Error err)
	{
		g_warning(err.what().c_str());
	}
}

void SettingsManager::set(const Glib::ustring& path, const Glib::ustring& value)
{
	Glib::ustring prefix = (path[0] == '/' ? "" : "/apps/linkage/");
	gconf->set(prefix + path, value);
}

void SettingsManager::set(const Glib::ustring& path, int value)
{
	Glib::ustring prefix = (path[0] == '/' ? "" : "/apps/linkage/");
	gconf->set(prefix + path, value);
}

void SettingsManager::set(const Glib::ustring& path, float value)
{
	Glib::ustring prefix = (path[0] == '/' ? "" : "/apps/linkage/");
	gconf->set(prefix + path, value);
}

void SettingsManager::set(const Glib::ustring& path, bool value)
{
	Glib::ustring prefix = (path[0] == '/' ? "" : "/apps/linkage/");
	gconf->set(prefix + path, value);
}

void SettingsManager::set(const Glib::ustring& path, const Glib::SListHandle<Glib::ustring> &values)
{
	Glib::ustring prefix = (path[0] == '/' ? "" : "/apps/linkage/");
	gconf->set_string_list(prefix + path, values);
}

void SettingsManager::set(const Glib::ustring& path, const Gnome::Conf::Client::SListHandleInts &values)
{
	Glib::ustring prefix = (path[0] == '/' ? "" : "/apps/linkage/");
	gconf->set_int_list(prefix + path, values);
}

void SettingsManager::set(const Glib::ustring& path, GConfEnumStringPair* table, gint value)
{
	Glib::ustring prefix = (path[0] == '/' ? "" : "/apps/linkage/");
	Glib::ustring enum_str = Glib::convert_const_gchar_ptr_to_ustring(
		gconf_enum_to_string(table, value));
	gconf->set(prefix + path, enum_str);
}

