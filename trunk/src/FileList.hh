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

//#include <gtkmm/menu.h>
//#include <gtkmm/checkmenuitem.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gdkmm/pixbuf.h>
#include <libglademm.h>

#include "linkage/Torrent.hh"
#include "linkage/WeakPtr.hh"

class FileList : public Gtk::TreeView
{
	enum Priority { P_NORMAL = 1, P_HIGH, P_MAX = 7};
	class ModelColumns : public Gtk::TreeModelColumnRecord
	{
	public:
		ModelColumns()
		{
			add(inconsistent);
			add(filter);
			add(icon);
			add(name);
			add(map);
			add(done);
			add(size);
			add(index);
		}
		Gtk::TreeModelColumn<bool> inconsistent;
		Gtk::TreeModelColumn<bool> filter;
		Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > icon;
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<std::vector<bool> > map;
		Gtk::TreeModelColumn<libtorrent::size_type> done;
		Gtk::TreeModelColumn<libtorrent::size_type> size;
		Gtk::TreeModelColumn<int> index;
	};
	
	ModelColumns columns;
	Glib::RefPtr<Gtk::TreeStore> model;

	//void on_set_priority(Priority p);

	typedef struct
	{
		libtorrent::torrent_info info;
		std::vector<bool> pieces;
		std::vector<float> file_progress;
		std::vector<bool> filter;
	} FileData;

	typedef std::list<Gtk::TreeIter> IterList;
	bool on_foreach(const Gtk::TreeModel::iterator& iter, IterList* list);
	void on_reverse_foreach(const Gtk::TreeModel::iterator& iter, const FileData& data);

	void refill_tree(const libtorrent::torrent_info& info);

	void on_filter_toggled(const Glib::ustring& path);
	void filter_children(const Gtk::TreeNodeChildren& children, bool filter);

	void format_data(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter, const Gtk::TreeModelColumn<libtorrent::size_type>& column);
	
	libtorrent::sha1_hash current_hash;
	
	int compare_piece_map(const Gtk::TreeIter& a, const Gtk::TreeIter& b);
	
public:
	void clear();
	void update(const WeakPtr<Torrent>& torrent);
	
	FileList(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	virtual ~FileList();
};

#endif /* FILELIST_HH */
