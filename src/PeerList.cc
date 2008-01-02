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

#include <gtkmm/main.h>
#include <gtkmm/stock.h>
#include <gtkmm/image.h>
#include <gtkmm/treeselection.h>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/cellrendererprogress.h>
#include <glibmm/i18n.h>

#include "libtorrent/identify_client.hpp"

#include "PeerList.hh"
#include "linkage/Utils.hh"
#include "linkage/compose.hpp"

/* dbus-c++ defines these ... */
#ifdef HAVE_CONFIG_H 
#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef VERSION

#include "config.h"
#endif

using namespace Linkage;

PeerList::PeerList(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::TreeView(cobject)
{
	model = Gtk::ListStore::create(columns);

	set_model(model);

	Gtk::TreeViewColumn* column = Gtk::manage(new Gtk::TreeViewColumn(_("Address")));
	column->set_sort_column(columns.address);
	column->set_resizable(true);
	column->pack_start(columns.flag, false);
	column->pack_start(columns.encryption, false);
	column->pack_start(columns.address);
	append_column(*column);

	int col = append_column(_("Downloaded"), columns.down);
	column = get_column(col - 1);
	Gtk::CellRendererText* cell = dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell_renderer());
	column->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(this, &PeerList::format_data), columns.down));
	col = append_column(_("Uploaded"), columns.up);
	column = get_column(col - 1);
	cell = dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell_renderer());
	column->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(this, &PeerList::format_data), columns.up));
	col = append_column(_("Download rate"), columns.down_rate);
	column = get_column(col - 1);
	cell = dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell_renderer());
	column->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(this, &PeerList::format_rates), columns.down_rate));
	col = append_column(_("Upload rate"), columns.up_rate);
	column = get_column(col - 1);
	cell = dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell_renderer());
	column->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(this, &PeerList::format_rates), columns.up_rate));

	Gtk::CellRendererProgress* prender = Gtk::manage(new Gtk::CellRendererProgress());
	col = append_column(_("Progress"), *prender);
	get_column(col - 1)->add_attribute(*prender, "value", columns.progress);
	append_column(_("Client"), columns.client);
	append_column(_("State"), columns.flags);
	append_column(_("Source"), columns.source);

	for (unsigned int i = 1; i < 9; i++)
	{
		column = get_column(i);
		column->set_sort_column_id(i + 2);
		column->set_resizable(true);
	}
}

PeerList::~PeerList()
{
}

void PeerList::format_data(Gtk::CellRenderer* cell,
	const Gtk::TreeIter& iter,
	const Gtk::TreeModelColumn<libtorrent::size_type>& column)
{
	Gtk::TreeRow row = *iter;
	Gtk::CellRendererText* cell_text = dynamic_cast<Gtk::CellRendererText*>(cell);
	cell_text->property_text() = suffix_value(row[column]);
}

void PeerList::format_rates(Gtk::CellRenderer* cell,
	const Gtk::TreeIter& iter,
	const Gtk::TreeModelColumn<float>& column)
{
	Gtk::TreeRow row = *iter;
	Gtk::CellRendererText* cell_text = dynamic_cast<Gtk::CellRendererText*>(cell);
	cell_text->property_text() = suffix_value(row[column]) + "/s";
}

bool PeerList::on_foreach(const Gtk::TreeModel::iterator& iter, PeerMap* peer_map)
{
	Gtk::TreeRow row = *iter;
	PeerMap::iterator peer_iter = peer_map->find(row[columns.address]);
	if (peer_iter != peer_map->end())
	{
		row[columns.remove] = false;

		Glib::ustring s = row[columns.address];

		set_peer_details(row, peer_iter->second);
		peer_map->erase(peer_iter);

		/* Give the UI some love, since this loop is pretty slow with many peers */
		while (Gtk::Main::events_pending())
			Gtk::Main::iteration(false);
	}

	return false;
}

void PeerList::update(const TorrentPtr& torrent)
{
	static bool working = false;

	if (working)
		return;
	else
		working = true;

	std::vector<libtorrent::peer_info> peers;
	if (!torrent->is_stopped())
		torrent->get_handle().get_peer_info(peers);

	if (peers.empty()) {
		model->clear();
		working = false;
		return;
	}

	PeerMap peer_map;
	for (unsigned int i = 0; i < peers.size(); i++)
	{
		peer_map[peer_as_string(peers[i])] = peers[i];
	}

	Gtk::TreeNodeChildren children = model->children();
	for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
	{
		Gtk::TreeRow row = *iter;
		row[columns.remove] = true;
	}

	// sorting mess up iteration when we change the values in the sort column
	Gtk::SortType order;
	int col;
	model->get_sort_column_id(col, order);
	model->set_sort_column_id(Gtk::TreeSortable::DEFAULT_UNSORTED_COLUMN_ID, order);

	model->foreach_iter(sigc::bind(sigc::mem_fun(this, &PeerList::on_foreach), &peer_map));

	// erase disconnected peers (can't do that in on_foreach...)
	children = model->children();
	for (Gtk::TreeIter iter = children.begin(); iter != children.end(); )
	{
		Gtk::TreeRow row = *iter;
		if (row[columns.remove])
			iter = model->erase(iter);
		else
			iter++;
	}

	/* Add all new (remaining) peers */
	for (PeerMap::iterator iter = peer_map.begin(); iter != peer_map.end(); ++iter)
	{
		Gtk::TreeRow row = *(model->append());
		set_peer_details(row, iter->second);
	}

	model->set_sort_column_id(col, order);

	working = false;
}

void PeerList::set_peer_details(Gtk::TreeRow& row, const libtorrent::peer_info& peer)
{
	row[columns.address] = peer_as_string(peer);

	Glib::RefPtr<Gdk::Pixbuf> pixbuf = row[columns.flag];
	if (!pixbuf)
	{
		char c[3];
		c[0] = peer.country[0];
		c[1] = peer.country[1];
		c[2] = '\0';
		Glib::ustring country = c;
		if (!country.empty() && country != "00" && country != "--" && country != "!!")
		{
			Glib::ustring flag = Glib::build_filename(FLAG_DIR, country.lowercase() + ".png");
			try
			{
				row[columns.flag] = Gdk::Pixbuf::create_from_file(flag);
			}
			catch (...) {}
		}
	}

	pixbuf = row[columns.encryption];
	if (!pixbuf &&
		peer.flags & libtorrent::peer_info::rc4_encrypted ||
		peer.flags & libtorrent::peer_info::plaintext_encrypted)
	{
		pixbuf = render_icon(Gtk::Stock::DIALOG_AUTHENTICATION, Gtk::ICON_SIZE_MENU);
		row[columns.encryption] = pixbuf;
	}

	row[columns.down] = peer.total_download;
	row[columns.up] = peer.total_upload;
	row[columns.down_rate] = peer.payload_down_speed;
	row[columns.up_rate] = peer.payload_up_speed;
	if (!(peer.flags & libtorrent::peer_info::seed))
	{
		// this is a real time hog, if we have a lot of peers and many pieces =/
		unsigned int completed = 0;
		for (unsigned int j = 0; j < peer.pieces.size(); j++)
		{
			if (peer.pieces[j])
				completed++;
		}
		double progress = (double)completed/peer.pieces.size();
		row[columns.progress] = progress*100;
	}
	else
		row[columns.progress] = 100;

	Glib::ustring client = libtorrent::identify_client(peer.pid);
	/* Cosmetic fix, replace Micro with µ if found */
	if (client.find("Micro", 0) == 0)
		client.replace(0, 5, "\u00b5");
	row[columns.client] = client;

	std::stringstream ss;
	ss.imbue(std::locale(""));
	if (peer.flags & libtorrent::peer_info::connecting)
		ss << _("Connecting");
	else if (peer.flags & libtorrent::peer_info::handshake)
		ss << _("Handshake");
	else if	(peer.flags & libtorrent::peer_info::queued)
		ss << _("Queued");
	else
	{
		if (peer.flags & libtorrent::peer_info::interesting && peer.flags & libtorrent::peer_info::remote_interested)
		{
			if (ss.tellp())
				ss << ", ";
			ss << _("Both interested");
		}
		else
		{
			if (peer.flags & libtorrent::peer_info::interesting)
			{
				if (ss.tellp())
					ss << ", ";
				ss << _("Interested");
			}
			if (peer.flags & libtorrent::peer_info::remote_interested)
			{
				if (ss.tellp())
					ss << ", ";
				ss << _("Remote interested");
			}
		}

		if (peer.flags & libtorrent::peer_info::choked && peer.flags & libtorrent::peer_info::remote_choked)
		{
			if (ss.tellp())
				ss << ", ";
			ss << _("Both choked");
		}
		else
		{
			if (peer.flags & libtorrent::peer_info::choked)
			{
				if (ss.tellp())
					ss << ", ";
				ss << _("Choked");
			}
			if (peer.flags & libtorrent::peer_info::remote_choked)
			{
				if (ss.tellp())
					ss << ", ";
				ss << _("Remote choked");
			}
		}
	}
	row[columns.flags] = Glib::locale_to_utf8(ss.str());

	if (peer.flags & libtorrent::peer_info::tracker)
		row[columns.source] = _("Tracker");
	else if (peer.flags & libtorrent::peer_info::dht)
		row[columns.source] = _("DHT");
	else if (peer.flags & libtorrent::peer_info::pex)
		row[columns.source] = _("PEX");
	else if (peer.flags & libtorrent::peer_info::lsd)
		row[columns.source] = _("LSD");
	else if (peer.flags & libtorrent::peer_info::resume_data)
		row[columns.source] = _("Resume data");
}

Glib::ustring PeerList::peer_as_string(const libtorrent::peer_info& peer)
{
	return peer.ip.address().to_string() + ":" + String::compose("%1", peer.ip.port());
}

