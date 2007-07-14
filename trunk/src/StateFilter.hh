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

#ifndef STATE_FILTER_HH
#define STATE_FILTER_HH

#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>
#include <libglademm.h>

#include "linkage/Torrent.hh"

class StateFilter : public Gtk::ComboBox
{
	Glib::RefPtr<Gnome::Glade::Xml> glade_xml;

	class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:
    ModelColumns()
    {
    	add(name);
    	add(state);
    }
    Gtk::TreeModelColumn<Glib::ustring> name;
    Gtk::TreeModelColumn<Torrent::State> state;
  };
	ModelColumns columns;
	Glib::RefPtr<Gtk::ListStore> model;

	sigc::signal<void, Torrent::State> m_signal_state_filter_changed;

	void on_selection_changed();

public:
	sigc::signal<void, Torrent::State> signal_state_filter_changed();

	void reselect();

	StateFilter(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	~StateFilter();
};

#endif /* STATE_FILTER_HH */

