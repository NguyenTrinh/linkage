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

CellRendererProgressText::CellRendererProgressText() :
	Glib::ObjectBase(typeid(CellRendererProgressText)),
	Gtk::CellRendererProgress(),
	m_prop_text_left(*this, "text-left"),
	m_prop_text_right(*this, "text-right")
{
	property_xpad() = 0;
	property_ypad() = 0;
}

CellRendererProgressText::~CellRendererProgressText()
{
}

void CellRendererProgressText::render_vfunc(
	const Glib::RefPtr<Gdk::Drawable>& window,
	Gtk::Widget& widget,
	const Gdk::Rectangle& bg_area,
	const Gdk::Rectangle& cell_area,
	const Gdk::Rectangle& exp_area,
	Gtk::CellRendererState flags)
{
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
	bar_height = (height - (widget.get_style()->get_ythickness() * 2))/2 + 2;
	
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

	Glib::RefPtr<Gdk::Pixbuf> pixbuf;
	pixbuf = widget.render_icon(Gtk::Stock::GO_DOWN, Gtk::ICON_SIZE_MENU);
	int pixbuf_x = text_area.get_x();
	int pixbuf_y = text_area.get_y();
	int pixbuf_width = pixbuf->get_width();
	int pixbuf_height = pixbuf->get_height();
	pixbuf->render_to_drawable_alpha(window, 0, 0, pixbuf_x, pixbuf_y,
		-1, -1, Gdk::PIXBUF_ALPHA_FULL, 0, Gdk::RGB_DITHER_NONE, 0, 0);


	Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create(widget.get_pango_context());
	layout->set_markup(m_prop_text_left);
	widget.get_style()->paint_layout(gdkwin, state, true, text_area,
		widget, "CellRendererProgressText_property_text_left",
		text_area.get_x() + x_offset + xpad + (pixbuf_width),
		text_area.get_y() + y_offset + ypad,
		layout);

	layout->set_markup(m_prop_text_right);
	int text_right_width, text_right_height;
	layout->get_pixel_size(text_right_width, text_right_height);

	pixbuf = widget.render_icon(Gtk::Stock::GO_UP, Gtk::ICON_SIZE_MENU);
	pixbuf_width = pixbuf->get_width();
	pixbuf_height = pixbuf->get_height();
	pixbuf_x = text_area.get_x() + text_area.get_width() - xpad
		- text_right_width - pixbuf_width;
	pixbuf_y = text_area.get_y();
	pixbuf->render_to_drawable_alpha(window, 0, 0, pixbuf_x, pixbuf_y,
		-1, -1, Gdk::PIXBUF_ALPHA_FULL, 0, Gdk::RGB_DITHER_NONE, 0, 0);

	widget.get_style()->paint_layout(gdkwin, state, true, text_area,
		widget, "CellRendererProgressText_property_text_right",
		text_area.get_x() + text_area.get_width() - xpad - text_right_width,
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
	layout->set_markup(m_prop_text_left + m_prop_text_right);
	int row2_width, row2_height;
	layout->get_pixel_size(row2_width, row2_height);
	row2_height = row2_height*2 - property_ypad() * 2;
	row2_width += 10 + row2_height - property_xpad() * 2;

	layout->set_text(property_text()); //progressbar text might be wider..
	int row1_width, row1_height;
	layout->get_pixel_size(row1_width, row1_height);
	row1_height -= property_ypad() * 2;
	row1_width -= property_xpad() * 2;

	int min_width = std::max(row1_width, row2_width);
	int min_height = std::max(row1_height, row2_height);

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

Glib::PropertyProxy<Glib::ustring> CellRendererProgressText::property_text_left()
{
	return m_prop_text_left.get_proxy();
}

Glib::PropertyProxy<Glib::ustring> CellRendererProgressText::property_text_right()
{
	return m_prop_text_right.get_proxy();
}
