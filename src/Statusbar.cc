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

#include "linkage/Utils.hh"

#include "Statusbar.hh"

Statusbar::Statusbar(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::Statusbar(cobject)
{
	Gtk::Frame* frame = manage(new Gtk::Frame());
	Gtk::HBox* box = manage(new Gtk::HBox());

	m_upload = manage(new Gtk::Label());
	box->pack_end(*m_upload, false, false);
	box->pack_end(*manage(new Gtk::Label("Upload rate:")), false, false, 10);
	
	m_download = manage(new Gtk::Label());
	box->pack_end(*m_download, false, false);
	box->pack_end(*manage(new Gtk::Label("Download rate:")), false, false, 10);
	
	m_connections = manage(new Gtk::Label());
	box->pack_end(*m_connections, false, false);
	box->pack_end(*manage(new Gtk::Label("Connections:")), false, false, 10);

	frame->add(*box);
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
