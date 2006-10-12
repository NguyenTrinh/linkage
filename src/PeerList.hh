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

#ifndef PEERLIST_HH
#define PEERLIST_HH

#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>

#ifdef HAVE_LIBGEOIP
#include <gdkmm/pixbuf.h>

#include <GeoIP.h> 
#endif /* HAVE_LIBGEOIP */

#include "linkage/Torrent.hh"

class PeerList : public Gtk::TreeView
{
  class ModelColumns : public Gtk::TreeModelColumnRecord
  {
  public:
    ModelColumns()
      { add(flag);
        add(address);
        add(down);
        add(up);
        add(down_rate);
        add(up_rate);
        add(progress);
        add(client);
      }
    Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > flag;
    Gtk::TreeModelColumn<Glib::ustring> address;
    Gtk::TreeModelColumn<Glib::ustring> down;
    Gtk::TreeModelColumn<Glib::ustring> up;
    Gtk::TreeModelColumn<Glib::ustring> down_rate;
    Gtk::TreeModelColumn<Glib::ustring> up_rate;
    Gtk::TreeModelColumn<double> progress;
    Gtk::TreeModelColumn<Glib::ustring> client;
  };

  ModelColumns columns;
  Glib::RefPtr<Gtk::ListStore> model;
  
public:
  void clear();
  void update(Torrent *torrent);
  
  PeerList();
  virtual ~PeerList();
};

#endif /* PEERLIST_HH */
