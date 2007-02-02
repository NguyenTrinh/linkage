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
#ifndef SETTINGSMANAGER_HH
#define SETTINGSMANAGER_HH

#include <sigc++/signal.h>
#include <glibmm/ustring.h>
#include <gtkmm_extra/keyfile.h>
#include "linkage/RefCounter.hh"

typedef Glib::ArrayHandle<Glib::ustring> UStringArray;
typedef Glib::ArrayHandle<int> IntArray;
typedef Glib::ArrayHandle<bool> BoolArray;

enum { NUM_DEFAULT_GROUPS = 4 };

class SettingsManager : public RefCounter<SettingsManager>
{
public: 
	Glib::ustring get_string(const Glib::ustring& group, const Glib::ustring& key) const;
	int get_int(const Glib::ustring& group, const Glib::ustring& key) const;
	bool get_bool(const Glib::ustring& group, const Glib::ustring& key) const;
	UStringArray get_string_list(const Glib::ustring& group, const Glib::ustring& key) const;
	IntArray get_int_list(const Glib::ustring& group, const Glib::ustring& key) const;
	
	void set(const Glib::ustring& group, const Glib::ustring& key, const Glib::ustring& value);
	void set(const Glib::ustring& group, const Glib::ustring& key, int value);
	void set(const Glib::ustring& group, const Glib::ustring& key, bool value);
	void set(const Glib::ustring& group, const Glib::ustring& key, UStringArray values);
	void set(const Glib::ustring& group, const Glib::ustring& key, IntArray values);

	void remove_group(const Glib::ustring& group);
	
	UStringArray get_groups() const;
	UStringArray get_keys(const Glib::ustring& group) const;
	
	bool has_group(const Glib::ustring& group) const;
	
	sigc::signal<void> signal_update_settings();
	
	void update();
	
	static Glib::RefPtr<SettingsManager> create();
	~SettingsManager();
	
protected:
	Glib::KeyFile* keyfile;
	
	sigc::signal<void> m_signal_update_settings;
	
	static Glib::ustring defaults;
	
	SettingsManager();
};

#endif /* SETTINGSMANAGER_HH */
