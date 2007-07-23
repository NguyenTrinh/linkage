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

#include <glibmm/i18n.h>

#include "linkage/Engine.hh"
#include "linkage/SettingsManager.hh"

#include "StateFilter.hh"

StateFilter::StateFilter(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::ComboBox(cobject),
	glade_xml(refGlade)
{
	model = Gtk::ListStore::create(columns);

	set_model(model);

	Torrent tmp;

	Gtk::TreeRow row = *(model->append());
	row[columns.name] = _("All");
	row[columns.state] = Torrent::NONE;

	for (int i = Torrent::ANNOUNCING; i != Torrent::ERROR; i *= 2)
	{
		Torrent::State state = Torrent::State(i);
		row = *(model->append());
		row[columns.name] = tmp.get_state_string(state);
		row[columns.state] = state;
	}

	pack_start(columns.name);

	signal_changed().connect(sigc::mem_fun(this, &StateFilter::on_selection_changed));

	show_all_children();
}

StateFilter::~StateFilter()
{
	int active = get_active_row_number();
	Engine::get_settings_manager()->set("ui/active_state", active);
}

sigc::signal<void, Torrent::State> StateFilter::signal_state_filter_changed()
{
	return m_signal_state_filter_changed;
}

void StateFilter::on_selection_changed()
{
	Gtk::TreeIter iter = get_active();
	if (iter)
	{
		Gtk::TreeRow row = *iter;
		m_signal_state_filter_changed.emit(row[columns.state]);
	}
}

void StateFilter::reselect()
{
	int active = Engine::get_settings_manager()->get_int("ui/active_state");
	set_active(active);
}

