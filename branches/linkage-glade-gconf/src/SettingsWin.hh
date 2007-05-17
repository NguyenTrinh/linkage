/*
Copyright (C) 2006	Christian Lundgren

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
#include <gtkmm/alignment.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>

#include "AlignedFrame.hh"
#include "AlignedLabel.hh"
#include "AlignedSpinButton.hh"
#include "GroupView.hh"

class SettingsWin : public Gtk::Window
{
	AlignedSpinButton *update_interval, *name_width;
	Gtk::CheckButton *auto_expand, *trunkate_names;
	Gtk::ColorButton *color_downloading, *color_seeding, *color_queued, *color_error;
	Gtk::ColorButton *color_check_queue, *color_checking, *color_finished;
	Gtk::ColorButton *color_allocating, *color_announcing, *color_stopped;

	Gtk::ComboBoxText* interfaces;
	AlignedSpinButton *min_port, *max_port;
	AlignedSpinButton *tracker_timeout;
	Gtk::CheckButton *enable_dht, *dht_fallback, *enable_pex, *multiple_connections;
	AlignedSpinButton *max_connections, *max_uploads, *max_active;
	AlignedSpinButton *up_rate, *down_rate;
	AlignedSpinButton *max_torrent_connections, *max_torrent_uploads, *seed_ratio;
	AlignedSpinButton* proxy_port;
	Gtk::Entry *proxy_ip, *proxy_user, *proxy_pass;

	Gtk::CheckButton *default_path, *move_finished, *allocate;
	Gtk::FileChooserButton *button_default_path, *button_move_finished;
	AlignedSpinButton* max_open;

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
				add(file);
			}
		Gtk::TreeModelColumn<bool> load;
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<Glib::ustring> description;
		Gtk::TreeModelColumn<Glib::ustring> author;
		Gtk::TreeModelColumn<Glib::ustring> website;
		Gtk::TreeModelColumn<Glib::ustring> file;
	};

	PluginModelColumns plugin_columns;
	Glib::RefPtr<Gtk::ListStore> model_plugins;
	AlignedLabel *label_author, *label_website, *label_file;
	AlignedFrame* frame_options;

	Gtk::Button *group_add, *group_remove;
	GroupView* groups_view;

	void on_plugin_toggled(const Glib::ustring& path);
	void on_plugin_changed(const Glib::RefPtr<Gtk::TreeSelection>& selection);

	void list_interfaces();

	bool on_delete_event(GdkEventAny*);
	void on_button_close();
	void on_hide();
	void on_show();
	void on_min_port_changed();
	void on_max_port_changed();

	void on_group_add();
	void on_group_remove();

	Glib::ustring hex_str(const Gdk::Color& color);

	bool is_separator(const Glib::RefPtr<Gtk::TreeModel>& model,
										const Gtk::TreeIter& iter);
public:
	SettingsWin(Gtk::Window *parent);
	virtual ~SettingsWin();
};

#endif /* SETTINGS_WIN_HH */
