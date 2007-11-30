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

#include "SettingsWidget.hh"

using namespace Linkage;

void CheckButtonSetting::init(const Glib::ustring& key_)
{
	if (key.empty())
		set_key(key_);
	g_return_if_fail(!key.empty());

	set_active(Engine::get_settings_manager()->get_bool(key));

	signal_toggled().connect(
		sigc::mem_fun(this, &SettingsWidget<bool>::on_changed));
}

void ColorButtonSetting::init(const Glib::ustring& key_)
{
	if (key.empty())
		set_key(key_);
	g_return_if_fail(!key.empty());

	set_color(Gdk::Color(Engine::get_settings_manager()->get_string(key)));

	signal_color_set().connect(
		sigc::mem_fun(this, &SettingsWidget<Glib::ustring>::on_changed));
}

void EntrySetting::init(const Glib::ustring& key_)
{
	if (key.empty())
		set_key(key_);
	g_return_if_fail(!key.empty());

	set_text(Engine::get_settings_manager()->get_string(key));

	signal_activate().connect(
		sigc::mem_fun(this, &SettingsWidget<Glib::ustring>::on_changed));
}

void FileChooserButtonSetting::init(const Glib::ustring& key_)
{
	if (key.empty())
		set_key(key_);
	g_return_if_fail(!key.empty());

	set_filename(Engine::get_settings_manager()->get_string(key));

	signal_selection_changed().connect(
		sigc::mem_fun(this, &SettingsWidget<Glib::ustring>::on_changed));
}

void ComboBoxSetting::init(const Glib::ustring& key_)
{
	if (key.empty())
		set_key(key_);
	g_return_if_fail(!key.empty());

	set_value(Engine::get_settings_manager()->get_string(key));

	signal_changed().connect(
		sigc::mem_fun(this, &SettingsWidget<Glib::ustring>::on_changed));
}

void ComboBoxSetting::set_value(Glib::ustring value)
{
	Gtk::TreeNodeChildren children = model->children();
	for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
	{
		Gtk::TreeRow row = *iter;
		if (row[columns.value] == value)
		{
			set_active(iter);
			break;
		}
	}
}

Glib::ustring ComboBoxSetting::get_value()
{
	Gtk::TreeIter iter = get_active();
	Glib::ustring ret;
	if (iter)
	{
		Gtk::TreeRow row = *iter;
		ret = row[columns.value];
	}
	return ret;
}

void ComboBoxSetting::append_pair(const Glib::ustring& name, const Glib::ustring& value)
{
	Gtk::TreeRow row = *(model->append());
	row[columns.name] = name;
	row[columns.value] = value;
}

