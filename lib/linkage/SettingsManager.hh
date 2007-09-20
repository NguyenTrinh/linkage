/*
Copyright (C) 2006-2006		Rob Page
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

#include <map>

#include <sigc++/signal.h>

#include <glibmm/object.h>
#include <glibmm/ustring.h>

#include <gconfmm.h>

typedef Gnome::Conf::SListHandle_ValueString UStringArray;
typedef Gnome::Conf::SListHandle_ValueInt IntArray;
typedef Gnome::Conf::SListHandle_ValueBool BoolArray;

// FIXME: this is a crude hack to ease the use of our SettingsWidget template
class Value : public Gnome::Conf::Value
{
public:
	template<class T>
	T get_value() const
	{
		/* TODO nullpointer is a horrible hack */
		T* nullpointer = 0;
		return get_value(nullpointer);
	}

	Glib::ustring get_value(Glib::ustring* null) const
	{
		return get_string();
	}
	int get_value(int* null) const
	{
		return get_int();
	}
	float get_value(float* null) const
	{
		return get_float();
	}
	bool get_value(bool* null) const
	{
		return get_bool();
	}

	Value(const Gnome::Conf::Value& value) : Gnome::Conf::Value(value) {}
	virtual ~Value() {}
};

class SettingsManager : public Glib::Object
{
	SettingsManager();

	Glib::RefPtr<Gnome::Conf::Client> gconf;

	sigc::signal<void, const Glib::ustring&, const Value&> m_signal_key_changed;
	std::map<Glib::ustring, sigc::signal<void, const Value&> > m_signals;

	void on_changed(guint id, Gnome::Conf::Entry entry);

public:
	Glib::ustring get_string(const Glib::ustring& path) const;
	int get_int(const Glib::ustring& path) const;
	float get_float(const Glib::ustring& path) const;
	bool get_bool(const Glib::ustring& path) const;
	UStringArray get_string_list(const Glib::ustring& path) const;
	IntArray get_int_list(const Glib::ustring& path) const;
	gint get_enum(const Glib::ustring& path, GConfEnumStringPair* table) const;

	void set(const Glib::ustring& path, const Glib::ustring& value);
	void set(const Glib::ustring& path, const int value);
	void set(const Glib::ustring& path, const float value);
	void set(const Glib::ustring& path, const bool value);
	void set(const Glib::ustring& path, const Glib::SListHandle<Glib::ustring> &values);
	void set(const Glib::ustring& path, const Gnome::Conf::Client::SListHandleInts &values);
	void set(const Glib::ustring& path, GConfEnumStringPair* table, gint value);

	sigc::signal<void, const Value&> signal(const Glib::ustring& key);	
	sigc::signal<void, const Glib::ustring&, const Value&> signal_key_changed();

	static Glib::RefPtr<SettingsManager> create();
	~SettingsManager();
};

#endif /* SETTINGSMANAGER_HH */
