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

#include <gtkmm/treeselection.h>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/cellrendererprogress.h>

#include "libtorrent/identify_client.hpp"

#include "PeerList.hh"
#include "linkage/Utils.hh"

PeerList::PeerList()
{
	model = Gtk::ListStore::create(columns);

	set_model(model);

	#ifdef HAVE_LIBGEOIP
		Gtk::TreeView::Column* pColumn = Gtk::manage(new Gtk::TreeView::Column("Address"));

		pColumn->pack_start(columns.flag, false);
		pColumn->pack_start(columns.address);

		pColumn->set_sort_column_id(columns.address);
		pColumn->set_resizable(true);

		append_column(*pColumn);
	#else
		append_column("Address", columns.address);
		Gtk::TreeView::Column* pColumn = get_column(0);
		pColumn->set_sort_column_id(0);
		pColumn->set_resizable(true);
	#endif

	append_column("Down", columns.down);
	append_column("Up", columns.up);
	append_column("Down rate", columns.down_rate);
	append_column("Up rate", columns.up_rate);
	Gtk::CellRendererProgress* prender = new Gtk::CellRendererProgress();
	unsigned int col = append_column("Progress", *Gtk::manage(prender));
	#ifndef HAVE_LIBGEOIP /* FIXME: Not tested! */
		col -= 1;
	#endif
	get_column(5)->add_attribute(*prender, "value", col);
	append_column("Client", columns.client);

	for(unsigned int i = 1; i < 7; i++)
	{
		Gtk::TreeView::Column* column = get_column(i);
		#ifdef HAVE_LIBGEOIP
			column->set_sort_column_id(i + 1);
		#else
			column->set_sort_column_id(i);
		#endif
		column->set_resizable(true);
	}
}

PeerList::~PeerList()
{
}

void PeerList::clear()
{
	model->clear();
}

void PeerList::update(const WeakPtr<Torrent>& torrent)
{
	std::vector<peer_info> peers;
	torrent->get_handle().get_peer_info(peers);

	Glib::ustring sel_addr;
	bool select = false;
	Gtk::TreeModel::iterator iter = get_selection()->get_selected();
	if (iter)
	{
		Gtk::TreeModel::Row row = *iter;
		sel_addr = row[columns.address];
		select = true;
	}
	
	set_model(Glib::RefPtr<Gtk::TreeModel>(NULL));
	
	clear();

	#ifdef HAVE_LIBGEOIP
		GeoIP* gi = GeoIP_new(GEOIP_STANDARD);
	#endif

	for (unsigned int i = 0; i < peers.size(); i++)
	{
		if (peers[i].flags & (peer_info::handshake | peer_info::connecting | peer_info::queued))
			continue;

		Gtk::TreeModel::Row row = *(model->append());

		#ifdef HAVE_LIBGEOIP
			Glib::ustring addr = peers[i].ip.address().to_string();
			const char *country_code = 0;
			country_code = GeoIP_country_code_by_addr(gi, addr.c_str());
			if (country_code != NULL)
			{
				Glib::ustring flag = Glib::build_filename(FLAG_DIR, Glib::ustring(country_code).lowercase() + ".png");
				try
				{
					row[columns.flag] = Gdk::Pixbuf::create_from_file(flag);
			 	}
			 	catch (Glib::Error& e) {}
			}
		#endif

		row[columns.address] = peers[i].ip.address().to_string() + ":" + str(peers[i].ip.port());
		row[columns.down] = suffix_value((unsigned int)peers[i].total_download);
		row[columns.up] = suffix_value((unsigned int)peers[i].total_upload);
		row[columns.down_rate] = suffix_value(peers[i].payload_down_speed) + "/s";
		row[columns.up_rate] = suffix_value(peers[i].payload_up_speed) + "/s";
		unsigned int completed = 0;
		for (unsigned int j = 0; j < peers[i].pieces.size(); j++)
		{
			if (peers[i].pieces[j])
				completed++;
		}
		double progress = (double)completed/peers[i].pieces.size();
		row[columns.progress] = progress*100;

		Glib::ustring client = identify_client(peers[i].pid);
		/* Cosmetic fix, replace Micro with µ if found */
		if (client.find("Micro", 0) == 0)
			client.replace(0, 5, "\u00b5");
		row[columns.client] = client;
	}

	#ifdef HAVE_LIBGEOIP
		GeoIP_delete(gi);
	#endif
	
	set_model(model);
	
	if (select)
	{
		for (unsigned int i = 0; i < model->children().size(); i++)
		{
			Gtk::TreeModel::Row row = model->children()[i];
			if (row[columns.address] == sel_addr)
			{
				get_selection()->select(row);
				break;
			}
		}
	}
}
