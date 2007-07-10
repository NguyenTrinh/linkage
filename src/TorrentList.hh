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

#ifndef TORRENTLIST_HH
#define TORRENTLIST_HH

#include <list>

#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelfilter.h>
#include <libglademm/xml.h>

#include "linkage/Torrent.hh"
#include "linkage/WeakPtr.hh"

#include "Group.hh"

class TorrentList : public Gtk::TreeView
{
	class ModelColumns : public Gtk::TreeModelColumnRecord
	{
	public:
		ModelColumns()
			{
				add(position);
				add(name);
				add(progress);
				add(state);
				add(down);
				add(up);
				add(down_rate);
				add(up_rate);
				add(seeds);
				add(peers);
				add(eta);
				add(hash);
			}
		Gtk::TreeModelColumn<unsigned int> position;
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<double> progress;
		Gtk::TreeModelColumn<Glib::ustring> state;
		Gtk::TreeModelColumn<size_type> down;
		Gtk::TreeModelColumn<size_type> up;
		Gtk::TreeModelColumn<float> down_rate;
		Gtk::TreeModelColumn<float> up_rate;
		Gtk::TreeModelColumn<unsigned int> seeds;
		Gtk::TreeModelColumn<unsigned int> peers;
		Gtk::TreeModelColumn<Glib::ustring> eta;
		Gtk::TreeModelColumn<sha1_hash> hash;
	};
	ModelColumns columns;

	Glib::RefPtr<Gtk::ListStore> model;
	Glib::RefPtr<Gtk::TreeModelFilter> filter;
	
	Glib::RefPtr<Gnome::Glade::Xml> glade_xml;

	Group m_active_group;
	Torrent::State m_cur_state;

	Gtk::TreeIter get_iter(const sha1_hash& hash);

	void on_added(const sha1_hash& hash, const Glib::ustring& name, unsigned int position);
	void on_removed(const sha1_hash& hash);

	Glib::ustring format_name(const WeakPtr<Torrent>& torrent);
	void format_rates(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter);

	bool on_button_press_event(GdkEventButton *event);
	
	bool on_filter(const Gtk::TreeModel::const_iterator& iter);

	sigc::signal<void, GdkEventButton*> m_signal_double_click;
	sigc::signal<void, GdkEventButton*> m_signal_right_click;

public:
	enum Column
	{
		COL_POSITION,
		COL_NAME,
		COL_PROGRESS,
		COL_STATUS,
		COL_DOWNLOADED,
		COL_UPLOADED,
		COL_DOWNRATE,
		COL_UPRATE,
		COL_SEEDS,
		COL_PEERS,
		COL_ETA,
		COL_HASH
	};
	
	void on_filter_set(const Group& group);
	void on_state_filter_changed(Torrent::State state);

	bool is_selected(const sha1_hash& hash);
	HashList get_selected_list();

	void set_sort_column(Column col_id);
	void set_sort_order(Gtk::SortType order);

	void update(const WeakPtr<Torrent>& torrent);

	Glib::SignalProxy0<void> signal_changed();
	sigc::signal<void, GdkEventButton*> signal_double_click();
	sigc::signal<void, GdkEventButton*> signal_right_click();

	TorrentList(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	virtual ~TorrentList();
};

#endif	/* TORRENTLIST_HH */
