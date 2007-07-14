/*
Copyright (C) 2005 by
Joris Guisson <joris.guisson@gmail.com>
Vincent Wagelaar <vincent@ricardis.tudelft.nl>

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

#ifndef PIECEMAP_HH
#define PIECEMAP_HH

#include <list>
#include <vector>

#include <gtkmm/drawingarea.h>
#include <gdkmm/window.h>
#include <libglademm.h>

struct Part
{
	int first;
	int last;
	double fac;
	
	Part(int fi, int la, double fa) : first(fi), last(la), fac(fa) {}
};

class PieceMap : public Gtk::DrawingArea
{
	std::vector<bool> m_map;

	std::list<Part> more_pieces();
	std::list<Part> more_pixels();
	
	void on_realize();
	bool on_expose_event(GdkEventExpose* event);

public:
	void set_map(const std::vector<bool>& map);

	PieceMap(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	~PieceMap();
};
	
#endif 
