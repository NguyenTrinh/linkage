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

#include <list>
#include <vector>

#include "CellRendererPieceMap.hh"

#include "linkage/Utils.hh"

CellRendererPieceMap::CellRendererPieceMap() :
	Glib::ObjectBase(typeid(CellRendererPieceMap)),
	Gtk::CellRenderer(),
	m_prop_map(*this, "map")
{
	property_xpad() = 0;
	property_ypad() = 0;
}

CellRendererPieceMap::~CellRendererPieceMap()
{
}

std::list<Part> CellRendererPieceMap::more_pixels(int width, int height)
{
	std::list<Part> parts;
	int count = m_prop_map.get_value().size();

	for (int i = 0; i < count; i++)
	{
		if (!m_prop_map.get_value()[i])
			continue;

		if (parts.empty())
			parts.push_back(Part(i, i, 1));
		else
		{
			Part & l = parts.back();
			if (l.last == (int)(i - 1))
				l.last = i;
			else
				parts.push_back(Part(i, i, 1));
		}
	}

	return parts;
}

std::list<Part> CellRendererPieceMap::more_pieces(int width, int height)
{
	std::list<Part> parts;
	int count = m_prop_map.get_value().size();
	double pieces_per_part = (double)count/width;

	for (int i = 0; i < width; i++)
	{
		int completed = 0;
		int start = (int)(i*pieces_per_part);
		int end = (int)((i+1)*pieces_per_part+0.5);
		for (int j = start; j < end; j++)
			if (m_prop_map.get_value()[j])
				completed++;

		if (!completed)
			continue;

		double fac = 2 - (double)completed / (end - start);

		if (parts.empty())
			parts.push_back(Part(i, i, fac));
		else
		{
			Part& l = parts.back();
			if (l.last == (int)(i - 1) && l.fac == fac)
				l.last = i;
			else
				parts.push_back(Part(i, i, fac));
		}
	}

	return parts;
}

void CellRendererPieceMap::get_size_vfunc(Gtk::Widget&,
																					const Gdk::Rectangle* cell_area,
																					int* x_offset, int* y_offset,
																					int* width, int* height) const
{
	const int xpad = property_xpad();
	const int ypad = property_ypad();
	const int xalign = property_xalign();
	const int yalign = property_yalign();

	int w = 0, h = 0;
	if (cell_area)
	{
		w = cell_area->get_width();
		h = cell_area->get_height();
	}
	const int calc_width = w - xpad * 2;
	const int calc_height = h - ypad * 2;

	if(width)
		*width = calc_width;

	if(height)
		*height = calc_height;

	if(cell_area)
	{
		if(x_offset)
		{
			*x_offset = (int)(xalign * (cell_area->get_width() - calc_width));
			*x_offset = std::max(0, *x_offset);
		}

		if(y_offset)
		{
			*y_offset = (int)(yalign * (cell_area->get_height() - calc_height));
			*y_offset = std::max(0, *y_offset);
		}
	}
}

void CellRendererPieceMap::render_vfunc(const Glib::RefPtr<Gdk::Drawable>& window,
																				Gtk::Widget& widget,
																				const Gdk::Rectangle& background_area,
																				const Gdk::Rectangle& cell_area,
																				const Gdk::Rectangle& expose_area,
																				Gtk::CellRendererState flags)
{
	Gdk::Color base = widget.get_style()->get_bg(Gtk::STATE_SELECTED);
	Gdk::Color bg = widget.get_style()->get_bg(Gtk::STATE_NORMAL);

	Glib::RefPtr<Gdk::Colormap> colormap = widget.get_screen()->get_default_colormap();

	const int cell_xpad = property_xpad();
	const int cell_ypad = property_ypad();

	int x_offset = 0, y_offset = 0, width = 0, height = 0;
	get_size(widget, cell_area, x_offset, y_offset, width, height);

	width -= cell_xpad * 2;
	height -= cell_ypad * 2;

	if(width <= 0 || height <= 0)
		return;

	Glib::RefPtr<Gdk::Window> cwin = Glib::RefPtr<Gdk::Window>::cast_dynamic<>(window);
	Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(cwin);

	gc->set_foreground(widget.get_style()->get_fg(Gtk::STATE_NORMAL));
	cwin->draw_rectangle(gc, true,
											 cell_area.get_x() + x_offset + cell_xpad,
											 cell_area.get_y() + y_offset + cell_ypad,
											 width - 1, height - 1);

	int bar_x = 0, bar_y = 0, bar_width = 0, bar_height = 0;
	bar_x = cell_area.get_x() + x_offset + cell_xpad + widget.get_style()->get_xthickness();
	bar_y = cell_area.get_y() + y_offset + cell_ypad + widget.get_style()->get_ythickness();
	bar_width = width - 1 - (widget.get_style()->get_xthickness() * 2);
	bar_height = height - 1 - (widget.get_style()->get_ythickness() * 2);

	double scale = 1.0;
	std::list<Part> parts;
	int num = m_prop_map.get_value().size();
	if (num >= bar_width)
		parts = more_pieces(bar_width, bar_height);
	else
	{
		parts = more_pixels(bar_width, bar_height);
		scale = (double)bar_width/num;
	}

	gc->set_foreground(bg);
	cwin->draw_rectangle(gc, true, bar_x, bar_y, bar_width, bar_height);

	for (std::list<Part>::iterator iter = parts.begin(); iter != parts.end(); ++iter)
	{
		Part& p = *iter;
		int pw = (int)(scale*(p.last - p.first + 1));
		int px = (int)(scale*p.first+0.5) + bar_x;
		if (px >= (bar_x + bar_width))
			break;
		if (px < expose_area.get_x())
			continue;

		Gdk::Color c = Linkage::lighter(base, p.fac);

		colormap->alloc_color(c);
		gc->set_foreground(c);
		cwin->draw_rectangle(gc, true, px, bar_y, pw, bar_height);
	}
}

Glib::PropertyProxy<std::vector<bool> > CellRendererPieceMap::property_map()
{
	return m_prop_map.get_proxy();
}
