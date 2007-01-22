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

#ifndef PIECEMAP_HH
#define PIECEMAP_HH

#include <gtkmm/drawingarea.h>
#include <gdkmm/window.h>

struct Part
{
  unsigned int first;
  unsigned int last;
  unsigned int fac;
  
  Part(unsigned int fi, unsigned int la, unsigned int fa);
};

class PieceMap : public Gtk::DrawingArea
{
  std::vector<bool> m_map;
  Gdk::Color m_dark, m_mid, m_light;

  std::list<Part> draw_more_pieces();
  std::list<Part> draw_more_pixels(); //FIXME: Broken again, dark not shown on more_pixels?
  void draw(); //FIXME: Should be public to be able to redraw unchanged map
  
  virtual void on_realize();
  virtual bool on_expose_event(GdkEventExpose*);

public:
  void set_map(const std::vector<bool>& map);

  PieceMap(Gdk::Color& dark, Gdk::Color& mid, Gdk::Color& light);
  virtual ~PieceMap();
};
  
#endif 
