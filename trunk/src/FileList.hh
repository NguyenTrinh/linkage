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

#ifndef FILELIST_HH
#define FILELIST_HH

#include <gtkmm/menu.h>
#include <gtkmm/radiomenuitem.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gdkmm/pixbuf.h>
#include <libglademm.h>

#include "linkage/Torrent.hh"

class FileList : public Gtk::TreeView
{
	// See libtorrent docs for info about these values
	enum Priority { P_SKIP, P_NORMAL, P_HIGH = 4, P_MAX = 6};

	class ModelColumns : public Gtk::TreeModelColumnRecord
	{
	public:
		ModelColumns()
		{
			add(inconsistent);
			add(icon);
			add(name);
			add(map);
			add(done);
			add(size);
			add(priority);
			add(index);
		}
		Gtk::TreeModelColumn<bool> inconsistent;
		Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<std::vector<bool> > map;
		Gtk::TreeModelColumn<libtorrent::size_type> done;
		Gtk::TreeModelColumn<libtorrent::size_type> size;
		Gtk::TreeModelColumn<Priority> priority;
		Gtk::TreeModelColumn<int> index;
	};
	
	ModelColumns columns;
	Glib::RefPtr<Gtk::TreeStore> model;

	Gtk::Menu m_menu;
	Gtk::RadioMenuItem *m_radio_max, *m_radio_high, *m_radio_normal, *m_radio_skip;

	void prioritize_row(const Gtk::TreeRow& row, Priority priority);
	void on_set_priority(Priority priority);

	bool on_button_press_event(GdkEventButton *event);

	typedef struct
	{
		boost::intrusive_ptr<libtorrent::torrent_info> info;
		std::vector<bool> pieces;
		std::vector<float> file_progress;
		std::vector<int> priorities;
	} FileData;

	typedef std::list<Gtk::TreeIter> IterList;
	bool on_foreach(const Gtk::TreeIter& iter, const FileData& data);

	void refill_tree(const boost::intrusive_ptr<libtorrent::torrent_info>& info);

	void format_data(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter, const Gtk::TreeModelColumn<libtorrent::size_type>& column);
	void format_priority(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter);
	
	Linkage::TorrentPtr m_cur_torrent;
	
	int compare_piece_map(const Gtk::TreeIter& a, const Gtk::TreeIter& b);

	std::pair<int,int> get_piece_range(const Gtk::TreeRow& row);

public:
	void clear();
	void update(const Linkage::TorrentPtr& torrent);
	
	FileList(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	~FileList();
};

#endif /* FILELIST_HH */
