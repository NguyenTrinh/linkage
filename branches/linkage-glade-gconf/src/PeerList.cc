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
#include <iostream>
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
	if (!torrent->is_stopped())
		torrent->get_handle().get_peer_info(peers);

	if (peers.empty()) {
		clear();
		return;
	}
		
	/* Sorting messes up iteration */
	int id;
	Gtk::SortType order;
	model->get_sort_column_id(id, order);
	model->set_sort_column(columns.client, Gtk::SORT_ASCENDING);

	Gtk::TreeNodeChildren children = model->children();
	if (children.size() != peers.size())
	{
		model->clear();
		for (int i = 0; i < peers.size(); i++)
		{
			Gtk::TreeRow row = *(model->append());
			row[columns.id] = peers[i].pid;
			row[columns.has_flag] = false;
			Glib::ustring client = identify_client(peers[i].pid);
			/* Cosmetic fix, replace Micro with µ if found */
			if (client.find("Micro", 0) == 0)
				client.replace(0, 5, "\u00b5");
			row[columns.client] = client;
		}
	}

	for (Gtk::TreeIter iter = children.begin(); iter != children.end(); )
	{
		Gtk::TreeRow row = *iter;

		peer_info peer;
		bool keep = false;
		for (int i = 0; i < peers.size(); i++)
		{
			if (peers[i].pid == row[columns.id])
			{
				peer = peers[i];
				peers.erase(peers.begin() + i);
				keep = true;
				break;
			}
		}
		
		if (!keep)
		{
			iter = model->erase(iter);
			continue;
		}
		else
			 ++iter;

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
		 		try
				{
					row[columns.flag] = Gdk::Pixbuf::create_from_file(Glib::build_filename(FLAG_DIR, "unknown.png"));
			 	}
			 	catch (Glib::Error& e) {}
		 	}
		}
		else if (!row[columns.has_flag])
		{
			try
			{
				row[columns.flag] = Gdk::Pixbuf::create_from_file(Glib::build_filename(FLAG_DIR, "unknown.png"));
		 	}
		 	catch (Glib::Error& e) {}
		}

		Glib::ustring address = peer.ip.address().to_string();
		row[columns.address] = address + ":" + str(peer.ip.port());
		row[columns.down] = peer.total_download;
		row[columns.up] = peer.total_upload;
		row[columns.down_rate] = peer.payload_down_speed;
		row[columns.up_rate] = peer.payload_up_speed;
		unsigned int completed = 0;
		for (unsigned int j = 0; j < peer.pieces.size(); j++)
		{
			if (peer.pieces[j])
				completed++;
		}
		double progress = (double)completed/peer.pieces.size();
		row[columns.progress] = progress*100;

		Glib::ustring flags;
		if (peer.flags & peer_info::connecting)
			flags = "Connecting";
		else if (peer.flags & peer_info::handshake)
			flags = "Handshake";
		else if	(peer.flags & peer_info::queued)
			flags = "Queued";
		else
		{
			if (peer.flags & peer_info::interesting)
			{
				if (!flags.empty())
					flags += ", ";
				flags += "Interested";
			}
			if (peer.flags & peer_info::choked)
			{
				if (!flags.empty())
					flags += ", ";
				flags += "Choked";
			}
			if (peer.flags & peer_info::remote_interested)
			{
				if (!flags.empty())
					flags += ", ";
				flags += "Remote interested";
			}
			if (peer.flags & peer_info::remote_choked)
			{
				if (!flags.empty())
					flags += ", ";
				flags += "Remote choked";
			}
		}
		row[columns.flags] = flags;
	}
	model->set_sort_column(id, order);
}
