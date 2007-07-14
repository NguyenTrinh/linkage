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

#include <gtkmm/stock.h>
#include <gtkmm/box.h>
#include <gtkmm/notebook.h>
#include <gtkmm/table.h>
#include <gtkmm/expander.h>
#include <gtkmm/button.h>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/cellrenderertoggle.h>
#include <libtorrent/entry.hpp>

#include "SettingsWin.hh"
#include "linkage/Engine.hh"
#include "linkage/Utils.hh"
#include "linkage/SettingsManager.hh"

SettingsWin::SettingsWin(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::Window(cobject),
		glade_xml(refGlade)
{
	glade_xml->get_widget("update_interval", update_interval);
	glade_xml->get_widget("name_width", name_width);
	glade_xml->get_widget("auto_expand", auto_expand);
	glade_xml->get_widget("trunkate_names", trunkate_names);
	glade_xml->get_widget("color_downloading", color_downloading);
	glade_xml->get_widget("color_seeding", color_seeding);
	glade_xml->get_widget("color_queued", color_queued);
	glade_xml->get_widget("color_error", color_error);
	glade_xml->get_widget("color_check_queue", color_check_queue);
	glade_xml->get_widget("color_checking", color_checking);
	glade_xml->get_widget("color_finished", color_finished);
	glade_xml->get_widget("color_allocating", color_allocating);
	glade_xml->get_widget("color_announcing", color_announcing);
	glade_xml->get_widget("color_stopped", color_stopped);

	glade_xml->get_widget("min_port", min_port);
	glade_xml->get_widget("max_port", max_port);
	glade_xml->get_widget("tracker_timeout", tracker_timeout);
	glade_xml->get_widget("enable_dht", enable_dht);
	glade_xml->get_widget("dht_fallback", dht_fallback);
	glade_xml->get_widget("enable_pex", enable_pex);
	glade_xml->get_widget("multiple_connections", multiple_connections);

	glade_xml->get_widget("max_connections", max_connections);
	glade_xml->get_widget("max_uploads", max_uploads);
	glade_xml->get_widget("max_active", max_active);
	glade_xml->get_widget("up_rate", up_rate);
	glade_xml->get_widget("down_rate", down_rate);

	glade_xml->get_widget("max_torrent_connections", max_torrent_connections);
	glade_xml->get_widget("max_torrent_uploads", max_torrent_uploads);
	glade_xml->get_widget("seed_ratio", seed_ratio);

	glade_xml->get_widget("proxy_port", proxy_port);
	glade_xml->get_widget("proxy_ip", proxy_ip);
	glade_xml->get_widget("proxy_user", proxy_user);
	glade_xml->get_widget("proxy_pass", proxy_pass);

	glade_xml->get_widget("move_finished", move_finished);
	glade_xml->get_widget("allocate", allocate);
	glade_xml->get_widget("use_default_path", default_path);
	glade_xml->get_widget("button_default_path", button_default_path);
	glade_xml->get_widget("button_move_finished", button_move_finished);
	glade_xml->get_widget("max_open", max_open);

	glade_xml->get_widget("treeview_plugins", treeview_plugins);
	glade_xml->get_widget("plugin_about", about_plugin);
	glade_xml->get_widget("plugin_configure", configure_plugin);
	/* FIXME: about and configure support for plugins */

	// connect callbacks
	Gtk::Button* button;
	glade_xml->get_widget("button_close", button);
	button->signal_clicked().connect(sigc::mem_fun(this, &SettingsWin::on_button_close));

	// populate the interfaces combobox
	// cant be done with glade because ComboBoxText is gtkmm only
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
	Gtk::Table* table;
	glade_xml->get_widget("table1", table);
	table->attach(*interfaces, 1, 3, 0, 1, Gtk::FILL, Gtk::SHRINK);	

	// setup the plugins listview
	model_plugins = Gtk::ListStore::create(plugin_columns);
	treeview_plugins->get_selection()->signal_changed().connect(sigc::bind(
		sigc::mem_fun(this, &SettingsWin::on_plugin_changed),
		treeview_plugins->get_selection()));
	treeview_plugins->set_model(model_plugins);
	Gtk::CellRendererToggle* trender = new Gtk::CellRendererToggle();
	trender->signal_toggled().connect(sigc::mem_fun(this, &SettingsWin::on_plugin_toggled));
	treeview_plugins->append_column("Load", *manage(trender));
	treeview_plugins->get_column(0)->add_attribute(*trender, "active", 0);
	treeview_plugins->append_column("Name", plugin_columns.name);
	treeview_plugins->append_column("Description", plugin_columns.description);
	for(unsigned int i = 0; i < 3; i++)
	{
		Gtk::TreeView::Column* column = treeview_plugins->get_column(i);
		column->set_sort_column_id(i);
		column->set_resizable(true);
	}

	min_port->signal_value_changed().connect(sigc::mem_fun(*this, &SettingsWin::on_min_port_changed));
	max_port->signal_value_changed().connect(sigc::mem_fun(*this, &SettingsWin::on_max_port_changed));

	show_all_children();
}

SettingsWin::~SettingsWin()
{
	delete interfaces;
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
	row[plugin_columns.load] = !row[plugin_columns.load];
}

void SettingsWin::on_plugin_changed(const Glib::RefPtr<Gtk::TreeSelection>& selection)
{
	Gtk::TreeIter iter = selection->get_selected();
	if (!iter)
		return;

	/*Gtk::TreeRow row = *iter;
	label_author->set_text(row[plugin_columns.author]);
	label_website->set_text(row[plugin_columns.website]);
	label_file->set_text(row[plugin_columns.file]);
	WeakPtr<Plugin> plugin = Engine::get_plugin_manager()->get_plugin(row[plugin_columns.name]);
	if (plugin)
	{
		FIXME: save previous plugin settings to settings manager
		frame_options->remove();
		Gtk::Widget* widget = plugin->get_config_widget();
		if (widget)
			frame_options->add(*widget);
		 FIXME: load plugin settings from settings manager
	}*/
}

Glib::ustring SettingsWin::hex_str(const Gdk::Color& color)
{
	char s[8];
	s[0] = '#';

	sprintf(&s[1], "%.2X", color.get_red() / 256);
	sprintf(&s[3], "%.2X", color.get_green() / 256);
	sprintf(&s[5], "%.2X", color.get_blue() / 256);

	return s;
}

void SettingsWin::on_hide()
{
	Gtk::Window::on_hide();

	Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();

	/* Network */
	if (interfaces->get_active_row_number() <= 0)
		sm->set("network/interface", Glib::ustring(""));
	else
			sm->set("network/interface", interfaces->get_active_text());
	sm->set("network/min_port", (int)min_port->get_value());
	sm->set("network/max_port", (int)max_port->get_value());
	sm->set("network/tracker_timeout", (int)tracker_timeout->get_value());
	sm->set("network/use_dht", enable_dht->get_active());
	sm->set("network/dht_fallback", dht_fallback->get_active());
	sm->set("network/use_pex", enable_pex->get_active());
	sm->set("network/multiple_connections_per_ip", multiple_connections->get_active());

	sm->set("network/max_up_rate", (int)up_rate->get_value());
	sm->set("network/max_down_rate", (int)down_rate->get_value());
	int uploads = (int)max_uploads->get_value();
	if (uploads == 0)
		uploads = -1;
	sm->set("network/max_uploads", uploads);
	int connections = (int)max_connections->get_value();
	if (connections == 1)
		connections = 2;
	else if (connections == 0)
		connections = -1;
	sm->set("network/max_connections", connections);
	sm->set("network/max_active", (int)max_active->get_value());
	
	uploads = (int)max_torrent_uploads->get_value();
	if (uploads == 0)
		uploads = -1;
	sm->set("network/max_torrent_uploads", uploads);
	connections = (int)max_torrent_connections->get_value();
	if (connections == 1)
		connections = 2;
	else if (connections == 0)
		connections = -1;
	sm->set("network/max_torrent_connections", connections);
	sm->set("network/seed_ratio", seed_ratio->get_value());

	sm->set("network/proxy/ip", proxy_ip->get_text());
	sm->set("network/proxy/port", (int)proxy_port->get_value());
	sm->set("network/proxy/login", proxy_user->get_text());
	sm->set("network/proxy/pass", proxy_pass->get_text());
	/* UI */
	sm->set("ui/interval", (int)update_interval->get_value());
	sm->set("ui/auto_expand", auto_expand->get_active());
	sm->set("ui/torrent_view/trunkate_names", trunkate_names->get_active());
	sm->set("ui/torrent_view/max_name_width", (int)name_width->get_value());
	sm->set("ui/colors/downloading", hex_str(color_downloading->get_color()));
	sm->set("ui/colors/finished", hex_str(color_finished->get_color()));
	sm->set("ui/colors/seeding", hex_str(color_seeding->get_color()));
	sm->set("ui/colors/announcing", hex_str(color_announcing->get_color()));
	sm->set("ui/colors/stopped", hex_str(color_stopped->get_color()));
	sm->set("ui/colors/queued", hex_str(color_queued->get_color()));
	sm->set("ui/colors/check_queue", hex_str(color_check_queue->get_color()));
	sm->set("ui/colors/checking", hex_str(color_checking->get_color()));
	sm->set("ui/colors/allocating", hex_str(color_allocating->get_color()));
	sm->set("ui/colors/error", hex_str(color_error->get_color()));
	/* Plugins */
	Gtk::TreeNodeChildren children = model_plugins->children();
	std::list<Glib::ustring> plugins;
	for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
	{
		Gtk::TreeRow row = *iter;
		if (row[plugin_columns.load])
			plugins.push_back(row[plugin_columns.name]);
	}
	sm->set("ui/plugins", Glib::SListHandle<Glib::ustring>(plugins));
	/* Files */
	sm->set("files/use_default_path", default_path->get_active());
	sm->set("files/default_path", button_default_path->get_filename());
	sm->set("files/move_finished", move_finished->get_active());
	sm->set("files/finished_path", button_move_finished->get_filename());
	sm->set("files/allocate", allocate->get_active());
	sm->set("files/max_open", (int)max_open->get_value());

	Engine::get_settings_manager()->update();
}

void SettingsWin::on_show()
{
	Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();

	/* Network */
	Glib::ustring interface = sm->get_string("network/interface");
	min_port->set_value((double)sm->get_int("network/min_port"));
	max_port->set_value((double)sm->get_int("network/max_port"));
	tracker_timeout->set_value((double)sm->get_int("network/tracker_timeout"));
	enable_dht->set_active(sm->get_bool("network/use_dht"));
	dht_fallback->set_active(sm->get_bool("network/dht_fallback"));
	enable_pex->set_active(sm->get_bool("network/use_pex"));

	up_rate->set_value((double)sm->get_int("network/max_up_rate"));
	down_rate->set_value((double)sm->get_int("network/max_down_rate"));
	max_uploads->set_value((double)sm->get_int("network/max_uploads"));
	max_connections->set_value((double)sm->get_int("network/max_connections"));
	max_active->set_value((double)sm->get_int("network/max_active"));

	max_torrent_uploads->set_value((double)sm->get_int("network/max_torrent_uploads"));
	max_torrent_connections->set_value((double)sm->get_int("network/max_torrent_connections"));
	seed_ratio->set_value(sm->get_float("network/seed_ratio"));

	proxy_ip->set_text(sm->get_string("network/proxy/ip"));
	proxy_port->set_value((double)sm->get_int("network/proxy/port"));
	proxy_user->set_text(sm->get_string("network/proxy/login"));
	proxy_pass->set_text(sm->get_string("network/proxy/pass"));
	if (!interface.empty())
		interfaces->set_active_text(interface);
	else
		interfaces->set_active(0);
	/* UI */
	update_interval->set_value((double)sm->get_int("ui/interval"));
	auto_expand->set_active(sm->get_bool("ui/auto_expand"));
	trunkate_names->set_active(sm->get_bool("ui/torrent_view/trunkate_names"));
	name_width->set_value((double)sm->get_int("ui/torrent_view/max_name_width"));
	Glib::RefPtr<Gdk::Colormap> colormap = get_screen()->get_default_colormap();
	Gdk::Color color = Gdk::Color(sm->get_string("ui/colors/downloading"));
	colormap->alloc_color(color);
	color_downloading->set_color(color);
	colormap->free_color(color);
	color = Gdk::Color(sm->get_string("ui/colors/finished"));
	colormap->alloc_color(color);
	color_finished->set_color(color);
	colormap->free_color(color);
	color = Gdk::Color(sm->get_string("ui/colors/seeding"));
	colormap->alloc_color(color);
	color_seeding->set_color(color);
	colormap->free_color(color);
	color = Gdk::Color(sm->get_string("ui/colors/announcing"));
	colormap->alloc_color(color);
	color_announcing->set_color(color);
	colormap->free_color(color);
	color = Gdk::Color(sm->get_string("ui/colors/stopped"));
	colormap->alloc_color(color);
	color_stopped->set_color(color);
	colormap->free_color(color);
	color = Gdk::Color(sm->get_string("ui/colors/queued"));
	colormap->alloc_color(color);
	color_queued->set_color(color);
	colormap->free_color(color);
	color = Gdk::Color(sm->get_string("ui/colors/check_queue"));
	colormap->alloc_color(color);
	color_check_queue->set_color(color);
	colormap->free_color(color);
	color = Gdk::Color(sm->get_string("ui/colors/checking"));
	colormap->alloc_color(color);
	color_checking->set_color(color);
	colormap->free_color(color);
	color = Gdk::Color(sm->get_string("ui/colors/allocating"));
	colormap->alloc_color(color);
	color_allocating->set_color(color);
	colormap->free_color(color);
	color = Gdk::Color(sm->get_string("ui/colors/error"));
	colormap->alloc_color(color);
	color_error->set_color(color);
	colormap->free_color(color);
	/* Files */
	default_path->set_active(sm->get_bool("files/use_default_path"));
	button_default_path->set_filename(sm->get_string("files/default_path"));
	move_finished->set_active(sm->get_bool("files/move_finished"));
	button_move_finished->set_filename(sm->get_string("files/finished_path"));
	allocate->set_active(sm->get_bool("files/allocate"));
	max_open->set_value((double)sm->get_int("files/max_open"));
	/* Plugins */
	if (model_plugins->children().empty())
	{
		PluginManager::PluginInfoList plugins = Engine::get_plugin_manager()->list_plugins();

		for (PluginManager::PluginInfoList::iterator iter = plugins.begin();
				 iter != plugins.end(); ++iter)
		{
			PluginManager::PluginInfo info = *iter;
			Gtk::TreeRow row = *(model_plugins->append());
			row[plugin_columns.name] = info.name;
			row[plugin_columns.description] = info.description;
			row[plugin_columns.author] = info.author;
			row[plugin_columns.website] = info.website;
			row[plugin_columns.file] = info.file;
			row[plugin_columns.load] = info.loaded;
		}
	}

	Gtk::Window::on_show();
}

bool SettingsWin::is_separator(const Glib::RefPtr<Gtk::TreeModel>& model,
									 							const Gtk::TreeIter& iter)
{
	Gtk::TreeRow row = *iter;
	Glib::ustring data;
	row.get_value(0, data);
	return (data == "-");
}

