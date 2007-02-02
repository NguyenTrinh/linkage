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

#include <list>
#include <vector>

#include "PieceMap.hh"

/* FIXME: Clean this class up! */

PieceMap::PieceMap(Gdk::Color& dark, Gdk::Color& mid, Gdk::Color& light)
{
	/*Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
	
	m_dark = Gdk::Color("blue");
	m_mid.set_red(32767);
	m_mid.set_green(32767);
	m_mid.set_blue(65535);
	m_light.set_red(52428);
	m_light.set_green(52428);
	m_light.set_blue(65535);
	
	colormap->alloc_color(m_dark);
	colormap->alloc_color(m_mid);
	colormap->alloc_color(m_light);*/
	
	m_dark = dark;
	m_mid = mid;
	m_light = light;
}

void PieceMap::on_realize()
{
	Gtk::DrawingArea::on_realize();
	
	get_window()->clear();
}

bool PieceMap::on_expose_event(GdkEventExpose*)
{
	if (!m_map.size())
		m_map.push_back(false);
		
	draw();
	
	return true;
}

PieceMap::~PieceMap()
{
}

std::list<Part> PieceMap::draw_more_pixels()
{
	std::list<Part> parts;
	unsigned int count = m_map.size();
	unsigned int width = get_allocation().get_width() - (get_style()->get_xthickness() * 2);
	unsigned int height = get_allocation().get_height() - (get_style()->get_ythickness() * 2);
	
	for (unsigned int i = 0; i < count; i++)
	{
		if (!m_map[i])
			continue;
			
		if (parts.empty())
			parts.push_back(Part(i, i, 100));
		else
		{
			Part & l = parts.back();
			if (l.last == (unsigned int)(i - 1))
				l.last = i;
			else
				parts.push_back(Part(i, i, 100));
		} 
	}
	return parts;
}

std::list<Part> PieceMap::draw_more_pieces()
{
	std::list<Part> parts;
	unsigned int count = m_map.size();
	unsigned int width = get_allocation().get_width() - (get_style()->get_xthickness() * 2);
	unsigned int height = get_allocation().get_height() - (get_style()->get_ythickness() * 2);
	double pieces_per_part = (double)count/width;
	
	for (unsigned int i = 0; i < width; i++)
	{
		unsigned int completed = 0;
		unsigned int start = (unsigned int)(i*pieces_per_part);
		unsigned int end = (unsigned int)((i+1)*pieces_per_part+0.5);
		for (unsigned int j = start; j < end; j++)
			if (m_map[j])
				completed++;
				
		if (!completed)
			continue;
			
		unsigned int fac = (unsigned int)(100*((double)completed / (end - start)) + 0.5);

		if (parts.empty())
			parts.push_back(Part(i, i, fac));
		else
		{
			Part & l = parts.back();
			if (l.last == (unsigned int)(i - 1) && l.fac == fac)
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
	
	draw();
}

void PieceMap::draw()
{
	set_size_request(-1, 32);
	if (!is_drawable())
		return;
	
	unsigned int w = get_allocation().get_width();
	unsigned int h = get_allocation().get_height();
	Glib::RefPtr<Gdk::Window> paint_win = get_window();
	Gdk::Rectangle rect = Gdk::Rectangle(0, 0, w, h);
	Gtk::Widget* widget = dynamic_cast<Gtk::Widget*>(this);
	get_style()->paint_box(paint_win, Gtk::STATE_NORMAL, Gtk::SHADOW_IN,
												 rect, *widget, "", 0, 0, w, h);
	
	std::list<Part> parts;
	double scale = 1.0;
	if (m_map.size() >= w - (get_style()->get_xthickness() * 2))
		parts = draw_more_pieces();
	else
	{
		parts = draw_more_pixels();
		scale = (double)(w - (get_style()->get_xthickness() * 2))/m_map.size();
	}
		
	Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(get_window());
	
	unsigned int rh = h - (get_style()->get_ythickness() * 2);
	for (std::list<Part>::iterator i = parts.begin();i != parts.end();++i)
	{
		Part & pa = *i;
		unsigned int rw = (unsigned int)(scale*(pa.last - pa.first + 1));
		unsigned int fac = pa.fac;
		unsigned int x = (unsigned int)(scale*pa.first) + get_style()->get_xthickness();

		if (fac >= 100)	
			gc->set_foreground(m_dark);
		else if (fac >= 50)
			gc->set_foreground(m_mid);
		else
			gc->set_foreground(m_light);
			
		get_window()->draw_rectangle(gc, true, x, get_style()->get_ythickness(), rw, rh);
	}
}

Part::Part(unsigned int fi, unsigned int la, unsigned int fa) : 
	first(fi), last(la), fac(fa)
{
}
