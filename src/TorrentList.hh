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
#include <gtkmm/treestore.h>
#include <gdkmm/pixbuf.h>

#include "linkage/Torrent.hh"
#include "linkage/WeakPtr.hh"
#include "GroupFilter.hh"

class TorrentList : public Gtk::TreeView
{
	class ModelColumns : public Gtk::TreeModelColumnRecord
	{
	public:
		ModelColumns()
			{
				add(children);
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
				add(is_group);
				add(hash);
			}
		Gtk::TreeModelColumn<sha1_hash> hash;
		Gtk::TreeModelColumn<unsigned int> children;
		Gtk::TreeModelColumn<unsigned int> position;
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<double> progress;
		Gtk::TreeModelColumn<Glib::ustring> state;
		Gtk::TreeModelColumn<unsigned int> down;
		Gtk::TreeModelColumn<unsigned int> up;
		Gtk::TreeModelColumn<unsigned int> down_rate;
		Gtk::TreeModelColumn<unsigned int> up_rate;
		Gtk::TreeModelColumn<unsigned int> seeds;
		Gtk::TreeModelColumn<unsigned int> peers;
		Gtk::TreeModelColumn<Glib::ustring> eta;
		Gtk::TreeModelColumn<bool> is_group;
	}; 
	ModelColumns columns;
	
	Glib::RefPtr<Gtk::TreeStore> model;
	
	std::list<GroupFilter*> filters;
	
	Gtk::TreeIter get_iter(const Glib::ustring& group);
	Gtk::TreeIter get_iter(const sha1_hash& hash);
	
	Gtk::TreeIter add_group(const Glib::ustring& name);
	void select(const Glib::ustring& path);
	
	void on_position_changed(const sha1_hash& hash, unsigned int position);
	void on_group_changed(const sha1_hash& hash, const Glib::ustring& group);
	
	void on_added(const sha1_hash& hash, const Glib::ustring& name, const Glib::ustring& group, unsigned int position);
	void on_removed(const sha1_hash& hash);
	
	void on_session_resumed();
	
	void format_rates(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter);
	void format_children(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter);
	void format_position(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter);
	
	bool on_button_press_event(GdkEventButton *event);

	sigc::signal<void, const sha1_hash&> m_signal_double_click;
	sigc::signal<void, const sha1_hash&> m_signal_right_click;
	
public:
	enum Column 
	{
		COL_CHILDREN,
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
		COL_IS_GROUP,
		COL_HASH
	};
	
	bool is_selected(const sha1_hash& hash);
	HashList get_selected_list();
	
	void set_sort_column(Column col_id);
	void set_sort_order(Gtk::SortType order);
	
	void update_groups();
	void update_row(const WeakPtr<Torrent>& torrent);
	
	Glib::SignalProxy0<void> signal_changed();
	sigc::signal<void, const sha1_hash&> signal_double_click();
	sigc::signal<void, const sha1_hash&> signal_right_click();

	TorrentList();
	virtual ~TorrentList();
};

#endif	/* TORRENTLIST_HH */
