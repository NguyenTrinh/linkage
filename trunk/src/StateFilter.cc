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

using namespace Linkage;

static GConfEnumStringPair active_state_table[] = {
	{ -1, NULL },
  { 0, "STATE_ALL" },
  { 1, "STATE_ANNOUNCING" },
  { 2, "STATE_DOWNLOADING" },
  { 3, "STATE_FINISHED" },
  { 4, "STATE_SEEDING" },
  { 5, "STATE_CHECK_QUEUE" },
  { 6, "STATE_CHECKING" },
  { 7, "STATE_ALLOCATING" },
  { 8, "STATE_ALLOCATING" },
  { 9, "STATE_QUEUED" },
  { 10, "STATE_ERROR" }
};

StateFilter::StateFilter(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::ComboBox(cobject),
	glade_xml(refGlade)
{
	model = Gtk::ListStore::create(columns);

	set_model(model);

	Gtk::TreeRow row = *(model->append());
	row[columns.name] = _("All");
	row[columns.state] = Torrent::NONE;

	for (int i = Torrent::ANNOUNCING; i != Torrent::ERROR; i *= 2)
	{
		row = *(model->append());

		Torrent::State state = Torrent::State(i);
		row[columns.name] = Torrent::state_string(state);
		row[columns.state] = state;
	}

	pack_start(columns.name);

	signal_changed().connect(sigc::mem_fun(this, &StateFilter::on_selection_changed));

	show_all_children();
}

StateFilter::~StateFilter()
{
	Engine::get_settings_manager()->set("ui/active_state", active_state_table, get_active_row_number());
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
	gint active = Engine::get_settings_manager()->get_enum("ui/active_state", active_state_table);
	set_active(active);
}

