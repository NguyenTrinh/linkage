/*
Copyright (C) 2006-2007   Christian Lundgren

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

#ifndef SETTINGS_WIDGET_HH
#define SETTINGS_WIDGET_HH

#include <sigc++/slot.h>

#include <glibmm/ustring.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>

#include <libglademm.h>

#include "linkage/Engine.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/Utils.hh"

template<class T> class SettingsWidget
{
protected:
	sigc::slot<T> get_value_func;
	sigc::slot<void, T> set_value_func;

	Glib::ustring key;

	virtual void on_extern_changed(const Linkage::Value& value)
	{
		T val = value.get_value<T>();
		if (val != (T)get_value_func())
			set_value_func(val);
	}
public:
	virtual void on_changed()
	{
		Linkage::Engine::get_settings_manager()->set(key, (T)get_value_func());
	}

	virtual void init(const Glib::ustring& key_ = Glib::ustring()) = 0;
	virtual void set_key(const Glib::ustring& key_)
	{
		key = key_;
		Linkage::Engine::get_settings_manager()->signal(key).connect(
			sigc::mem_fun(this, &SettingsWidget<T>::on_extern_changed));
	}
	virtual const Glib::ustring& get_key()
	{
		return key;
	}

	SettingsWidget() {}
	virtual ~SettingsWidget() {}
};

//spinbuttons are used to store both ints and floats
template<class T> class SpinButtonSetting : public Gtk::SpinButton, public SettingsWidget<T>
{
	void set_template_value(T value)
	{
		set_value((double)value);
	}
	T get_template_value()
	{
		return (T)get_value();
	}
public:
	void init(const Glib::ustring& key_ = Glib::ustring())
	{
		if (SettingsWidget<T>::key.empty())
			SettingsWidget<T>::set_key(key_);
		g_return_if_fail(!SettingsWidget<T>::key.empty());

		if (typeid(T) == typeid(int))
			set_value((double)Linkage::Engine::get_settings_manager()->get_int(SettingsWidget<T>::key));
		else if (typeid(T) == typeid(float))
			set_value((double)Linkage::Engine::get_settings_manager()->get_float(SettingsWidget<T>::key));
		else
			g_warning("Unkown type value, ignoring.");

		signal_changed().connect(
			sigc::mem_fun(this, &SettingsWidget<T>::on_changed));
	}
	SpinButtonSetting(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& xml)
		: Gtk::SpinButton(cobject)
	{
		SettingsWidget<T>::set_value_func = sigc::mem_fun(this, &SpinButtonSetting::set_template_value);
		SettingsWidget<T>::get_value_func = sigc::mem_fun(this, &SpinButtonSetting::get_template_value);
	}
};

class CheckButtonSetting : public Gtk::CheckButton, public SettingsWidget<bool>
{
public:
	void init(const Glib::ustring& key_ = Glib::ustring());
	CheckButtonSetting(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& xml)
		: Gtk::CheckButton(cobject)
	{
		set_value_func = sigc::mem_fun(this, &Gtk::CheckButton::set_active);
		get_value_func = sigc::mem_fun(this, &Gtk::CheckButton::get_active);
	}
};

class ColorButtonSetting : public Gtk::ColorButton, public SettingsWidget<Glib::ustring>
{
	void set_value(Glib::ustring hex)
	{
		set_color(Gdk::Color(hex));
	}
	Glib::ustring get_value()
	{
		return Linkage::hex_color(get_color());
	}
public:
	void init(const Glib::ustring& key_ = Glib::ustring());
	ColorButtonSetting(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& xml)
		: Gtk::ColorButton(cobject)
	{
		set_value_func = sigc::mem_fun(this, &ColorButtonSetting::set_value);
		get_value_func = sigc::mem_fun(this, &ColorButtonSetting::get_value);
	}
};

class EntrySetting : public Gtk::Entry, public SettingsWidget<Glib::ustring>
{
public:
	void init(const Glib::ustring& key_ = Glib::ustring());
	EntrySetting(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& xml)
		: Gtk::Entry(cobject)
	{
		set_value_func = sigc::mem_fun(this, &Gtk::Entry::set_text);
		get_value_func = sigc::mem_fun(this, &Gtk::Entry::get_text);
	}
};

class FileChooserButtonSetting : public Gtk::FileChooserButton, public SettingsWidget<Glib::ustring>
{
	//set_filename returns bool, doesn't match our slots
	void set_value(Glib::ustring path)
	{
		set_filename(path);
	}
public:
	void init(const Glib::ustring& key_ = Glib::ustring());
	FileChooserButtonSetting(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& xml)
		: Gtk::FileChooserButton(cobject)
	{
		set_value_func = sigc::mem_fun(this, &FileChooserButtonSetting::set_value);
		get_value_func = sigc::mem_fun(this, &Gtk::FileChooserButton::get_filename);
	}
};

class ComboBoxSetting : public Gtk::ComboBox, public SettingsWidget<Glib::ustring>
{
	class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:
    ModelColumns()
    {
    	add(name);
    	add(value);
    }
    Gtk::TreeModelColumn<Glib::ustring> name;
    Gtk::TreeModelColumn<Glib::ustring> value; //maps to a GConfEnumStringPair
  };
	ModelColumns columns;
	Glib::RefPtr<Gtk::ListStore> model;

	void set_value(Glib::ustring value);
	Glib::ustring get_value();
public:
	void append_pair(const Glib::ustring& name, const Glib::ustring& value);
	void init(const Glib::ustring& key_ = Glib::ustring());
	ComboBoxSetting(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& xml)
		: Gtk::ComboBox(cobject)
	{
		set_value_func = sigc::mem_fun(this, &ComboBoxSetting::set_value);
		get_value_func = sigc::mem_fun(this, &ComboBoxSetting::get_value);

		model = Gtk::ListStore::create(columns);
		set_model(model);
		pack_start(columns.name);
	}
};

#endif /* SETTINGS_WIDGET_HH */

