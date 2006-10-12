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

#include "PieceMap.hh"

/* FIXME: Clean this class up! */

PieceMap::PieceMap(Gdk::Color& dark, Gdk::Color& mid, Gdk::Color& light)
{
  /*Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
  
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

void PieceMap::on_realize()
{
  Gtk::DrawingArea::on_realize();
  
  get_window()->clear();
}

bool PieceMap::on_expose_event(GdkEventExpose*)
{
  if (!map_.size())
    map_.push_back(false);
    
  draw();
  
  return true;
}

PieceMap::~PieceMap()
{
}

std::list<Part> PieceMap::draw_more_pixels()
{
  std::list<Part> parts;
  int count = map_.size();
  int width = get_allocation().get_width() - (get_style()->get_xthickness() * 2);
  int height = get_allocation().get_height() - (get_style()->get_ythickness() * 2);
  
  for (int i = 0; i < count; i++)
  {
    if (!map_[i])
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

std::list<Part> PieceMap::draw_more_pieces()
{
  std::list<Part> parts;
  int count = map_.size();
  int width = get_allocation().get_width() - (get_style()->get_xthickness() * 2);
  int height = get_allocation().get_height() - (get_style()->get_ythickness() * 2);
  double pieces_per_part = (double)count/width;
  
  for (int i = 0; i < width; i++)
  {
    int completed = 0;
    int start = (int)(i*pieces_per_part);
    int end = (int)((i+1)*pieces_per_part+0.5);
    for (int j = start; j < end; j++)
      if (map_[j])
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

void PieceMap::set_map(const std::vector<bool>& map)
{
  map_.assign(map.begin(), map.end());
  
  draw();
}

void PieceMap::draw()
{
  set_size_request(-1, 32);
  if (!is_drawable())
    return;
  
  int w = get_allocation().get_width();
  int h = get_allocation().get_height();
  Glib::RefPtr<Gdk::Window> paint_win = get_window();
  Gdk::Rectangle rect = Gdk::Rectangle(0, 0, w, h);
  Gtk::Widget* widget = dynamic_cast<Gtk::Widget*>(this);
  get_style()->paint_box(paint_win, Gtk::STATE_NORMAL, Gtk::SHADOW_IN,
                         rect, *widget, "", 0, 0, w, h);
  
  std::list<Part> parts;
  double scale = 1.0;
  if (map_.size() >= w - (get_style()->get_xthickness() * 2))
    parts = draw_more_pieces();
  else
  {
    parts = draw_more_pixels();
    scale = (double)(w - (get_style()->get_xthickness() * 2))/map_.size();
  }
    
  Glib::RefPtr<Gdk::GC> gc = Gdk::GC::create(get_window());
  
  int rh = h - (get_style()->get_ythickness() * 2);
  for (std::list<Part>::iterator i = parts.begin();i != parts.end();++i)
	{
    Part & pa = *i;
		int rw = int(scale*(pa.last - pa.first + 1));
		int fac = pa.fac;
		int x = int(scale*pa.first) + get_style()->get_xthickness();

    if (fac >= 100)  
      gc->set_foreground(dark_);
    else if (fac >= 50)
      gc->set_foreground(mid_);
    else
      gc->set_foreground(light_);
      
    get_window()->draw_rectangle(gc, true, x, get_style()->get_ythickness(), rw, rh);
  }
}

Part::Part(int first_, int last_, int fac_) : 
  first(first_), last(last_), fac(fac_)
{
}
