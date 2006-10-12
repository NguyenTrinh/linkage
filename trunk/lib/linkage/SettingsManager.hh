/*
Copyright (C) 2006  Christian Lundgren

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifndef SETTINGSMANAGER_HH
#define SETTINGSMANAGER_HH

#include <sigc++/signal.h>
#include <glibmm/ustring.h>
#include <gtkmm_extra/keyfile.h>
#include <iostream>


typedef Glib::ArrayHandle<Glib::ustring> UStringArray;
typedef Glib::ArrayHandle<int> IntArray;
typedef Glib::ArrayHandle<bool> BoolArray;

enum { NUM_DEFAULT_GROUPS = 4 };

class SettingsManager 
{
public:
  static SettingsManager* instance();
  static void goodnight();

  template <class T> T get(const Glib::ustring& group, const Glib::ustring& key)
	{
    /* TODO nullpointer is a horrible hack */
		T* nullpointer = 0;
		try
		{
			return get(group, key, nullpointer);
		}
		catch (Glib::Error& error)
		{
      std::cerr << error.what() << std::endl;
    }
  }
  
  Glib::ustring get(const Glib::ustring& group, const Glib::ustring& key, Glib::ustring* nullpointer) const;
  int get(const Glib::ustring& group, const Glib::ustring& key, int* nullpointer) const;
  bool get(const Glib::ustring& group, const Glib::ustring& key, bool* nullpointer) const;
  UStringArray get(const Glib::ustring& group, const Glib::ustring& key, UStringArray* nullpointer) const;
  IntArray get(const Glib::ustring& group, const Glib::ustring& key, IntArray* nullpointer) const;
  
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
  
  //FIXME: Should not be public
  sigc::signal<void> signal_update_settings_;
  
protected:
  
  Glib::KeyFile keyfile;

  static SettingsManager* smInstance;

  SettingsManager();
  ~SettingsManager();
  
  static Glib::ustring defaults;
};

#endif /* SETTINGSMANAGER_HH */
