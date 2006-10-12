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

#ifndef CELLRENDERER_PIECEMAP_HH
#define CELLRENDERER_PIECEMAP_HH

#include <gtkmm/cellrenderer.h>

/* Has Part class */
#include "PieceMap.hh"

class CellRendererPieceMap : public Gtk::CellRenderer
{
  //std::vector<bool> map_;
  Gdk::Color dark_, mid_, light_;
  
  Glib::Property<std::list<bool> > property_map_;
protected:
  std::list<Part> draw_more_pieces(const std::list<bool>& map,
                                       int width,
                                       int height);
  std::list<Part> draw_more_pixels(const std::list<bool>& map,
                                       int width,
                                       int height);
  
  virtual void render_vfunc(const Glib::RefPtr<Gdk::Drawable>& window,
                            Gtk::Widget& widget,
                            const Gdk::Rectangle&,
                            const Gdk::Rectangle& cell_area,
                            const Gdk::Rectangle&,
                            Gtk::CellRendererState flags);

  virtual void get_size_vfunc(Gtk::Widget& widget,
                              const Gdk::Rectangle* cell_area,
                              int* x_offset, int* y_offset,
                              int* width,    int* height) const;                              
public:
  Glib::PropertyProxy<std::list<bool> > property_map();
  
  CellRendererPieceMap(Gdk::Color& dark, 
                       Gdk::Color& mid, 
                       Gdk::Color& light);
  virtual ~CellRendererPieceMap();
};
  
#endif 
