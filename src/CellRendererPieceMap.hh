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

#ifndef CELLRENDERER_PIECEMAP_HH
#define CELLRENDERER_PIECEMAP_HH

#include <gtkmm/cellrenderer.h>

#include "PieceMap.hh"

class CellRendererPieceMap : public Gtk::CellRenderer
{
	Glib::Property<std::vector<bool> > m_prop_map;

	std::list<Part> more_pieces(int width, int height);
	std::list<Part> more_pixels(int width, int height);
	
	void render_vfunc(const Glib::RefPtr<Gdk::Drawable>& window,
										Gtk::Widget& widget,
										const Gdk::Rectangle&,
										const Gdk::Rectangle& cell_area,
										const Gdk::Rectangle&,
										Gtk::CellRendererState flags);

	void get_size_vfunc(Gtk::Widget& widget,
											const Gdk::Rectangle* cell_area,
											int* x_offset, int* y_offset,
											int* width,		int* height) const;															
public:
	Glib::PropertyProxy<std::vector<bool> > property_map();

	CellRendererPieceMap();
	~CellRendererPieceMap();
};
	
#endif 
