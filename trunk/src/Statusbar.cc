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

#include <Statusbar.hh>

Statusbar::Statusbar()
{
	/* FIXME: for some reason these labels are drawn above the statusbar border */
	m_upload = manage(new Gtk::Label());
	pack_end(*m_upload, false, false);
	pack_end(*manage(new Gtk::Label("Upload rate:")), false, false, 5);
	
	m_download = manage(new Gtk::Label());
	pack_end(*m_download, false, false);
	pack_end(*manage(new Gtk::Label("Download rate:")), false, false, 5);
	
	m_connections = manage(new Gtk::Label());
	pack_end(*m_connections, false, false);
	pack_end(*manage(new Gtk::Label("Connections:")), false, false, 5);
}

Statusbar::~Statusbar()
{
}

void Statusbar::set_connections_label(const Glib::ustring& text)
{
	m_connections->set_text(text);
}

void Statusbar::set_upload_label(const Glib::ustring& text)
{
	m_upload->set_text(text);
}

void Statusbar::set_download_label(const Glib::ustring& text)
{
	m_download->set_text(text);
}

