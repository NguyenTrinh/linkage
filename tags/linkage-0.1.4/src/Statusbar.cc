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

#include <gtkmm/frame.h>
#include <gtkmm/box.h>
#include <gtkmm/image.h>
#include <gtkmm/stock.h>

#include "linkage/Utils.hh"

#include "Statusbar.hh"

Statusbar::Statusbar(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::Statusbar(cobject)
{
	Gtk::Frame* frame = manage(new Gtk::Frame());
	Gtk::HBox* box = manage(new Gtk::HBox());

	Gtk::Image* image = NULL;

	m_upload = manage(new Gtk::Label());
	box->pack_end(*m_upload, false, false);
	image = manage(new Gtk::Image(Gtk::Stock::GO_UP, Gtk::ICON_SIZE_MENU));
	box->pack_end(*image, false, false, 5);
	
	m_download = manage(new Gtk::Label());
	box->pack_end(*m_download, false, false);
	image = manage(new Gtk::Image(Gtk::Stock::GO_DOWN, Gtk::ICON_SIZE_MENU));
	box->pack_end(*image, false, false, 5);
	
	m_connections = manage(new Gtk::Label());
	box->pack_end(*m_connections, false, false);
	image = manage(new Gtk::Image(Gtk::Stock::NETWORK, Gtk::ICON_SIZE_MENU));
	box->pack_end(*image, false, false, 5);

	frame->add(*box);
	frame->set_shadow_type(Gtk::SHADOW_IN);
	pack_end(*frame, false, false);
}

Statusbar::~Statusbar()
{
}

void Statusbar::set_status(int connections, float down_rate, float up_rate)
{
	m_connections->set_text(str(connections));
	m_download->set_text(suffix_value(down_rate) + "/s");
	m_upload->set_text(suffix_value(up_rate) + "/s");
}

