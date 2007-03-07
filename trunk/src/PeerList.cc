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

#include "config.h"

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

	Gtk::TreeViewColumn* column = Gtk::manage(new Gtk::TreeViewColumn("Address"));
	column->set_sort_column_id(columns.address);
	column->set_resizable(true);
	#ifdef HAVE_LIBGEOIP
		column->pack_start(columns.flag, false);
		column->pack_start(columns.address);
	#else
		column->pack_start(columns.address);
	#endif
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

	for(unsigned int i = 1; i < 7; i++)
	{
		column = get_column(i);
		column->set_sort_column_id(i + 1);
		column->set_resizable(true);
	}
}

PeerList::~PeerList()
{
}

void PeerList::format_data(Gtk::CellRenderer* cell,
									const Gtk::TreeIter& iter,
									const Gtk::TreeModelColumn<size_type>& column)
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

void PeerList::clear()
{
	model->clear();
}

void PeerList::update(const WeakPtr<Torrent>& torrent)
{
	std::vector<peer_info> peers;
	if (torrent->is_running())
		torrent->get_handle().get_peer_info(peers);

	Glib::ustring sel_addr;
	Gtk::TreeIter iter = get_selection()->get_selected();
	if (iter)
	{
		Gtk::TreeRow row = *iter;
		sel_addr = row[columns.address];
	}

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
		if (row[columns.address] == sel_addr)
			get_selection()->select(row);
		row[columns.down] = peers[i].total_download;
		row[columns.up] = peers[i].total_upload;
		row[columns.down_rate] = peers[i].payload_down_speed;
		row[columns.up_rate] = peers[i].payload_up_speed;
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
}
