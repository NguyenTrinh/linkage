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

#include <vector>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/cellrendererprogress.h>
#include <gtkmm/cellrenderertoggle.h>

#include "FileList.hh"
#include "CellRendererPieceMap.hh"
#include "linkage/Utils.hh"
#include "linkage/TorrentManager.hh"
#include "UI.hh"

FileList::FileList()
{
  model = Gtk::ListStore::create(columns);
  
  set_model(model);
  
  Gtk::CellRendererToggle* trender = new Gtk::CellRendererToggle();
  trender->signal_toggled().connect(sigc::mem_fun(*this, &FileList::on_filter_toggled));
  int cols_count = append_column("Filter", *Gtk::manage(trender));
  get_column(0)->add_attribute(*trender, "active", cols_count - 1);
  append_column("Name", columns.name);
  
  /* FIXME: Get colors from SettingsManager */
  Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
  
  Gdk::Color light, mid, dark = Gdk::Color("blue");
  mid.set_red(32767);
  mid.set_green(32767);
  mid.set_blue(65535);
  light.set_red(52428);
  light.set_green(52428);
  light.set_blue(65535);
  
  colormap->alloc_color(dark);
  colormap->alloc_color(mid);
  colormap->alloc_color(light);
  
  CellRendererPieceMap *renderer = new CellRendererPieceMap(dark, mid, light);
  Gtk::TreeViewColumn *column = new Gtk::TreeViewColumn("Progress", *Gtk::manage(renderer));
  append_column(*Gtk::manage(column));

  column->add_attribute(renderer->property_map(), columns.map);

  cols_count = append_column("Done", columns.done);
  column = get_column(cols_count - 1);
  Gtk::CellRendererText* cell = dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell_renderer());
  column->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(this, &FileList::format_data), columns.done));
  cols_count = append_column("Size", columns.size);
  column = get_column(cols_count - 1);
  cell = dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell_renderer());
  column->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(this, &FileList::format_data), columns.size));

  for(int i = 0; i < 5; i++)
  {
    Gtk::TreeView::Column* column = get_column(i);
    column->set_sort_column_id(i);
    column->set_resizable(true);
  }
}

FileList::~FileList()
{
}

void FileList::clear()
{
  model->clear();
}

void FileList::on_filter_toggled(const Glib::ustring& path)
{
  Gtk::TreeRow row = *(model->get_iter(path));
  row[columns.filter] = !row[columns.filter];
  
  HashList list = UI::instance()->torrent_list->get_selected_list();
  
  if (list.size() == 1)
  {
    Torrent* torrent = TorrentManager::instance()->get_torrent(*list.begin());
    torrent_info info = torrent->get_handle().get_torrent_info();
    
    std::vector<bool> filter(info.num_files(), false);
    
    Gtk::TreeNodeChildren children = model->children();
    for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
    {
      row = *iter;
      if (row[columns.filter])
      {
        int i = 0;
        for (i = 0; i < info.num_files(); i++)
          if (row[columns.name] == info.file_at(i).path.string())
            filter[i] = true;
      }
    }
    torrent->set_filter(filter);
  }
}

void FileList::format_data(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter, const Gtk::TreeModelColumn<int>& column)
{
  Gtk::TreeRow row = *iter;
  dynamic_cast<Gtk::CellRendererText*>(cell)->property_text() = suffix_value(row[column]);
}

/* FIXME: this doens't correctly display the progress bar on some finished files (1 pixel row missing at the end)
   It's probably a bug in *PieceMap, best show here though.
   
   This method hogs cpu =/ */
void FileList::update(Torrent* torrent)
{
  torrent_handle handle = torrent->get_handle();
  torrent_info info = handle.get_torrent_info();
  torrent_status status = handle.status();
  std::vector<bool> pieces = *status.pieces;
  std::vector<partial_piece_info> queue;
  handle.get_download_queue(queue);
  std::vector<float> fp;
  handle.file_progress(fp);
  std::vector<bool> filter = torrent->get_filter();
  
  int byte_pos = 0;
  int piece_index = 0;
  int completed_bytes = 0;
  int overlapped_bytes = 0;
  int pos = 0;
  
  Glib::ustring sel_name;
  bool select = false;
  Gtk::TreeIter iter = get_selection()->get_selected();
  if (iter)
  {
    Gtk::TreeModel::Row row = *iter;
    sel_name = row[columns.name];
    select = true;
  }
  
  clear();
  
  Gtk::CellRendererToggle* cell = dynamic_cast<Gtk::CellRendererToggle*>(get_column(0)->get_first_cell_renderer());
  cell->property_activatable() = !handle.is_seed();
  
  for (int i = 0; i < info.num_files(); i++)
  {
    std::list<bool> map;
    
    file_entry file = info.file_at(i);
    peer_request file_info = info.map_file(i, 0, file.size);
    
    int byte_pos_in_file = 0;
    int piece_index = file_info.piece;
    while (byte_pos_in_file < file.size)
    {
      if (pieces[piece_index])
        map.push_back(true);
      else
        map.push_back(false);
        
      byte_pos_in_file += info.piece_size(piece_index);
      piece_index++;
    }
    
    int completed_bytes = int(fp[i] * file.size);
    
    Gtk::TreeModel::Row row = *(model->append());
    row[columns.filter] = filter[i];
    row[columns.name] = file.path.string();
    row[columns.map] = map;
    row[columns.done] = completed_bytes;
    row[columns.size] = file.size;
  }
  if (select)
    for (int i = 0; i < model->children().size(); i++)
    {
      Gtk::TreeModel::Row row = model->children()[i];
      if (row[columns.name] == sel_name)
      {
        get_selection()->select(row);
        break;
      }
    }
}
