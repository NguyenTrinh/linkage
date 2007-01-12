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

#ifndef SETTINGS_WIN_HH
#define SETTINGS_WIN_HH

#include <gtkmm/window.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/comboboxentrytext.h>
#include <gtkmm/entry.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>

#include "GroupFilter.hh"

class SettingsWin : public Gtk::Window
{
  class PluginModelColumns : public Gtk::TreeModelColumnRecord
  {
  public:
    PluginModelColumns()
      { 
        add(load);
        add(name);
        add(description);
      }
    Gtk::TreeModelColumn<bool> load;
    Gtk::TreeModelColumn<Glib::ustring> name;
    Gtk::TreeModelColumn<Glib::ustring> description;
  };
  
  class GroupFilterRow : public Gtk::HBox
  {
  	Gtk::Entry *m_name;
  	Gtk::ComboBoxText *m_eval, *m_tag;
  	Gtk::ComboBoxEntryText *m_filter;
  	
  	void on_tag_changed();
  	
  public:
  	GroupFilter get_filter() const;
  	
    GroupFilterRow();
    GroupFilterRow(const Glib::ustring& filter,
				  				 GroupFilter::TagType tag,
				  				 GroupFilter::EvalType eval,
				  				 const Glib::ustring& name);
    ~GroupFilterRow();
  };
  
  class GroupFilterView : public Gtk::VBox
  {
  	std::list<GroupFilterRow*> m_children;
  	
  public:
  	void append(GroupFilterRow* row);
  	void remove(GroupFilterRow* row);
  	const std::list<GroupFilterRow*>& children();
  	
  	GroupFilterView();
  	~GroupFilterView();
  };
  
  PluginModelColumns plugin_columns;
  
  Glib::RefPtr<Gtk::ListStore> model_plugins;
  
  Gtk::SpinButton* update_interval;
  /* Gtk::CheckButton *tray_icon, *min_to_tray, *close_to_tray; */
  Gtk::CheckButton* auto_expand;
  
  Gtk::ComboBoxText* interfaces;
  Gtk::SpinButton *min_port, *max_port, *proxy_port;
  Gtk::SpinButton *up_rate, *down_rate;
  Gtk::SpinButton *max_connections, *max_uploads, *max_active;
  Gtk::SpinButton *tracker_timeout;
  
  Gtk::CheckButton *default_path, *move_finished, *allocate, *use_proxy;
  Gtk::FileChooserButton *button_default_path, *button_move_finished;
  Gtk::Entry *proxy_ip, *proxy_user, *proxy_pass;
  
  GroupFilterView* groups_view;
  
  void on_plugin_toggled(const Glib::ustring& path);

  void list_interfaces();
  bool is_separator(const Glib::RefPtr<Gtk::TreeModel>& model, 
                    const Gtk::TreeModel::iterator& iter);
  
  virtual bool on_delete_event(GdkEventAny*);
  void on_button_close();
  void on_hide();
  void on_show();
  void on_min_port_changed();
  void on_max_port_changed();
  
  void on_proxy_toggled();
  
  void on_group_add();
  void on_group_remove();
  
public:
  SettingsWin(Gtk::Window *parent);
  virtual ~SettingsWin();
};

#endif /* SETTINGS_WIN_HH */
