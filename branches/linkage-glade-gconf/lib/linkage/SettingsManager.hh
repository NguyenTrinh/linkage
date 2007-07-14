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
#ifndef SETTINGSMANAGER_HH
#define SETTINGSMANAGER_HH

#include <sigc++/signal.h>
#include <glibmm/ustring.h>

#include <gconfmm.h>

#include "linkage/RefCounter.hh"

typedef Gnome::Conf::SListHandle_ValueString UStringArray;
typedef Gnome::Conf::SListHandle_ValueInt IntArray;
typedef Gnome::Conf::SListHandle_ValueBool BoolArray;

enum { NUM_DEFAULT_GROUPS = 4 };

class SettingsManager : public RefCounter<SettingsManager>
{
	SettingsManager();

	Glib::RefPtr<Gnome::Conf::Client> gconf;
	
	sigc::signal<void> m_signal_update_settings;
	
public: 
	Glib::ustring get_string(const Glib::ustring& path) const;
	int get_int(const Glib::ustring& path) const;
	double get_float(const Glib::ustring& path) const;
	bool get_bool(const Glib::ustring& path) const;
	UStringArray get_string_list(const Glib::ustring& path) const;
	IntArray get_int_list(const Glib::ustring& path) const;
	
	void set(const Glib::ustring& path, const Glib::ustring& value);
	void set(const Glib::ustring& path, const int value);
	void set(const Glib::ustring& path, const double value);
	void set(const Glib::ustring& path, const bool value);
	void set(const Glib::ustring& path, const Glib::SListHandle<Glib::ustring> &values);
	void set(const Glib::ustring& path, const Gnome::Conf::Client::SListHandleInts &values);
	
	sigc::signal<void> signal_update_settings();
	
	void update();
	
	static Glib::RefPtr<SettingsManager> create();
	~SettingsManager();
};

#endif /* SETTINGSMANAGER_HH */
