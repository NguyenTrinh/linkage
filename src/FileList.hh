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

#ifndef FILELIST_HH
#define FILELIST_HH

#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>

#include "linkage/Torrent.hh"

class FileList : public Gtk::TreeView
{
  class ModelColumns : public Gtk::TreeModelColumnRecord
  {
  public:
    ModelColumns()
      { 
        add(filter);
        add(name);
        add(map);
        add(done);
        add(size);
      }
    Gtk::TreeModelColumn<bool> filter;
    Gtk::TreeModelColumn<Glib::ustring> name;
    Gtk::TreeModelColumn<std::list<bool> > map;
    Gtk::TreeModelColumn<int> done;
    Gtk::TreeModelColumn<int> size;
  };
  
  ModelColumns columns;
  Glib::RefPtr<Gtk::ListStore> model;
  
  void on_filter_toggled(const Glib::ustring& path);
  
  void format_data(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter, const Gtk::TreeModelColumn<int>& column);
public:
  void clear();
  void update(Torrent* torrent);
  
  FileList();
  virtual ~FileList();
};

#endif /* FILELIST_HH */