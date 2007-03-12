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

#ifndef STATUSBAR_HH
#define STATUSBAR_HH

#include <gtkmm/statusbar.h>
#include <gtkmm/label.h>

class Statusbar : public Gtk::Statusbar
{
	Gtk::Label* m_connections;
	Gtk::Label* m_download;
	Gtk::Label* m_upload;

public:
	void set_status(int connections, float down_rate, float up_rate);
	
	Statusbar();
	~Statusbar();
};

#endif /* STATUSBAR_HH */
