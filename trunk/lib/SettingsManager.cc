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
#include <iostream>
#include <glibmm/iochannel.h>
#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>

#include "linkage/SettingsManager.hh"
#include "linkage/Utils.hh"

Glib::ustring SettingsManager::defaults = "[Network]\n"
																					"MinPort=6881\n"
																					"MaxPort=6889\n"
																					"MaxUpRate=0\n"
																					"MaxDownRate=0\n"
																					"MaxUploads=0\n"
																					"MaxConnections=0\n"
																					"MaxActive=3\n"
																					"TrackerTimeout=10\n"
																					"UseProxy=0\n"
																					"ProxyIP=\n"
																					"ProxyPort=8080\n"
																					"ProxyLogin=\n"
																					"ProxyPass=\n"
																					"Interface=\n"
																					"[UI]\n"
																					"Interval=1\n"
																					"WinHeight=-1\n"
																					"WinWidth=-1\n"
																					"Expanded=0\n"
																					"AutoExpand=0\n"
																					"Page=0\n"
																					"Selected=\n"
																					"SortColumn=1\n"
																					"SortOrder=0\n"
																					"Plugins=\n"
																					"ColorDark=0;0;65535;\n"
																					"ColorMid=32767;32767;65535;\n"
																					"ColorLight=52428;52428;65535;\n"
																					"[Files]\n"
																					"UseDefPath=0\n"
																					"DefPath=\n"
																					"MoveFinished=0\n"
																					"FinishedPath=\n"
																					"Allocate=0\n"
																					"DefGroup=Downloads\n"
																					"[Groups]\n"
																					"Downloads=0\n"
																					"Seeds=0\n";

Glib::RefPtr<SettingsManager> SettingsManager::create()
{
	return Glib::RefPtr<SettingsManager>(new SettingsManager());
}
																					
SettingsManager::SettingsManager() : RefCounter<SettingsManager>::RefCounter(this)
{
	Glib::ustring file = Glib::build_filename(get_config_dir(), "config");
	Glib::ustring data_dir = Glib::build_filename(get_config_dir(), "data");

	/* Create data dir if it doesn't exists, bail if we can't */
	if(g_mkdir_with_parents(data_dir.c_str(), 0755) == -1)
		std::cerr << "Could not	create directories: " << data_dir << std::endl;
			
	/* Dump default config if file doens't exists, bail if we can't	*/
	if (!Glib::file_test(file, Glib::FILE_TEST_EXISTS))
	{
		try
		{
			Glib::RefPtr<Glib::IOChannel> out = Glib::IOChannel::create_from_file(file, "w");
			out->write(defaults);
			out->close();
		}
		catch(Glib::Error& e)
		{
			std::cerr << "Could not save keyfile: " << e.what()	<< std::endl;
		}
	}
	
	/* Read keyfile, bail if we can't */
	keyfile = new Glib::KeyFile();
	if (!keyfile->load_from_file(file))
		std::cerr << "Could not read keyfile"	<< std::endl;
}

SettingsManager::~SettingsManager()
{
	Glib::ustring file = Glib::build_filename(get_config_dir(), "config");
	try
	{
		Glib::RefPtr<Glib::IOChannel> out = Glib::IOChannel::create_from_file(file, "w");
		out->write(keyfile->to_data());
		out->close();
	}
	catch(Glib::Error& e)
	{
		std::cerr << "Could not save keyfile: " << e.what() << std::endl;
	}
	delete keyfile;
}

sigc::signal<void> SettingsManager::signal_update_settings()
{
	return m_signal_update_settings;
}

Glib::ustring SettingsManager::get_string(const Glib::ustring& group, const Glib::ustring& key) const
{
	return keyfile->get_string(group, key);
}

int SettingsManager::get_int(const Glib::ustring& group, const Glib::ustring& key) const
{
	return keyfile->get_integer(group, key);
}

bool SettingsManager::get_bool(const Glib::ustring& group, const Glib::ustring& key) const
{
	return keyfile->get_boolean(group, key);
}

UStringArray SettingsManager::get_string_list(const Glib::ustring& group, const Glib::ustring& key) const
{
	return keyfile->get_string_list(group, key);
}

IntArray SettingsManager::get_int_list(const Glib::ustring& group, const Glib::ustring& key) const
{
	return keyfile->get_integer_list(group, key);
}

UStringArray SettingsManager::get_groups() const
{
	return keyfile->get_groups();
}

UStringArray SettingsManager::get_keys(const Glib::ustring& group) const
{
	return keyfile->get_keys(group);
}

bool SettingsManager::has_group(const Glib::ustring& group) const
{
	return keyfile->has_group(group);
}
		
void SettingsManager::set(const Glib::ustring& group, const Glib::ustring& key, const Glib::ustring& value)
{
	keyfile->set_string(group, key, value);
}

void SettingsManager::set(const Glib::ustring& group, const Glib::ustring& key, int value)
{
	keyfile->set_integer(group, key, value);
}

void SettingsManager::set(const Glib::ustring& group, const Glib::ustring& key, bool value)
{
	keyfile->set_boolean(group, key, value);
}

void SettingsManager::set(const Glib::ustring& group, const Glib::ustring& key, UStringArray values)
{
	keyfile->set_string_list(group, key, values);
}

void SettingsManager::set(const Glib::ustring& group, const Glib::ustring& key, IntArray values)
{
	keyfile->set_integer_list(group, key, values);
}

void SettingsManager::remove_group(const Glib::ustring& group)
{
	keyfile->remove_group(group);
}

void SettingsManager::update()
{
	m_signal_update_settings.emit();
}

