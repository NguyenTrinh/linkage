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

#include <gtkmm/treepath.h>
#include <gtkmm/treeselection.h>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/cellrendererprogress.h>

#include "TorrentList.hh"
#include "linkage/TorrentManager.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/SessionManager.hh"
#include "linkage/Utils.hh"

typedef Gtk::TreeSelection::ListHandle_Path PathList;

TorrentList::TorrentList()
{
  model = Gtk::TreeStore::create(columns);

  set_model(model);

  get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);

  append_column("#", columns.number);
  append_column("Name", columns.name);
  append_column("Status", columns.state);
  int cols_count = append_column("Down", columns.down);
  Gtk::TreeViewColumn* column = get_column(cols_count - 1);
  Gtk::CellRendererText* cell = dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell_renderer());
  column->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(this, &TorrentList::format_data), columns.down, ""));
  cols_count = append_column("Up", columns.up);
  column = get_column(cols_count - 1);
  cell = dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell_renderer());
  column->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(this, &TorrentList::format_data), columns.up, ""));
  cols_count = append_column("Down rate", columns.down_rate);
  column = get_column(cols_count - 1);
  cell = dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell_renderer());
  column->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(this, &TorrentList::format_data), columns.down_rate, "/s"));
  cols_count = append_column("Up rate", columns.up_rate);
  column = get_column(cols_count - 1);
  cell = dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell_renderer());
  column->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(this, &TorrentList::format_data), columns.up_rate, "/s"));
  append_column("Seeds", columns.seeds);
  append_column("Peers", columns.peers);
  Gtk::CellRendererProgress* prender = new Gtk::CellRendererProgress();
  cols_count = append_column("Progress", *Gtk::manage(prender));
  column = get_column(cols_count - 1);
  column->add_attribute(*prender, "value", cols_count);
  column->add_attribute(*prender, "text", cols_count + 1);

  for(int i = 0; i < 10; i++)
  {
    Gtk::TreeView::Column* column = get_column(i);
    column->set_sort_column_id(i + 1);
    if (i == 1)
    {
      std::vector<Gtk::CellRenderer*> renders = column->get_cell_renderers();
      column->clear_attributes(*renders[0]);
      column->add_attribute(*renders[0], "markup", 2);
    }
    column->set_resizable(true);
  }
  
  SettingsManager* sm = SettingsManager::instance();
  std::list<Glib::ustring> groups = sm->get_keys("Groups");
  for (std::list<Glib::ustring>::iterator iter = groups.begin(); iter != groups.end(); ++iter)
  {
    Gtk::TreePath path = model->get_path(add_group(*iter));
  }
  
  Gtk::SortType sort_order = Gtk::SortType(sm->get<int>("UI", "SortOrder"));
  model->set_sort_column_id(sm->get<int>("UI", "SortColumn"), sort_order);
  
  TorrentManager* tm = TorrentManager::instance();
  tm->signal_position_changed().connect(sigc::mem_fun(*this, &TorrentList::on_position_changed));
  tm->signal_group_changed().connect(sigc::mem_fun(*this, &TorrentList::on_group_changed));
  tm->signal_added().connect(sigc::mem_fun(*this, &TorrentList::on_added));
  tm->signal_removed().connect(sigc::mem_fun(*this, &TorrentList::on_removed));
  
  SessionManager::instance()->signal_session_resumed().connect(sigc::mem_fun(*this, &TorrentList::on_session_resumed));
}

TorrentList::~TorrentList()
{
  SettingsManager* sm = SettingsManager::instance();

  int column;
  Gtk::SortType order;
  model->get_sort_column_id(column, order);

  sm->set("UI", "SortOrder", int(order));
  sm->set("UI", "SortColumn", column);
  
  PathList path_list = get_selection()->get_selected_rows();
  if (path_list.size() == 1)
  {
    Gtk::TreeIter iter = model->get_iter(*path_list.begin());
    if (iter)
    {
      Gtk::TreeRow row = *iter;
      Glib::ustring hash_str = str(row[columns.hash]);
      sm->set("UI", "Selected", hash_str);
    }
  }
  
  Gtk::TreeNodeChildren children = model->children();
  for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
  {
    Gtk::TreePath path = model->get_path(iter);
    Gtk::TreeRow row = *iter;
    Glib::ustring name = row[columns.name];
    name = name.substr(3, name.size()-7);
    sm->set("Groups", name, row_expanded(path));
  }
}

Glib::SignalProxy0<void> TorrentList::signal_changed()
{
  return get_selection()->signal_changed();
}

void TorrentList::on_session_resumed()
{
	SettingsManager* sm = SettingsManager::instance();
	Gtk::TreeNodeChildren children = model->children();
  for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
  {
    Gtk::TreePath path = model->get_path(iter);
    Gtk::TreeRow row = *iter;
    Glib::ustring name = row[columns.name];
    name = name.substr(3, name.size()-7);
    if (sm->get<bool>("Groups", name))
	    expand_row(path, false);
  }
  
  Glib::ustring selected_hash = sm->get<Glib::ustring>("UI", "Selected");
  Gtk::TreeIter iter;
  
  Gtk::TreeNodeChildren parents = model->children();
  for (Gtk::TreeIter parent_iter = parents.begin();
       parent_iter != parents.end(); ++parent_iter)
  {
    Gtk::TreeRow row = *parent_iter;
    Gtk::TreeNodeChildren children = row.children();
    for (Gtk::TreeIter child_iter = children.begin();
          child_iter != children.end(); ++child_iter)
    {
      Gtk::TreeRow child_row = *child_iter;
      if (selected_hash == str(child_row[columns.hash]))
        iter = child_iter;
    }
  }
  
  if (iter)
    get_selection()->select(iter);
}

void TorrentList::on_position_changed(const sha1_hash& hash, int position)
{
  Gtk::TreeIter iter = get_iter(hash);
  
  if (iter)
  {
    Gtk::TreeRow row = *iter;
    row[columns.number] = position;
  }
}

void TorrentList::on_group_changed(const sha1_hash& hash, const Glib::ustring& group)
{
  Gtk::TreeIter iter = get_iter(hash);
  if (iter)
  {
    bool selected = is_selected(hash); //FIXME: Doesn't honour a multiple selection
    
    Gtk::TreeRow group_row = *get_iter(group);
    model->erase(iter);
    
    Gtk::TreeRow new_row = *(model->append(group_row.children()));
    new_row[columns.hash] = hash;
    if (selected)
      get_selection()->select(new_row);
  }
}

void TorrentList::on_added(const sha1_hash& hash, const Glib::ustring& name, const Glib::ustring& group, int position)
{
  Gtk::TreeRow group_row = *get_iter(group);
  Gtk::TreeRow new_row = *(model->append(group_row.children()));
  new_row[columns.hash] = hash;
  new_row[columns.name] = name;
  new_row[columns.number] = position;
}

void TorrentList::on_removed(const sha1_hash& hash)
{
  Gtk::TreeIter iter = get_iter(hash);
  model->erase(iter);
}

Gtk::TreeIter TorrentList::get_iter(const Glib::ustring& group)
{
  Gtk::TreeIter match;
  
  Gtk::TreeNodeChildren children = model->children();
  for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
  {
    Gtk::TreeRow row = *iter;
    if ("<i>" + group + "</i>" == row[columns.name])
      match = iter;
  }
  return match;
}

Gtk::TreeIter TorrentList::get_iter(const sha1_hash& hash)
{
  Gtk::TreeIter match;
  
  Gtk::TreeNodeChildren parents = model->children();
  for (Gtk::TreeIter parent_iter = parents.begin();
       parent_iter != parents.end(); ++parent_iter)
  {
    Gtk::TreeRow row = *parent_iter;
    Gtk::TreeNodeChildren children = row.children();
    for (Gtk::TreeIter child_iter = children.begin();
          child_iter != children.end(); ++child_iter)
    {
      Gtk::TreeRow child_row = *child_iter;
      if (hash == child_row[columns.hash])
        match = child_iter;
    }
  }
  return match;
}

bool TorrentList::is_selected(const sha1_hash& hash)
{
  PathList path_list = get_selection()->get_selected_rows();
  if (path_list.size() == 1)
  {
    Gtk::TreeIter iter = model->get_iter(*path_list.begin());
    if (iter)
    {
      Gtk::TreeRow row = *iter;
      return (hash == row[columns.hash]);
    }
  }
  return false;
}

HashList TorrentList::get_selected_list()
{
  PathList path_list = get_selection()->get_selected_rows();
  
  //Sort the selected list by columns.number to ease moving
  std::list<Gtk::TreeRow> ordered_list;
  for (PathList::iterator iter = path_list.begin();
        iter != path_list.end(); ++iter)
  {
    Gtk::TreeRow row = *(model->get_iter(*iter));
    if (row.parent()) //Don't add selected groups
    {
      for (std::list<Gtk::TreeRow>::iterator liter = ordered_list.begin();
            liter != ordered_list.end(); ++liter)
      {
        if (row[columns.number] < (*liter)[columns.number])
          ordered_list.insert(liter, row);
      }
      ordered_list.push_back(row);
    }
  }
  ordered_list.unique();
  HashList list;
  for (std::list<Gtk::TreeRow>::iterator iter = ordered_list.begin();
       iter != ordered_list.end(); ++iter)
  {
    list.push_back((*iter)[columns.hash]);
  }
  
  return list;
}

void TorrentList::select(const Glib::ustring& path)
{
  Gtk::TreeIter iter = model->get_iter(path);
  if (iter)
    get_selection()->select(iter);
}

void TorrentList::format_data(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter, const Gtk::TreeModelColumn<int>& column, const Glib::ustring& suffix)
{
  Gtk::TreeRow row = *iter;
  if (row.parent())
    dynamic_cast<Gtk::CellRendererText*>(cell)->property_text() = suffix_value(row[column]) + suffix;
}

void TorrentList::update_groups()
{
  Gtk::TreeNodeChildren parents = model->children();
  for (Gtk::TreeIter group_iter = parents.begin();
       group_iter != parents.end(); ++group_iter)
  {
    Gtk::TreeRow group_row = *group_iter;
    
    int peers = 0, seeds = 0;
    double progress = 0;
    Gtk::TreeNodeChildren children = group_row.children();
    for (Gtk::TreeIter iter = children.begin();
        iter != children.end(); ++iter)
    {
      Gtk::TreeRow row = *iter;
      peers += row[columns.peers];
      seeds += row[columns.seeds];
      progress += row[columns.progress];
    }
    group_row[columns.number] = children.size();
    group_row[columns.peers] = peers;
    group_row[columns.seeds] = seeds;
    if (children.size())
    {
      group_row[columns.progress] = progress/children.size();
      group_row[columns.eta] = "Average " + str(progress/children.size(), 2) + " %";
    }
    else
    {
      group_row[columns.progress] = 0;
      group_row[columns.eta] = "";
    }
  }
}

Gtk::TreeIter TorrentList::add_group(const Glib::ustring& name)
{
  Gtk::TreeIter iter = model->append();
  Gtk::TreeRow row = *iter;
  row[columns.name] = "<i>" + name + "</i>";
  return iter;
}

void TorrentList::update_row(Torrent* torrent)
{
  std::vector<Glib::ustring> states;
  states.push_back("Queued");
  states.push_back("Checking");
  states.push_back("Announcing");
  states.push_back("Downloading metadata");
  states.push_back("Downloading");
  states.push_back("Finished");
  states.push_back("Seeding");
  states.push_back("Allocating");

  int previous_down = SettingsManager::instance()->get<int>(str(torrent->get_hash()), "Down");
  int previous_up = SettingsManager::instance()->get<int>(str(torrent->get_hash()), "Up");
      
  Gtk::TreeRow row = *get_iter(torrent->get_hash());
  
  row[columns.number] = torrent->get_position();
  row[columns.name] = torrent->get_name();

  if (!torrent->get_handle().is_valid()) //Torrent is stopped
  {
    row[columns.state] = "Stopped";
    row[columns.down] = previous_down;
    row[columns.up] = previous_up;
    row[columns.down_rate] = 0;
    row[columns.up_rate] = 0;
    row[columns.seeds] = 0;
    row[columns.peers] = 0;
    row[columns.progress] = 0.0;
    row[columns.eta] = "";
    return;
  }
  
  torrent_status status = torrent->get_handle().status();
  int up = status.total_payload_upload + previous_up;
  int down = status.total_payload_download + previous_down;
  
  if (torrent->get_handle().is_seed()) // || status.state == torrent_status::finished)
  {
    
    int size = status.total_wanted_done - up;
    if (down)
    {
      double ratio = up/down;
      if (ratio > 1)
        ratio = 1;
      row[columns.eta] = str(ratio, 3) + " " + get_eta(size, status.upload_payload_rate);
      row[columns.progress] = double(ratio*100);
    }
    else
    {
      row[columns.eta] = "0.000 " + get_eta(size, status.upload_payload_rate);
      row[columns.progress] = 0;
    }
  }
  else
  {
    row[columns.progress] = double(status.progress*100);
    row[columns.eta] = str(double(status.progress*100), 2) + " % " + get_eta(status.total_wanted-status.total_wanted_done, status.download_payload_rate);
  }

  if (!torrent->get_handle().is_paused())
    row[columns.state] =  states[status.state];
  else
    row[columns.state] =  "Queued";
  row[columns.down] = down;
  row[columns.up] = up;
  row[columns.down_rate] = (int)status.download_payload_rate;
  row[columns.up_rate] = (int)status.upload_payload_rate;
  row[columns.seeds] = status.num_seeds;
  row[columns.peers] = status.num_peers - status.num_seeds;
}
