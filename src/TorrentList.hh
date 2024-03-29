/*
Copyright (C) 2006-2008   Christian Lundgren
Copyright (C) 2008        Dave Moore

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

#include <vector>

#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelfilter.h>
#include <libglademm/xml.h>

#include "linkage/Torrent.hh"

#include "EditColumnsDialog.hh"
#include "TorrentListRenderers.hh"
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
			add(ratio);
			add(tracker);
			add(tracker_status);
			add(hash);
			add(total_seeds);
			add(total_peers);
			add(color);
		}

		Gtk::TreeModelColumn<unsigned int> position;
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<float> progress;
		Gtk::TreeModelColumn<Glib::ustring> state;
		Gtk::TreeModelColumn<libtorrent::size_type> down;
		Gtk::TreeModelColumn<libtorrent::size_type> up;
		Gtk::TreeModelColumn<float> down_rate;
		Gtk::TreeModelColumn<float> up_rate;
		Gtk::TreeModelColumn<int> seeds;
		Gtk::TreeModelColumn<int> peers;
		Gtk::TreeModelColumn<libtorrent::size_type> eta;
		Gtk::TreeModelColumn<float> ratio;
		Gtk::TreeModelColumn<Glib::ustring> tracker;
		Gtk::TreeModelColumn<Glib::ustring> tracker_status;
		Gtk::TreeModelColumn<libtorrent::sha1_hash> hash;
		Gtk::TreeModelColumn<int> total_seeds;
		Gtk::TreeModelColumn<int> total_peers;
		Gtk::TreeModelColumn<Glib::ustring> color;
	};
	ModelColumns columns;

	Glib::RefPtr<Gtk::ListStore> model;
	Glib::RefPtr<Gtk::TreeModelFilter> filter;

	Glib::RefPtr<Gnome::Glade::Xml> glade_xml;

	GroupPtr m_active_group;
	Linkage::Torrent::State m_cur_state;
	
	ColumnDataList m_cols;

	void on_added(const Linkage::TorrentPtr& torrent);
	void on_removed(const Linkage::TorrentPtr& torrent);
	void on_position_changed(const Linkage::WeakTorrentPtr& weak);
	void on_state_changed(const Linkage::WeakTorrentPtr& weak);

	Glib::ustring get_state_color(int state);

	bool on_button_press_event(GdkEventButton *event);
	bool on_key_press_event(GdkEventKey *event);

	bool on_filter(const Gtk::TreeModel::const_iterator& iter);

	sigc::signal<void, GdkEventButton*> m_signal_double_click;
	sigc::signal<void, GdkEventButton*> m_signal_right_click;
	sigc::signal<void, GdkEventKey*> m_signal_key_press;

	void on_filter_set(const GroupPtr& group);
	void on_state_filter_changed(Linkage::Torrent::State state);

	void update_row(Gtk::TreeRow& row);

	void save_column_setup();
	std::string parse_column_string(std::string raw_order);

	friend class UI;

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
		COL_RATIO,
		COL_TRACKER,
		COL_TRACKER_STATUS,
		COL_HASH,
		TOTAL_COLS
	}; 

	bool is_selected(const libtorrent::sha1_hash& hash);
	Linkage::HashList get_selected_list();

	const ColumnDataList& get_column_data() const { return m_cols; }
	void toggle_column(int col);

	void set_sort_column(Column col_id);
	void set_sort_order(Gtk::SortType order);

	void update();

	Glib::SignalProxy0<void> signal_changed();
	sigc::signal<void, GdkEventButton*> signal_double_click();
	sigc::signal<void, GdkEventButton*> signal_right_click();
	sigc::signal<void, GdkEventKey*> signal_key_press();

	TorrentList(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	virtual ~TorrentList();
};

#endif	/* TORRENTLIST_HH */
