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

#include "PieceMap.hh"

#include "linkage/Utils.hh"

PieceMap::PieceMap(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::DrawingArea(cobject)
{
}

void PieceMap::on_realize()
{
	Gtk::DrawingArea::on_realize();

	set_size_request(-1, 32);

	get_window()->clear();
}

bool PieceMap::on_expose_event(GdkEventExpose* event)
{
	Gdk::Color base = get_style()->get_bg(Gtk::STATE_SELECTED);
	Gdk::Color bg = get_style()->get_bg(Gtk::STATE_NORMAL);

	Glib::RefPtr<Gdk::Colormap> colormap = get_screen()->get_default_colormap();

	int fw = get_allocation().get_width();
	int fh = get_allocation().get_height();
	Glib::RefPtr<Gdk::Window> window = get_window();
	Gdk::Rectangle rect = Gdk::Rectangle(0, 0, fw, fh);
	Gtk::Widget* widget = dynamic_cast<Gtk::Widget*>(this);
	get_style()->paint_box(window, Gtk::STATE_NORMAL, Gtk::SHADOW_IN,
												 rect, *widget, "", 0, 0, fw, fh);

	int x = event->area.x;
	//int y = event->area.y;
	int w = event->area.width;
	//int h = event->area.height;

	std::list<Part> parts;
	double scale = 1.0;
	int mw = fw - (get_style()->get_xthickness() * 2);
	if ((int)m_map.size() >= mw)
		parts = more_pieces();
	else
	{
		parts = more_pixels();
		scale = (double)mw/m_map.size();
	}
		
	Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(window);
	
	int ph = fh - (get_style()->get_ythickness() * 2);
	for (std::list<Part>::iterator iter = parts.begin(); iter != parts.end(); ++iter)
	{
		Part& p = *iter;
		int pw = (int)(scale*(p.last - p.first + 1) + 0.5);
		int px = (int)(scale*p.first + 0.5) + get_style()->get_xthickness();

		/* Don't redraw unnecessary parts */
		if (px > (x + w))
			break;
		if ((px + pw) < x)
			continue;

		Gdk::Color c = Linkage::lighter(base, p.fac);

		colormap->alloc_color(c);
		gc->set_foreground(c);
		get_window()->draw_rectangle(gc, true, px, get_style()->get_ythickness(), pw, ph);
		colormap->free_color(c);
	}

	return true;
}

PieceMap::~PieceMap()
{
}

std::list<Part> PieceMap::more_pixels()
{
	std::list<Part> parts;
	int count = m_map.size();
	
	for (int i = 0; i < count; i++)
	{
		if (!m_map[i])
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

std::list<Part> PieceMap::more_pieces()
{
	std::list<Part> parts;
	int count = m_map.size();
	int width = get_allocation().get_width() - (get_style()->get_xthickness() * 2);
	double pieces_per_part = (double)count/width;
	
	for (int i = 0; i < width; i++)
	{
		int completed = 0;
		int start = (int)(i*pieces_per_part);
		int end = (int)((i+1)*pieces_per_part+0.5);
		for (int j = start; j < end; j++)
			if (m_map[j])
				completed++;

		if (!completed)
			continue;

		double fac = 2 - (double)completed / (end - start);

		if (parts.empty())
			parts.push_back(Part(i, i, fac));
		else
		{
			Part & l = parts.back();
			if (l.last == (int)(i - 1) && l.fac == fac)
				l.last = i;
			else
				parts.push_back(Part(i, i, fac));
		} 
	}
	return parts;
}

void PieceMap::set_map(const std::vector<bool>& map)
{
	m_map.assign(map.begin(), map.end());

	Glib::RefPtr<Gdk::Window> window = get_window();
	if (window)
	{
		Gdk::Rectangle r(0, 0, get_allocation().get_width(), get_allocation().get_height());
		window->invalidate_rect(r, false);
	}
}
