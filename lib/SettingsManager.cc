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
#include <iostream>
#include <fstream>
#include <glibmm/iochannel.h>
#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>

#include <libtorrent/bencode.hpp>

#include "linkage/SettingsManager.hh"
#include "linkage/Utils.hh"

Glib::RefPtr<SettingsManager> SettingsManager::create()
{
	return Glib::RefPtr<SettingsManager>(new SettingsManager());
}

SettingsManager::SettingsManager() : RefCounter<SettingsManager>::RefCounter(this)
{
	Glib::ustring groups_file = Glib::build_filename(get_config_dir(), "groups");

	/* Create data dir if it doesn't exists */
	if(g_mkdir_with_parents(get_data_dir().c_str(), 0755) == -1)
		g_warning(("Could not create directory: " + get_data_dir()).c_str());
	
	Gnome::Conf::init();
	gconf = Gnome::Conf::Client::get_default_client();
	
	/* Dump default groups if file doens't exists */
	if (!Glib::file_test(groups_file, Glib::FILE_TEST_EXISTS))
	{
		using namespace libtorrent;
		// create the default bencoded groups file
		entry e(entry::dictionary_t);
		entry::list_type filters;
		entry::dictionary_type filter;

		filter["filter"] = entry("");
		filter["eval"] = entry(int(1));
		filter["tag"] = entry(int(1));
		filters.push_back(entry(filter));
		e["Downloads"] = entry(filters);

		filters.clear();
		filter["filter"] = entry("Seeding");
		filter["eval"] = entry(int(1));
		filter["tag"] = entry(3);
		filters.push_back(entry(filter));
		e["Seeds"] = entry(filters);

		write_groups_data(e);
	}
	
	/// TODO check gconf for the schema to make sure we
	/// have some defaults
}

SettingsManager::~SettingsManager() {}

sigc::signal<void> SettingsManager::signal_update_settings()
{
	return m_signal_update_settings;
}

Glib::ustring SettingsManager::get_string(const Glib::ustring& path) const
{
	Glib::ustring prefix = (path[0] == '/' ? "" : "/apps/linkage/");
	return gconf->get_string(prefix + path);
}

int SettingsManager::get_int(const Glib::ustring& path) const
{
	Glib::ustring prefix = (path[0] == '/' ? "" : "/apps/linkage/");
	return gconf->get_int(prefix + path);
}

double SettingsManager::get_float(const Glib::ustring& path) const
{
	Glib::ustring prefix = (path[0] == '/' ? "" : "/apps/linkage/");
	return gconf->get_float(prefix + path);
}

bool SettingsManager::get_bool(const Glib::ustring& path) const
{
	Glib::ustring prefix = (path[0] == '/' ? "" : "/apps/linkage/");
	return gconf->get_bool(prefix + path);
}

UStringArray SettingsManager::get_string_list(const Glib::ustring& path) const
{
	Glib::ustring prefix = (path[0] == '/' ? "" : "/apps/linkage/");
	return gconf->get_string_list(prefix + path);
}

IntArray SettingsManager::get_int_list(const Glib::ustring& path) const
{
	Glib::ustring prefix = (path[0] == '/' ? "" : "/apps/linkage/");
	return gconf->get_int_list(prefix + path);
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

void SettingsManager::set(const Glib::ustring& path, double value)
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

void SettingsManager::update()
{
	m_signal_update_settings.emit();
}

void SettingsManager::write_groups_data(const libtorrent::entry& ent)
{
	Glib::ustring file = Glib::build_filename(get_config_dir(), "groups");
	try
	{
		std::ofstream out(file.c_str(), std::ios_base::binary);
		libtorrent::bencode(std::ostream_iterator<char>(out), ent);
		out.close();
	}
	catch(std::exception& e)
	{
		g_warning("Could not save groups file: %s", e.what());
	}
}

void SettingsManager::get_groups_data(libtorrent::entry& ent)
{
	try
	{
		std::ifstream in(Glib::build_filename(get_config_dir(), "groups").c_str(), 
			std::ios_base::binary);
		in.unsetf(std::ios_base::skipws);
		ent = bdecode(std::istream_iterator<char>(in),
			std::istream_iterator<char>());
	}
	catch (std::exception& e)
	{
		g_warning("Could not save groups file: %s", e.what());
	}
}
