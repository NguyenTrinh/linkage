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

#include "CellRendererProgressText.hh"
#include <iostream>

CellRendererProgressText::CellRendererProgressText() :
	Glib::ObjectBase(typeid(CellRendererProgressText)),
	Gtk::CellRendererProgress(),
	m_prop_text1(*this, "text1"),
	m_prop_text2(*this, "text2")
{
	property_xpad() = 0;
	property_ypad() = 0;
}

CellRendererProgressText::~CellRendererProgressText()
{
}

void CellRendererProgressText::render_vfunc(const Glib::RefPtr<Gdk::Drawable>& window,
																				Gtk::Widget& widget,
																				const Gdk::Rectangle& bg_area,
																				const Gdk::Rectangle& cell_area,
																				const Gdk::Rectangle& exp_area,
																				Gtk::CellRendererState flags)
{
	static int min_size = -1;
	
	const int xpad = property_xpad();
	const int ypad = property_ypad();
	
	int x_offset = 0, y_offset = 0, width = 0, height = 0;
	get_size(widget, cell_area, x_offset, y_offset, width, height);

	width -= xpad * 2;
	height -= ypad * 2;
	
	int bar_x = 0, bar_y = 0, bar_width = 0, bar_height = 0;
	bar_x = cell_area.get_x() + x_offset + xpad + widget.get_style()->get_xthickness();
	bar_y = cell_area.get_y() + y_offset + ypad + widget.get_style()->get_ythickness();
	bar_width = width - (widget.get_style()->get_xthickness() * 2);
	bar_height = height - (widget.get_style()->get_ythickness() * 2);
	
	/* If our cell area spans two rows we should draw text as well
		 instead of messing with DPI and 
		 widget.get_pango_context()->get_font_description().get_size()
		 we do an ugly hack with static min_size */
	if (bar_height < min_size || min_size < 0)
		min_size = bar_height;
	
	bool draw_text = (bar_height > min_size);
	if (draw_text)
	{
		bar_height = min_size;
	}
	
	Gdk::Rectangle bar_area(bar_x, bar_y, bar_width, bar_height);

	Gtk::CellRendererProgress::render_vfunc(window, widget, bg_area, bar_area, exp_area, flags);
	
	if(width <= 0 || height <= 0)
		return;

	Gtk::StateType state;
	if (flags & Gtk::CELL_RENDERER_SELECTED)
		state = Gtk::STATE_SELECTED;
	else
		state = Gtk::STATE_NORMAL;

	Glib::RefPtr<Gdk::Window> gdkwin = Glib::RefPtr<Gdk::Window>::cast_dynamic<>(window);
	Gdk::Rectangle text_area(bar_x + xpad,
														bar_y + bar_height + ypad,
														bar_width - xpad,
														height - bar_height - xpad);
	Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(widget.get_pango_context());
	layout->set_markup("DL: " + m_prop_text1);
	widget.get_style()->paint_layout(gdkwin, state, true, text_area,
																		widget,
                        						"CellRendererProgressText_property_text1",
                        						text_area.get_x() + x_offset + xpad,
                        						text_area.get_y() + y_offset + ypad,
                        						layout);
   layout->set_markup("UL: " + m_prop_text2);
   int text2_width, text2_height;
   layout->get_pixel_size(text2_width, text2_height);
   widget.get_style()->paint_layout(gdkwin, state, true, text_area,
																		widget,
                        						"CellRendererProgressText_property_text2",
                        						text_area.get_x() + text_area.get_width() - xpad - text2_width,
                        						text_area.get_y() + y_offset + ypad,
                        						layout);
}

void CellRendererProgressText::get_size_vfunc(Gtk::Widget& widget,
																					const Gdk::Rectangle* cell_area,
																					int* x_offset, int* y_offset,
																					int* width, int* height) const
{
	int w = 0, h = 0;
	if (cell_area)
	{
		w = cell_area->get_width();
		h = cell_area->get_height();
	}
	const int calc_width = w - property_xpad() * 2;
	const int calc_height = h - property_ypad() * 2;
	
	Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(widget.get_pango_context());
	layout->set_markup("DL:UL:     " + m_prop_text1 + m_prop_text2);
	int min_width, min_height;
  layout->get_pixel_size(min_width, min_height);
	min_width += 5 - property_xpad()*2;
	min_height -= property_ypad() * 2;
	
	if (width)
		*width = std::max(calc_width, min_width);

	if (height)
		*height = std::max(calc_height, min_height);

	if (cell_area)
	{
		if (x_offset)
		{
			*x_offset = (int)(property_xalign() * (cell_area->get_width() - calc_width));
			*x_offset = std::max(0, *x_offset);
		}

		if (y_offset)
		{
			*y_offset = (int)(property_yalign() * (cell_area->get_height() - calc_height));
			*y_offset = std::max(0, *y_offset);
		}
	}
}

Glib::PropertyProxy<Glib::ustring> CellRendererProgressText::property_text1()
{
	return m_prop_text1.get_proxy();
}

Glib::PropertyProxy<Glib::ustring> CellRendererProgressText::property_text2()
{
	return m_prop_text2.get_proxy();
}
