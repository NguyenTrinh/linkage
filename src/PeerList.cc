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
#include <gtkmm/treeselection.h>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/cellrendererprogress.h>

#include "libtorrent/identify_client.hpp"

#include "PeerList.hh"
#include "linkage/Utils.hh"

PeerList::PeerList(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::TreeView(cobject)
{
	model = Gtk::ListStore::create(columns);

	set_model(model);

	Gtk::TreeViewColumn* column = Gtk::manage(new Gtk::TreeViewColumn("Address"));
	column->set_sort_column_id(columns.address);
	column->set_resizable(true);
	column->pack_start(columns.flag, false);
	column->pack_start(columns.address);
	append_column(*column);

	int col = append_column("Down", columns.down);
	column = get_column(col - 1);
	Gtk::CellRendererText* cell = dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell_renderer());
	column->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(this, &PeerList::format_data), columns.down));
	col = append_column("Up", columns.up);
	column = get_column(col - 1);
	cell = dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell_renderer());
	column->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(this, &PeerList::format_data), columns.up));
	col = append_column("Down rate", columns.down_rate);
	column = get_column(col - 1);
	cell = dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell_renderer());
	column->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(this, &PeerList::format_rates), columns.down_rate));
	col = append_column("Up rate", columns.up_rate);
	column = get_column(col - 1);
	cell = dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell_renderer());
	column->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(this, &PeerList::format_rates), columns.up_rate));

	Gtk::CellRendererProgress* prender = new Gtk::CellRendererProgress();
	col = append_column("Progress", *Gtk::manage(prender));
	get_column(col - 1)->add_attribute(*prender, "value", col);
	append_column("Client", columns.client);
	append_column("State", columns.flags);

	for(unsigned int i = 1; i < 8; i++)
	{
		column = get_column(i);
		column->set_sort_column_id(i + 1);
		column->set_resizable(true);
	}

	m_state = STATE_IDLE;
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

void PeerList::update(const WeakPtr<Torrent>& torrent)
{
	/* FIXME: thread loop, tell old thread to exit and do cond/wait */
	if (m_state == STATE_BUSY)
		return;
	else
		m_state = STATE_BUSY;

	std::vector<libtorrent::peer_info> peers;
	if (!torrent->is_stopped())
		torrent->get_handle().get_peer_info(peers);

	if (peers.empty()) {
		model->clear();
		m_state = STATE_IDLE;
		return;
	}

	PeerMap peer_map;
	for (unsigned int i = 0; i < peers.size(); i++)
	{
		peer_map[peers[i].pid] = peers[i];
	}

	Gtk::TreeNodeChildren children = model->children();
	for (Gtk::TreeIter iter = children.begin(); iter != children.end();)
	{
		libtorrent::peer_info peer;
		Gtk::TreeRow row = *iter;

		PeerMap::iterator peer_iter = peer_map.find(row[columns.id]);
		if (peer_iter == peer_map.end())
		{
			iter = model->erase(iter);
			continue;
		}
		else
		{
			peer = peer_iter->second;
			peer_map.erase(peer_iter);

			iter++;
		}

		/* Give the UI some love, since this loop is pretty slow with many peers */
		while (Gtk::Main::events_pending())
			Gtk::Main::iteration(false);

		set_peer_details(row, peer);
	}

	/* Add all new (remaining) peers */
	for (PeerMap::iterator iter = peer_map.begin(); iter != peer_map.end(); ++iter)
	{
		Gtk::TreeRow row = *(model->append());
		set_peer_details(row, iter->second);
	}

	m_state = STATE_IDLE;
}

void PeerList::set_peer_details(Gtk::TreeRow& row, const libtorrent::peer_info& peer)
{
	row[columns.id] = peer.pid;

	Glib::ustring address = peer.ip.address().to_string();
	row[columns.address] = address + ":" + str(peer.ip.port());

	if (!row[columns.has_flag])
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
				row[columns.has_flag] = true;
			}
			catch (Glib::Error& e)
			{
				g_warning(e.what().c_str());
			}
		}
	}

	row[columns.down] = peer.total_download;
	row[columns.up] = peer.total_upload;
	row[columns.down_rate] = peer.payload_down_speed;
	row[columns.up_rate] = peer.payload_up_speed;
	if (!peer.seed)
	{
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

	std::stringstream flags;
	if (peer.flags & libtorrent::peer_info::connecting)
		flags << "Connecting";
	else if (peer.flags & libtorrent::peer_info::handshake)
		flags << "Handshake";
	else if	(peer.flags & libtorrent::peer_info::queued)
		flags << "Queued";
	else
	{
		if (peer.flags & libtorrent::peer_info::interesting)
		{
			if (flags.tellp())
				flags << ", ";
			flags << "Interested";
		}
		if (peer.flags & libtorrent::peer_info::choked)
		{
			if (flags.tellp())
				flags << ", ";
			flags << "Choked";
		}
		if (peer.flags & libtorrent::peer_info::remote_interested)
		{
			if (flags.tellp())
				flags << ", ";
			flags << "Remote interested";
		}
		if (peer.flags & libtorrent::peer_info::remote_choked)
		{
			if (flags.tellp())
				flags << ", ";
			flags << "Remote choked";
		}
	}
	row[columns.flags] = flags.str();
}
