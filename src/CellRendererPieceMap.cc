/*
Copyright (C) 2006  Christian Lundgren

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <list>
#include <vector>

#include "CellRendererPieceMap.hh"

CellRendererPieceMap::CellRendererPieceMap(Gdk::Color& dark, 
                                           Gdk::Color& mid, 
                                           Gdk::Color& light) :
  Glib::ObjectBase(typeid(CellRendererPieceMap)),
  Gtk::CellRenderer(),
  property_map_(*this, "map")
{
  property_xpad() = 0;
  property_ypad() = 0;
  
  /*Glib::RefPtr<Gdk::Colormap> colormap = screen->get_default_colormap();
  
  dark_ = Gdk::Color("blue");
  mid_.set_red(32767);
  mid_.set_green(32767);
  mid_.set_blue(65535);
  light_.set_red(52428);
  light_.set_green(52428);
  light_.set_blue(65535);
  
  colormap->alloc_color(dark_);
  colormap->alloc_color(mid_);
  colormap->alloc_color(light_);*/
  
  dark_ = dark;
  mid_ = mid;
  light_ = light;
}

CellRendererPieceMap::~CellRendererPieceMap()
{
}

std::list<Part> 
CellRendererPieceMap::draw_more_pixels(const std::list<bool>& map,
                                       int width, 
                                       int height)
{
  std::list<Part> parts;
  std::vector<bool> vmap(map.begin(), map.end());
  int count = vmap.size();
  
  for (int i = 0; i < count; i++)
  {
    if (!vmap[i])
      continue;
      
    if (parts.empty())
      parts.push_back(Part(i, i, 100));
    else
    {
      Part & l = parts.back();
      if (l.last == int(i - 1))
        l.last = i;
      else
        parts.push_back(Part(i, i, 100));
    } 
  }
  
  return parts;
}

std::list<Part> 
CellRendererPieceMap::draw_more_pieces(const std::list<bool>& map,
                                       int width, 
                                       int height)
{
  std::list<Part> parts;
  std::vector<bool> vmap(map.begin(), map.end());
  int count = vmap.size();
  double pieces_per_part = (double)count/width;
  
  for (int i = 0; i < width; i++)
  {
    int completed = 0;
    int start = (int)(i*pieces_per_part);
    int end = (int)((i+1)*pieces_per_part+0.5);
    for (int j = start; j < end; j++)
      if (vmap[j])
        completed++;
        
    if (!completed)
      continue;
      
    int fac = int(100*((double)completed / (end - start)) + 0.5);

    if (parts.empty())
      parts.push_back(Part(i, i, fac));
    else
    {
      Part & l = parts.back();
      if (l.last == int(i - 1) && l.fac == fac)
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
                                          int* width,    int* height) const
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
      *x_offset = int(xalign * (cell_area->get_width() - calc_width));
      *x_offset = std::max(0, *x_offset);
    }

    if(y_offset)
    {
      *y_offset = int(yalign * (cell_area->get_height() - calc_height));
      *y_offset = std::max(0, *y_offset);
    }
  }
}

void CellRendererPieceMap::render_vfunc(const Glib::RefPtr<Gdk::Drawable>& window,
                                        Gtk::Widget& widget,
                                        const Gdk::Rectangle&,
                                        const Gdk::Rectangle& cell_area,
                                        const Gdk::Rectangle&,
                                        Gtk::CellRendererState flags)
{
  const int cell_xpad = property_xpad();
  const int cell_ypad = property_ypad();
  
  int x_offset = 0, y_offset = 0, width = 0, height = 0;
  get_size(widget, cell_area, x_offset, y_offset, width, height);

  width  -= cell_xpad * 2;
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
  int num = property_map_.get_value().size();
  if (num >= bar_width)
    parts = draw_more_pieces(property_map_.get_value(), bar_width, bar_height);
  else
  {
    parts = draw_more_pixels(property_map_.get_value(), bar_width, bar_height);
    scale = double(bar_width)/num;
  }
  
  gc->set_foreground(widget.get_style()->get_bg(Gtk::STATE_NORMAL));
  cwin->draw_rectangle(gc, true, bar_x, bar_y, bar_width, bar_height);
  
  for (std::list<Part>::iterator i = parts.begin();i != parts.end(); ++i)
  {
    Part & pa = *i;
    int fac = pa.fac;
    int rw = int(scale*(pa.last - pa.first + 1));
    int x = int(scale*pa.first) + bar_x;

    if (fac >= 100)  
      gc->set_foreground(dark_);
    else if (fac >= 50)
      gc->set_foreground(mid_);
    else
      gc->set_foreground(light_);
    
    cwin->draw_rectangle(gc, true, x, bar_y, rw, bar_height);
  }
}

Glib::PropertyProxy<std::list<bool> > CellRendererPieceMap::property_map()
{
  return property_map_.get_proxy();
}
