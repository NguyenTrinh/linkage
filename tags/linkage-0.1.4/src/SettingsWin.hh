/*
Copyright (C) 2006-2007   Christian Lundgren
Copyright (C) 2007        Dave Moore

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA	02110-1301, USA.
*/

#ifndef SETTINGS_WIN_HH
#define SETTINGS_WIN_HH

#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>

#include <libglademm.h>

class SettingsWin : public Gtk::Window
{
	Glib::RefPtr<Gnome::Glade::Xml> glade_xml;

	Gtk::SpinButton *update_interval, *name_width;
	Gtk::CheckButton *auto_expand, *trunkate_names, *allow_linebreaks;
	Gtk::ColorButton *color_downloading, *color_seeding, *color_queued, *color_error;
	Gtk::ColorButton *color_check_queue, *color_checking, *color_finished;
	Gtk::ColorButton *color_allocating, *color_announcing, *color_stopped;

	Gtk::ComboBoxText* interfaces;
	Gtk::SpinButton *min_port, *max_port;
	Gtk::SpinButton *tracker_timeout;
	Gtk::CheckButton *enable_dht, *dht_fallback, *enable_pex, *multiple_connections;
	Gtk::SpinButton *max_connections, *max_uploads, *max_active;
	Gtk::SpinButton *up_rate, *down_rate;
	Gtk::SpinButton *max_torrent_connections, *max_torrent_uploads, *seed_ratio;
	Gtk::SpinButton	*proxy_port;
	Gtk::Entry *proxy_ip, *proxy_user, *proxy_pass;

	Gtk::CheckButton *default_path, *move_finished, *allocate;
	Gtk::FileChooserButton *button_default_path, *button_move_finished;
	Gtk::SpinButton *max_open;
	
	class PluginModelColumns : public Gtk::TreeModelColumnRecord
	{
	public:
		PluginModelColumns()
			{
				add(load);
				add(name);
				add(description);
				add(author);
				add(website);
				add(version);
				add(configurable);
				add(file);
			}
		Gtk::TreeModelColumn<bool> load;
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<Glib::ustring> description;
		Gtk::TreeModelColumn<Glib::ustring> author;
		Gtk::TreeModelColumn<Glib::ustring> website;
		Gtk::TreeModelColumn<Glib::ustring> version;
		Gtk::TreeModelColumn<bool> configurable;
		Gtk::TreeModelColumn<Glib::ustring> file;
	};

	Gtk::TreeView* treeview_plugins;
	PluginModelColumns plugin_columns;
	Glib::RefPtr<Gtk::ListStore> model_plugins;
	Gtk::Button *about_plugin, *configure_plugin;

	void on_plugin_toggled(const Glib::ustring& path);
	void on_plugin_changed();
	void on_configure_plugin();
	void on_about_plugin();

	void list_interfaces();

	bool on_delete_event(GdkEventAny*);
	void on_button_close();
	void on_hide();
	void on_show();
	void on_min_port_changed();
	void on_max_port_changed();

	Glib::ustring hex_str(const Gdk::Color& color);

	bool is_separator(const Glib::RefPtr<Gtk::TreeModel>& model,
										const Gtk::TreeIter& iter);
public:
	SettingsWin(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	virtual ~SettingsWin();
};

#endif /* SETTINGS_WIN_HH */

