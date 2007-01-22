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
#include "linkage/Engine.hh"

FileList::FileList()
{
  model = Gtk::ListStore::create(columns);
  
  set_model(model);
  
  Gtk::CellRendererToggle* trender = new Gtk::CellRendererToggle();
  trender->signal_toggled().connect(sigc::mem_fun(*this, &FileList::on_filter_toggled));
  unsigned int cols_count = append_column("Filter", *Gtk::manage(trender));
  get_column(0)->add_attribute(*trender, "active", cols_count - 1);
  append_column("Name", columns.name);
  
  /* FIXME: Get colors from SettingsManager */
  Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();
  
  std::vector<unsigned int> rgb = Engine::instance()->get_settings_manager()->get_int_list("UI", "ColorDark");
  Gdk::Color light, mid, dark;
  dark.set_red(rgb[0]);
  dark.set_green(rgb[1]);
  dark.set_blue(rgb[2]);
  rgb = Engine::instance()->get_settings_manager()->get_int_list("UI", "ColorMid");
  mid.set_red(rgb[0]);
  mid.set_green(rgb[1]);
  mid.set_blue(rgb[2]);
  rgb = Engine::instance()->get_settings_manager()->get_int_list("UI", "ColorLight");
  light.set_red(rgb[0]);
  light.set_green(rgb[1]);
  light.set_blue(rgb[2]);
  
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

  for(unsigned int i = 0; i < 5; i++)
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
  WeakPtr<Torrent> torrent = Engine::instance()->get_torrent_manager()->get_torrent(current_hash);
  
  if (torrent->is_valid())
  {
  	row[columns.filter] = !row[columns.filter];
  	torrent->filter_file(row[columns.name], row[columns.filter]);
  }
}

void FileList::format_data(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter, const Gtk::TreeModelColumn<unsigned int>& column)
{
  Gtk::TreeRow row = *iter;
  dynamic_cast<Gtk::CellRendererText*>(cell)->property_text() = suffix_value(row[column]);
}

/* FIXME: this doens't correctly display the progress bar on some finished files (1 pixel row missing at the end)
   It's probably a bug in *PieceMap, best shown here though.
   
   This method hogs cpu =/ */
void FileList::update(const WeakPtr<Torrent>& torrent)
{
	current_hash = torrent->get_hash();
	
  torrent_handle handle = torrent->get_handle();
  torrent_info info = torrent->get_info();
  torrent_status status = torrent->get_status();
  std::vector<bool> pieces = *status.pieces;
  std::vector<partial_piece_info> queue = torrent->get_download_queue();
  std::vector<float> fp = torrent->get_file_progress();
  std::vector<bool> filter = torrent->get_filter();

  unsigned int byte_pos = 0;
  unsigned int piece_index = 0;
  unsigned int completed_bytes = 0;
  unsigned int overlapped_bytes = 0;
  unsigned int pos = 0;
  
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
  cell->property_activatable() = !(torrent->get_state() & Torrent::SEEDING);
  
  for (unsigned int i = 0; i < info.num_files(); i++)
  {
    std::list<bool> map;
    
    file_entry file = info.file_at(i);
    peer_request file_info = info.map_file(i, 0, file.size);
    
    unsigned int byte_pos_in_file = 0;
    unsigned int piece_index = file_info.piece;
    while (byte_pos_in_file < file.size)
    {
      if (pieces[piece_index])
        map.push_back(true);
      else
        map.push_back(false);
        
      byte_pos_in_file += info.piece_size(piece_index);
      piece_index++;
    }
    
    unsigned int completed_bytes = (unsigned int)(fp[i] * file.size);
    
    Gtk::TreeModel::Row row = *(model->append());
    row[columns.filter] = filter[i];
    row[columns.name] = file.path.string();
    row[columns.map] = map;
    row[columns.done] = completed_bytes;
    row[columns.size] = file.size;
  }
  if (select)
    for (unsigned int i = 0; i < model->children().size(); i++)
    {
      Gtk::TreeModel::Row row = model->children()[i];
      if (row[columns.name] == sel_name)
      {
        get_selection()->select(row);
        break;
      }
    }
}
