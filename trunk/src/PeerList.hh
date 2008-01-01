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

#ifndef PEERLIST_HH
#define PEERLIST_HH

#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <libglademm.h>

#include "libtorrent/peer_id.hpp"

#include "linkage/Torrent.hh"

class PeerList : public Gtk::TreeView
{
	typedef std::map<Glib::ustring, libtorrent::peer_info> PeerMap;

	class ModelColumns : public Gtk::TreeModelColumnRecord
	{
	public:
		ModelColumns()
		{
			add(flag);
			add(encryption);
			add(address);
			add(down);
			add(up);
			add(down_rate);
			add(up_rate);
			add(progress);
			add(client);
			add(flags);
			add(source);
			add(remove);
		}
		Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > flag;
		Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > encryption;
		Gtk::TreeModelColumn<Glib::ustring> address;
		Gtk::TreeModelColumn<libtorrent::size_type> down;
		Gtk::TreeModelColumn<libtorrent::size_type> up;
		Gtk::TreeModelColumn<float> down_rate;
		Gtk::TreeModelColumn<float> up_rate;
		Gtk::TreeModelColumn<double> progress;
		Gtk::TreeModelColumn<Glib::ustring> client;
		Gtk::TreeModelColumn<Glib::ustring> flags;
		Gtk::TreeModelColumn<Glib::ustring> source;
		Gtk::TreeModelColumn<bool> remove;
	};

	ModelColumns columns;
	Glib::RefPtr<Gtk::ListStore> model;

	void format_data(Gtk::CellRenderer* cell,
		const Gtk::TreeIter& iter,
		const Gtk::TreeModelColumn<libtorrent::size_type>& column);
	void format_rates(Gtk::CellRenderer* cell,
		const Gtk::TreeIter& iter,
		const Gtk::TreeModelColumn<float>& column);

	bool on_foreach(const Gtk::TreeModel::iterator& iter, PeerMap* peer_map);
	void set_peer_details(Gtk::TreeRow& row, const libtorrent::peer_info& peer);

	Glib::ustring peer_as_string(const libtorrent::peer_info& peer);
public:
	void update(const Linkage::TorrentPtr& torrent);

	PeerList(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	virtual ~PeerList();
};

#endif /* PEERLIST_HH */
