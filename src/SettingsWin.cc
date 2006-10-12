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

#include <gtkmm/stock.h>
#include <gtkmm/box.h>
#include <gtkmm/notebook.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include <gtkmm/button.h>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/cellrenderertoggle.h>
#include <gtkmm/treeview.h>

#include "SettingsWin.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/Utils.hh"
#include "linkage/PluginManager.hh"

SettingsWin::SettingsWin(Gtk::Window *parent)
{
  set_title("Prefrences");
  set_transient_for(*parent);
  set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
  set_skip_taskbar_hint(true);
  
  Gtk::VBox* main_box = manage(new Gtk::VBox());
  main_box->set_spacing(5);
  Gtk::Notebook* notebook = manage(new Gtk::Notebook());
  notebook->set_border_width(5);
  main_box->pack_start(*notebook, true, true);
  
  Gtk::HBox* hbox = manage(new Gtk::HBox());
  Gtk::Button* button = manage(new Gtk::Button(Gtk::Stock::CLOSE));
  button->signal_clicked().connect(sigc::mem_fun(this, &SettingsWin::on_button_close));
  hbox->pack_end(*button, false, false);
  main_box->pack_start(*hbox, false, false);
  
  Gtk::Table* table = manage(new Gtk::Table(2, 2));
  table->set_spacings(10);
  table->set_border_width(5);
  Gtk::Label* label = manage(new Gtk::Label("UI update interval:"));
  label->set_alignment(0, 0.5);
  table->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK);
  auto_expand = manage(new Gtk::CheckButton("Auto expand details"));
  table->attach(*auto_expand, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK);
  /*tray_icon = manage(new Gtk::CheckButton("Enable tray icon"));
  table->attach(*tray_icon, 0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK);
  min_to_tray = manage(new Gtk::CheckButton("Minimize to tray"));
  table->attach(*min_to_tray, 0, 1, 3, 4, Gtk::FILL, Gtk::SHRINK, 10);
  close_to_tray = manage(new Gtk::CheckButton("Close to tray"));
  table->attach(*close_to_tray, 0, 1, 4, 5, Gtk::FILL, Gtk::SHRINK, 10);*/
  Gtk::Adjustment *adj = manage(new Gtk::Adjustment(1.0, 1.0, 60.0));
  update_interval = manage(new Gtk::SpinButton(*adj));
  update_interval->set_numeric(true);
  update_interval->set_update_policy(Gtk::UPDATE_IF_VALID);
  table->attach(*update_interval, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK);
  
  notebook->append_page(*table, "General");
  
  table = manage(new Gtk::Table(8, 3));
  table->set_spacings(10);
  table->set_border_width(5);
  label = manage(new Gtk::Label("Listen on interface:"));
  label->set_alignment(0, 0.5);
  table->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK);
  label = manage(new Gtk::Label("Port range:"));
  label->set_alignment(0, 0.5);
  table->attach(*label, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK);
  label = manage(new Gtk::Label("Max. global upload rate:"));
  label->set_alignment(0, 0.5);
  table->attach(*label, 0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK);
  label = manage(new Gtk::Label("Max. global download rate:"));
  label->set_alignment(0, 0.5);
  table->attach(*label, 0, 1, 3, 4, Gtk::FILL, Gtk::SHRINK);
  label = manage(new Gtk::Label("Max. connections / torrent:"));
  label->set_alignment(0, 0.5);
  table->attach(*label, 0, 1, 4, 5, Gtk::FILL, Gtk::SHRINK);
  label = manage(new Gtk::Label("Max. uploads / torrent:"));
  label->set_alignment(0, 0.5);
  table->attach(*label, 0, 1, 5, 6, Gtk::FILL, Gtk::SHRINK);
  label = manage(new Gtk::Label("Max. active downloads:"));
  label->set_alignment(0, 0.5);
  table->attach(*label, 0, 1, 6, 7, Gtk::FILL, Gtk::SHRINK);
  label = manage(new Gtk::Label("Tracker timeout:"));
  label->set_alignment(0, 0.5);
  table->attach(*label, 0, 1, 7, 8, Gtk::FILL, Gtk::SHRINK);
  use_proxy = manage(new Gtk::CheckButton("Use proxy"));
  use_proxy->signal_toggled().connect(sigc::mem_fun(*this, &SettingsWin::on_proxy_toggled));
  table->attach(*use_proxy, 0, 3, 8, 9, Gtk::FILL, Gtk::SHRINK);

  Gtk::Table* table_proxy = manage(new Gtk::Table(3, 3));
  table_proxy->set_spacings(10);
  table_proxy->set_border_width(5);
  label = manage(new Gtk::Label("Proxy ip - port:"));
  label->set_alignment(0, 0.5);
  table_proxy->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK, 10);
  label = manage(new Gtk::Label("Proxy username:"));
  label->set_alignment(0, 0.5);
  table_proxy->attach(*label, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK, 10);
  label = manage(new Gtk::Label("Proxy password:"));
  label->set_alignment(0, 0.5);
  table_proxy->attach(*label, 0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK, 10);
  
  interfaces = manage(new Gtk::ComboBoxText());
  interfaces->set_row_separator_func(sigc::mem_fun(this, &SettingsWin::is_separator));
  interfaces->append_text("None specified");
  interfaces->append_text("-");
  std::list<Glib::ustring> if_list = get_interfaces();
  for (std::list<Glib::ustring>::iterator iter = if_list.begin();
        iter != if_list.end(); ++iter)
  {
    interfaces->append_text(*iter);
  }
  
  table->attach(*interfaces, 1, 2, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK);
  adj = manage(new Gtk::Adjustment(1024.0, 1024.0, 65534.0));
  min_port = manage(new Gtk::SpinButton(*adj));
  min_port->set_numeric(true);
  min_port->set_update_policy(Gtk::UPDATE_IF_VALID);
  table->attach(*min_port, 1, 2, 1, 2, Gtk::SHRINK|Gtk::EXPAND, Gtk::SHRINK);
  adj = manage(new Gtk::Adjustment(0.0, 0.0, 1000.0));
  up_rate = manage(new Gtk::SpinButton(*adj));
  up_rate->set_numeric(true);
  up_rate->set_update_policy(Gtk::UPDATE_IF_VALID);
  table->attach(*up_rate, 1, 2, 2, 3, Gtk::SHRINK|Gtk::EXPAND, Gtk::SHRINK);
  adj = manage(new Gtk::Adjustment(0.0, 0.0, 1000.0));
  down_rate = manage(new Gtk::SpinButton(*adj));
  down_rate->set_numeric(true);
  down_rate->set_update_policy(Gtk::UPDATE_IF_VALID);
  table->attach(*down_rate, 1, 2, 3, 4, Gtk::SHRINK|Gtk::EXPAND, Gtk::SHRINK);
  adj = manage(new Gtk::Adjustment(0.0, 0.0, 10000.0));
  max_connections = manage(new Gtk::SpinButton(*adj));
  max_connections->set_numeric(true);
  max_connections->set_update_policy(Gtk::UPDATE_IF_VALID);
  table->attach(*max_connections, 1, 2, 4, 5, Gtk::SHRINK|Gtk::EXPAND, Gtk::SHRINK);
  adj = manage(new Gtk::Adjustment(0.0, 0.0, 10000.0));
  max_uploads = manage(new Gtk::SpinButton(*adj));
  max_uploads->set_numeric(true);
  max_uploads->set_update_policy(Gtk::UPDATE_IF_VALID);
  table->attach(*max_uploads, 1, 2, 5, 6, Gtk::SHRINK|Gtk::EXPAND, Gtk::SHRINK);
  adj = manage(new Gtk::Adjustment(0.0, 0.0, 100.0));
  max_active = manage(new Gtk::SpinButton(*adj));
  max_active->set_numeric(true);
  max_active->set_update_policy(Gtk::UPDATE_IF_VALID);
  table->attach(*max_active, 1, 2, 6, 7, Gtk::SHRINK|Gtk::EXPAND, Gtk::SHRINK);
  
  adj = manage(new Gtk::Adjustment(10.0, 5.0, 60.0));
  tracker_timeout = manage(new Gtk::SpinButton(*adj));
  tracker_timeout->set_numeric(true);
  tracker_timeout->set_update_policy(Gtk::UPDATE_IF_VALID);
  table->attach(*tracker_timeout, 1, 2, 7, 8, Gtk::SHRINK|Gtk::EXPAND, Gtk::SHRINK);
  
  proxy_ip = manage(new Gtk::Entry());
  table_proxy->attach(*proxy_ip, 1, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);
  proxy_user = manage(new Gtk::Entry());
  table_proxy->attach(*proxy_user, 1, 2, 1, 2, Gtk::FILL, Gtk::SHRINK);
  proxy_pass = manage(new Gtk::Entry());
  proxy_pass->property_visibility() = false;
  table_proxy->attach(*proxy_pass, 1, 2, 2, 3, Gtk::FILL, Gtk::SHRINK);
  
  adj = manage(new Gtk::Adjustment(1025.0, 1025.0, 65535.0));
  max_port = manage(new Gtk::SpinButton(*adj));
  max_port->set_numeric(true);
  max_port->set_update_policy(Gtk::UPDATE_IF_VALID);
  table->attach(*max_port, 2, 3, 1, 2, Gtk::SHRINK, Gtk::SHRINK);
  label = manage(new Gtk::Label("kB/s"));
  label->set_alignment(0, 0.5);
  table->attach(*label, 2, 3, 2, 3, Gtk::SHRINK, Gtk::SHRINK);
  label = manage(new Gtk::Label("kB/s"));
  label->set_alignment(0, 0.5);
  table->attach(*label, 2, 3, 3, 4, Gtk::SHRINK, Gtk::SHRINK);
  
  adj = manage(new Gtk::Adjustment(8080.0, 0.0, 65535.0));
  proxy_port = manage(new Gtk::SpinButton(*adj));
  proxy_port->set_numeric(true);
  proxy_port->set_update_policy(Gtk::UPDATE_IF_VALID);
  table_proxy->attach(*proxy_port, 2, 3, 0, 1, Gtk::SHRINK, Gtk::SHRINK);
  
  Gtk::VBox* vbox = manage(new Gtk::VBox());
  vbox->pack_start(*table);
  vbox->pack_start(*table_proxy);
  notebook->append_page(*vbox, "Network");
  
  table = manage(new Gtk::Table(2, 3));
  table->set_spacings(10);
  table->set_border_width(5);
  default_path = manage(new Gtk::CheckButton("Use default save path:"));
  default_path->set_alignment(0, 0.5);
  table->attach(*default_path, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK);
  move_finished = manage(new Gtk::CheckButton("Move finished torrents:"));
  move_finished->set_alignment(0, 0.5);
  table->attach(*move_finished, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK);
  allocate = manage(new Gtk::CheckButton("Allocate disk space"));
  table->attach(*allocate, 0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK);
  button_default_path = manage(new Gtk::FileChooserButton("Select folder",
                                  Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER));
  table->attach(*button_default_path, 1, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);
  button_move_finished = manage(new Gtk::FileChooserButton("Select folder",
                                    Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER));
  table->attach(*button_move_finished, 1, 2, 1, 2, Gtk::FILL, Gtk::SHRINK);
  
  button_move_finished->set_sensitive(false);
  move_finished->set_sensitive(false);
  
  notebook->append_page(*table, "Files & Folders");
  
  model_plugins = Gtk::ListStore::create(columns);
  Gtk::TreeView* treeview_plugins = manage(new Gtk::TreeView());
  treeview_plugins->set_model(model_plugins);
  Gtk::CellRendererToggle* trender = new Gtk::CellRendererToggle();
  trender->signal_toggled().connect(sigc::mem_fun(*this, &SettingsWin::on_plugin_toggled));
  int cols_count = treeview_plugins->append_column("Load", *manage(trender));
  treeview_plugins->get_column(0)->add_attribute(*trender, "active", cols_count - 1);
  treeview_plugins->append_column("Name", columns.name);
  treeview_plugins->append_column("Description", columns.description);
  for(int i = 0; i < 3; i++)
  {
    Gtk::TreeView::Column* column = treeview_plugins->get_column(i);
    column->set_sort_column_id(i);
    column->set_resizable(true);
  }
  
  notebook->append_page(*treeview_plugins, "Plugins");
  
  add(*main_box);
  
  min_port->signal_value_changed().connect(sigc::mem_fun(*this, &SettingsWin::on_min_port_changed));
  max_port->signal_value_changed().connect(sigc::mem_fun(*this, &SettingsWin::on_max_port_changed));
  
  //signal_hide().connect(sigc::mem_fun(*this, &SettingsWin::on_hide));
  //signal_show().connect(sigc::mem_fun(*this, &SettingsWin::on_show));
  
  show_all_children();
}

SettingsWin::~SettingsWin()
{
}

bool SettingsWin::on_delete_event(GdkEventAny*)
{
  hide();
  
  return true;
}

void SettingsWin::on_button_close()
{
  hide();
}

void SettingsWin::on_proxy_toggled()
{
  if (use_proxy->get_active())
    proxy_ip->get_parent()->show();
  else
    proxy_ip->get_parent()->hide();
}

void SettingsWin::on_min_port_changed()
{
  max_port->set_range(min_port->get_value() + 1, 65535.0);
}

void SettingsWin::on_max_port_changed()
{
  min_port->set_range(1024.0, max_port->get_value() - 1);
}

void SettingsWin::on_plugin_toggled(const Glib::ustring& path)
{
  Gtk::TreeRow row = *(model_plugins->get_iter(path));
  row[columns.load] = !row[columns.load];
}
  
void SettingsWin::on_hide()
{
  SettingsManager* sm = SettingsManager::instance();
  
  /* Network */
  if (interfaces->get_active_row_number() <= 0)
    sm->set("Network", "Interface", Glib::ustring(""));
  else
    sm->set("Network", "Interface", interfaces->get_active_text());
  sm->set("Network", "MinPort", (int)min_port->get_value());
  sm->set("Network", "MaxPort", (int)max_port->get_value());
  sm->set("Network", "MaxUpRate", (int)up_rate->get_value());
  sm->set("Network", "MaxDownRate", (int)down_rate->get_value());
  int uploads = (int)max_uploads->get_value();
  if (uploads == 0)
    uploads = -1;
  sm->set("Network", "MaxUploads", uploads);
  int connections = (int)max_connections->get_value();
  if (connections == 1)
    connections = 2;
  else if (connections == 0)
    connections = -1;
  sm->set("Network", "MaxConnections", connections);
  sm->set("Network", "MaxActive", (int)max_active->get_value());
  sm->set("Network", "TrackerTimeout", (int)tracker_timeout->get_value());
  sm->set("Network", "UseProxy", use_proxy->get_active());
  sm->set("Network", "ProxyIp", proxy_ip->get_text());
  sm->set("Network", "ProxyPort", (int)proxy_port->get_value());
  sm->set("Network", "ProxyLogin", proxy_user->get_text());
  sm->set("Network", "ProxyPass", proxy_pass->get_text());
  /* UI */
  sm->set("UI", "Interval", (int)update_interval->get_value());
  sm->set("UI", "AutoExpand", auto_expand->get_active());
  /* Use libegg plugin instead until gtkmm 2.10 is common among distros
  sm->set("UI", "TrayIcon", tray_icon->get_active());
  sm->set("UI", "MinToTray", min_to_tray->get_active());
  sm->set("UI", "CloseToTray", close_to_tray->get_active());*/
  Gtk::TreeNodeChildren children = model_plugins->children();
  std::list<Glib::ustring> plugins;
  for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
  {
    Gtk::TreeRow row = *iter;
    if (row[columns.load])
      plugins.push_back(row[columns.name]);
  }
  sm->set("UI", "Plugins", UStringArray(plugins));
  /* Files */
  sm->set("Files", "UseDefPath", default_path->get_active());
  sm->set("Files", "DefPath", button_default_path->get_filename());
  sm->set("Files", "MoveFinished", move_finished->get_active());
  sm->set("Files", "FinishedPath", button_move_finished->get_filename());
  sm->set("Files", "Allocate", allocate->get_active());
  
  Gtk::Window::on_hide();
  
  SettingsManager::instance()->signal_update_settings_.emit();
}

void SettingsWin::on_show()
{
  SettingsManager* sm = SettingsManager::instance();

  /* Network */
  Glib::ustring interface = sm->get<Glib::ustring>("Network", "Interface");
  min_port->set_value((double)sm->get<int>("Network", "MinPort"));
  max_port->set_value((double)sm->get<int>("Network", "MaxPort"));
  up_rate->set_value((double)sm->get<int>("Network", "MaxUpRate"));
  down_rate->set_value((double)sm->get<int>("Network", "MaxDownRate"));
  max_uploads->set_value((double)sm->get<int>("Network", "MaxUploads"));
  max_connections->set_value((double)sm->get<int>("Network", "MaxConnections"));
  max_active->set_value((double)sm->get<int>("Network", "MaxActive"));
  tracker_timeout->set_value((double)sm->get<int>("Network", "TrackerTimeout"));
  use_proxy->set_active(sm->get<bool>("Network", "UseProxy"));
  proxy_ip->set_text(sm->get<Glib::ustring>("Network", "ProxyIP"));
  proxy_port->set_value((double)sm->get<int>("Network", "ProxyPort"));
  proxy_user->set_text(sm->get<Glib::ustring>("Network", "ProxyLogin"));
  proxy_pass->set_text(sm->get<Glib::ustring>("Network", "ProxyPass"));
  /* UI */
  update_interval->set_value((double)sm->get<int>("UI", "Interval"));
  auto_expand->set_active(sm->get<bool>("UI", "AutoExpand"));
  std::list<Glib::ustring> plugins = sm->get<UStringArray>("UI", "Plugins");
  /* Files */
  default_path->set_active(sm->get<bool>("Files", "UseDefPath"));
  button_default_path->set_filename(sm->get<Glib::ustring>("Files", "DefPath"));
  move_finished->set_active(sm->get<bool>("Files", "MoveFinished"));
  button_move_finished->set_filename(sm->get<Glib::ustring>("Files", "FinishedPath"));
  allocate->set_active(sm->get<bool>("Files", "Allocate"));

  if (interface != "")
    interfaces->set_active_text(interface);
  else
    interfaces->set_active(0);
    
  if (!use_proxy->get_active())
    proxy_ip->get_parent()->hide();
  else
    proxy_ip->get_parent()->show();

  if (!model_plugins->children().size())
  {
    std::list<PluginInfo> plugins = PluginManager::instance()->list_plugins();
    
    for (std::list<PluginInfo>::iterator iter = plugins.begin();
         iter != plugins.end(); ++iter)
    {
      PluginInfo info = *iter;
      Gtk::TreeRow row = *(model_plugins->append());
      row[columns.name] = info.get_name();
      row[columns.description] = info.get_description();
      row[columns.load] = info.get_loaded();
    }
  }
    
  for (std::list<Glib::ustring>::iterator iter = plugins.begin();
       iter != plugins.end(); ++iter)
  {
    bool active = false;
    Gtk::TreeRow row;
    Gtk::TreeNodeChildren children = model_plugins->children();
    for (Gtk::TreeIter tree_iter = children.begin();
         tree_iter != children.end(); ++tree_iter)
    {
      row = *tree_iter;
      if (row[columns.name] == *iter)
      {
        active = true;
        break;
      }
    }
    row[columns.load] = active;
  }
  Gtk::Window::on_show();
}

bool SettingsWin::is_separator(const Glib::RefPtr<Gtk::TreeModel>& model, 
                               const Gtk::TreeModel::iterator& iter)
{
  bool separator = false;
  int selected = interfaces->get_active_row_number();
  interfaces->set_active(iter);
  if (interfaces->get_active_text() == "-")
    separator = true;
  interfaces->set_active(selected);
  return separator;
}
