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

#include <gtkmm/stock.h>
#include <gtkmm/box.h>
#include <gtkmm/notebook.h>
#include <gtkmm/table.h>
#include <gtkmm/expander.h>
#include <gtkmm/button.h>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/cellrenderertoggle.h>

#include "AlignedLabel.hh"
#include "SettingsWin.hh"
#include "linkage/Engine.hh"
#include "linkage/Utils.hh"

SettingsWin::SettingsWin(Gtk::Window* parent)
{
	set_title("Prefrences");
	set_transient_for(*parent);
	set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
	set_skip_taskbar_hint(true);
	set_border_width(5);

	Gtk::VBox* main_box = manage(new Gtk::VBox());
	main_box->set_spacing(5);
	Gtk::Notebook* notebook = manage(new Gtk::Notebook());
	main_box->pack_start(*notebook, true, true);

	Gtk::HBox* hbox = manage(new Gtk::HBox());
	Gtk::Button* button = manage(new Gtk::Button(Gtk::Stock::CLOSE));
	button->signal_clicked().connect(sigc::mem_fun(this, &SettingsWin::on_button_close));
	hbox->pack_end(*button, false, false);
	main_box->pack_start(*hbox, false, false);

	Gtk::VBox* interface_box = manage(new Gtk::VBox());
	AlignedFrame* frame = manage(new AlignedFrame("General"));
	Gtk::Table* table = manage(new Gtk::Table(3, 2));
	AlignedLabel* label = manage(new AlignedLabel("Update interval:"));
	table->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK);
	trunkate_names = manage(new Gtk::CheckButton("Trunkate names, width:"));
	table->attach(*trunkate_names, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK);
	auto_expand = manage(new Gtk::CheckButton("Auto expand details"));
	table->attach(*auto_expand, 0, 2, 2, 3, Gtk::FILL, Gtk::SHRINK);
	update_interval = manage(new AlignedSpinButton(1.0, 6.0));
	table->attach(*update_interval, 1, 2, 0, 1, Gtk::SHRINK, Gtk::SHRINK);
	name_width = manage(new AlignedSpinButton(1.0, 100.0));
	table->attach(*name_width, 1, 2, 1, 2, Gtk::SHRINK, Gtk::SHRINK);
	frame->add(*table);
	interface_box->pack_start(*frame, false, false);

	frame = manage(new AlignedFrame("State colors"));
	table = manage(new Gtk::Table(5, 4));
	label = manage(new AlignedLabel("Downloading:"));
	table->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Finished:"));
	table->attach(*label, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Seeding:"));
	table->attach(*label, 0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Announcing:"));
	table->attach(*label, 0, 1, 3, 4, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Stopped:"));
	table->attach(*label, 0, 1, 4, 5, Gtk::FILL, Gtk::SHRINK);
	color_downloading = manage(new Gtk::ColorButton());
	table->attach(*color_downloading, 1, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);
	color_finished = manage(new Gtk::ColorButton());
	table->attach(*color_finished, 1, 2, 1, 2, Gtk::FILL, Gtk::SHRINK);
	color_seeding = manage(new Gtk::ColorButton());
	table->attach(*color_seeding, 1, 2, 2, 3, Gtk::FILL, Gtk::SHRINK);
	color_announcing = manage(new Gtk::ColorButton());
	table->attach(*color_announcing, 1, 2, 3, 4, Gtk::FILL, Gtk::SHRINK);
	color_stopped = manage(new Gtk::ColorButton());
	table->attach(*color_stopped, 1, 2, 4, 5, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Queued:"));
	table->attach(*label, 2, 3, 0, 1, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Queued for checking:"));
	table->attach(*label, 2, 3, 1, 2, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Checking:"));
	table->attach(*label, 2, 3, 2, 3, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Allocating:"));
	table->attach(*label, 2, 3, 3, 4, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Error:"));
	table->attach(*label, 2, 3, 4, 5, Gtk::FILL, Gtk::SHRINK);
	color_queued = manage(new Gtk::ColorButton());
	table->attach(*color_queued, 3, 4, 0, 1, Gtk::FILL, Gtk::SHRINK);
	color_check_queue = manage(new Gtk::ColorButton());
	table->attach(*color_check_queue, 3, 4, 1, 2, Gtk::FILL, Gtk::SHRINK);
	color_checking = manage(new Gtk::ColorButton());
	table->attach(*color_checking, 3, 4, 2, 3, Gtk::FILL, Gtk::SHRINK);
	color_allocating = manage(new Gtk::ColorButton());
	table->attach(*color_allocating, 3, 4, 3, 4, Gtk::FILL, Gtk::SHRINK);
	color_error = manage(new Gtk::ColorButton());
	table->attach(*color_error, 3, 4, 4, 5, Gtk::FILL, Gtk::SHRINK);
	frame->add(*table);
	interface_box->pack_start(*frame, false, false);

	notebook->append_page(*interface_box, "Interface");

	Gtk::VBox* network_box = manage(new Gtk::VBox());
	frame = manage(new AlignedFrame("Connection"));
	table = manage(new Gtk::Table(6, 3));
	label = manage(new AlignedLabel("Interface:"));
	table->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Ports:"));
	table->attach(*label, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Tracker timeout:"));
	table->attach(*label, 0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK);
	enable_dht = manage(new Gtk::CheckButton("Enable DHT"));
	table->attach(*enable_dht, 0, 1, 3, 4, Gtk::FILL, Gtk::SHRINK);
	dht_fallback = manage(new Gtk::CheckButton("Only as fallback"));
	table->attach(*dht_fallback, 1, 2, 3, 4, Gtk::FILL, Gtk::SHRINK);
	enable_pex = manage(new Gtk::CheckButton("Enable PEX"));
	table->attach(*enable_pex, 0, 1, 4, 5, Gtk::FILL, Gtk::SHRINK);
	multiple_connections = manage(new Gtk::CheckButton("Allow multiple connections per IP"));
	table->attach(*multiple_connections, 0, 3, 5, 6, Gtk::FILL, Gtk::SHRINK);
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
	table->attach(*interfaces, 1, 3, 0, 1, Gtk::FILL, Gtk::SHRINK);
	min_port = manage(new AlignedSpinButton(1024.0, 65534.0));
	min_port->set_value(1024.0);
	table->attach(*min_port, 1, 2, 1, 2, Gtk::FILL, Gtk::SHRINK);
	max_port = manage(new AlignedSpinButton(1025.0, 65535.0));
	max_port->set_value(1025.0);
	table->attach(*max_port, 2, 3, 1, 2, Gtk::SHRINK, Gtk::SHRINK);
	tracker_timeout = manage(new AlignedSpinButton(5.0, 60.0));
	tracker_timeout->set_value(10.0);
	table->attach(*tracker_timeout, 1, 2, 2, 3, Gtk::FILL, Gtk::SHRINK);
	frame->add(*table);
	network_box->pack_start(*frame, false, false);

	frame = manage(new AlignedFrame("Session"));
	table = manage(new Gtk::Table(5, 2));
	label = manage(new AlignedLabel("Maximum connections:"));
	table->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Maximum uploads:"));
	table->attach(*label, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Maximum upload rate:"));
	table->attach(*label, 0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Maximum download rate:"));
	table->attach(*label, 0, 1, 3, 4, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Maximum active downloads:"));
	table->attach(*label, 0, 1, 4, 5, Gtk::FILL, Gtk::SHRINK);
	max_connections = manage(new AlignedSpinButton(0.0, 10000.0));
	table->attach(*max_connections, 1, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);
	max_uploads = manage(new AlignedSpinButton(0.0, 10000.0));
	table->attach(*max_uploads, 1, 2, 1, 2, Gtk::FILL, Gtk::SHRINK);
	up_rate = manage(new AlignedSpinButton(0.0, 10000.0));
	table->attach(*up_rate, 1, 2, 2, 3, Gtk::FILL, Gtk::SHRINK);
	down_rate = manage(new AlignedSpinButton(0.0, 10000.0));
	table->attach(*down_rate, 1, 2, 3, 4, Gtk::FILL, Gtk::SHRINK);
	max_active = manage(new AlignedSpinButton(0.0, 10000.0));
	table->attach(*max_active, 1, 2, 4, 5, Gtk::FILL, Gtk::SHRINK);
	frame->add(*table);
	network_box->pack_start(*frame, false, false);

	frame = manage(new AlignedFrame("Torrent"));
	table = manage(new Gtk::Table(3, 2));
	label = manage(new AlignedLabel("Maximum connections:"));
	table->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Maximum uploads:"));
	table->attach(*label, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Seed until ratio reaches:"));
	table->attach(*label, 0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK);
	max_torrent_connections = manage(new AlignedSpinButton(0.0, 10000.0));
	table->attach(*max_torrent_connections, 1, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);
	max_torrent_uploads = manage(new AlignedSpinButton(0.0, 10000.0));
	table->attach(*max_torrent_uploads, 1, 2, 1, 2, Gtk::FILL, Gtk::SHRINK);
	seed_ratio = manage(new AlignedSpinButton(0.0, 10000.0));
	table->attach(*seed_ratio, 1, 2, 2, 3, Gtk::FILL, Gtk::SHRINK);
	frame->add(*table);
	network_box->pack_start(*frame, false, false);

	Gtk::Expander* expander_proxy = manage(new Gtk::Expander());
	label = manage(new AlignedLabel());
	label->set_markup("<b>Proxy</b>");
	expander_proxy->set_label_widget(*label);
	Gtk::Alignment* alignment = manage(new Gtk::Alignment());
	alignment->property_left_padding() = 12;
	table = manage(new Gtk::Table(3, 3));
	label = manage(new AlignedLabel("Proxy:"));
	table->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK, 10);
	label = manage(new AlignedLabel("Username:"));
	table->attach(*label, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK, 10);
	label = manage(new AlignedLabel("Password:"));
	table->attach(*label, 0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK, 10);
	proxy_ip = manage(new Gtk::Entry());
	table->attach(*proxy_ip, 1, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);
	proxy_port = manage(new AlignedSpinButton(0.0, 65535.0));
	proxy_port->set_value(8080.0);
	table->attach(*proxy_port, 2, 3, 0, 1, Gtk::FILL, Gtk::SHRINK);
	proxy_user = manage(new Gtk::Entry());
	table->attach(*proxy_user, 1, 2, 1, 2, Gtk::FILL, Gtk::SHRINK);
	proxy_pass = manage(new Gtk::Entry());
	proxy_pass->property_visibility() = false;
	table->attach(*proxy_pass, 1, 2, 2, 3, Gtk::FILL, Gtk::SHRINK);
	alignment->add(*table);
	expander_proxy->add(*alignment);
	network_box->pack_start(*expander_proxy, false, false);

	notebook->append_page(*network_box, "Network");

	Gtk::VBox* files_box = manage(new Gtk::VBox());
	frame = manage(new AlignedFrame("Folders"));
	table = manage(new Gtk::Table(2, 2));
	default_path = manage(new Gtk::CheckButton("Use default path:"));
	default_path->set_alignment(0, 0.5);
	table->attach(*default_path, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK);
	move_finished = manage(new Gtk::CheckButton("Move finished:"));
	move_finished->set_alignment(0, 0.5);
	table->attach(*move_finished, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK);
	button_default_path = manage(new Gtk::FileChooserButton("Select folder",
																	Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER));
	table->attach(*button_default_path, 1, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);
	button_move_finished = manage(new Gtk::FileChooserButton("Select folder",
																		Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER));
	table->attach(*button_move_finished, 1, 2, 1, 2, Gtk::FILL, Gtk::SHRINK);
	frame->add(*table);
	files_box->pack_start(*frame, false, false);

	frame = manage(new AlignedFrame("Files"));
	table = manage(new Gtk::Table(2, 2));
	allocate = manage(new Gtk::CheckButton("Allocate disk space"));
	table->attach(*allocate, 0, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Maximum open files:"));
	table->attach(*label, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK);
	max_open = manage(new AlignedSpinButton(1.0, 100000.0));
	table->attach(*max_open, 1, 2, 1, 2, Gtk::FILL, Gtk::SHRINK);
	frame->add(*table);
	files_box->pack_start(*frame, false, false);

	notebook->append_page(*files_box, "Files & Folders");

	Gtk::VBox* plugins_box = manage(new Gtk::VBox());
	model_plugins = Gtk::ListStore::create(plugin_columns);
	Gtk::TreeView* treeview_plugins = manage(new Gtk::TreeView());
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
	plugins_box->pack_start(*treeview_plugins, true, true);

	frame = manage(new AlignedFrame("Plugin"));
	table = manage(new Gtk::Table(3, 2));
	label = manage(new AlignedLabel("Author:"));
	table->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Website:"));
	table->attach(*label, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("File:"));
	table->attach(*label, 0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK);
	label_author = manage(new AlignedLabel());
	table->attach(*label_author, 1, 2, 0, 1, Gtk::FILL, Gtk::SHRINK);
	label_website = manage(new AlignedLabel());
	table->attach(*label_website, 1, 2, 1, 2, Gtk::FILL, Gtk::SHRINK);
	label_file = manage(new AlignedLabel());
	table->attach(*label_file, 1, 2, 2, 3, Gtk::FILL, Gtk::SHRINK);
	frame->add(*table);
	plugins_box->pack_start(*frame, false, false);

	frame_options = manage(new AlignedFrame("Options"));
	plugins_box->pack_start(*frame_options, false, false);

	notebook->append_page(*plugins_box, "Plugins");

	Gtk::HBox* groups_box = manage(new Gtk::HBox());
	groups_view = manage(new GroupView());
	groups_box->pack_start(*groups_view, false, false);
	group_add = manage(new Gtk::Button(Gtk::Stock::ADD));
	group_add->signal_clicked().connect(sigc::mem_fun(*this, &SettingsWin::on_group_add));
	group_remove = manage(new Gtk::Button(Gtk::Stock::REMOVE));
	group_remove->unset_flags(Gtk::CAN_FOCUS);
	group_remove->signal_clicked().connect(sigc::mem_fun(*this, &SettingsWin::on_group_remove));
	Gtk::VBox* groups_vbox = manage(new Gtk::VBox());
	groups_vbox->pack_start(*group_add, false, false);
	groups_vbox->pack_start(*group_remove, false, false);
	groups_box->pack_end(*groups_vbox);

	notebook->append_page(*groups_box, "Groups");

	add(*main_box);

	min_port->signal_value_changed().connect(sigc::mem_fun(*this, &SettingsWin::on_min_port_changed));
	max_port->signal_value_changed().connect(sigc::mem_fun(*this, &SettingsWin::on_max_port_changed));

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

void SettingsWin::on_group_add()
{
	Glib::ustring number = str(groups_view->get_children().size());
	GroupRow* row = new GroupRow("Group " + number);
	groups_view->append(row);
	if (groups_view->children().size() > 1)
		group_remove->set_sensitive(true);
}

void SettingsWin::on_group_remove()
{
	GroupRow* row = groups_view->get_selected();
	if (row)
		groups_view->erase(row);
	if (groups_view->children().size() == 1)
		group_remove->set_sensitive(false);
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

	Gtk::TreeRow row = *iter;
	label_author->set_text(row[plugin_columns.author]);
	label_website->set_text(row[plugin_columns.website]);
	label_file->set_text(row[plugin_columns.file]);
	WeakPtr<Plugin> plugin = Engine::get_plugin_manager()->get_plugin(row[plugin_columns.name]);
	if (plugin)
	{
		/* FIXME: save previous plugin settings to settings manager */
		frame_options->remove();
		Gtk::Widget* widget = plugin->get_config_widget();
		if (widget)
			frame_options->add(*widget);
		/* FIXME: load plugin settings from settings manager */
	}
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
		sm->set("Network", "Interface", Glib::ustring(""));
	else
		sm->set("Network", "Interface", interfaces->get_active_text());
	sm->set("Network", "MinPort", (int)min_port->get_value());
	sm->set("Network", "MaxPort", (int)max_port->get_value());
	sm->set("Network", "TrackerTimeout", (int)tracker_timeout->get_value());
	sm->set("Network", "UseDHT", enable_dht->get_active());
	sm->set("Network", "DHTFallback", dht_fallback->get_active());
	sm->set("Network", "UsePEX", enable_pex->get_active());
	sm->set("Network", "MultipleConnectionsPerIP", multiple_connections->get_active());

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
	
	uploads = (int)max_torrent_uploads->get_value();
	if (uploads == 0)
		uploads = -1;
	sm->set("Network", "MaxTorrentUploads", uploads);
	connections = (int)max_torrent_connections->get_value();
	if (connections == 1)
		connections = 2;
	else if (connections == 0)
		connections = -1;
	sm->set("Network", "MaxTorrentConnections", connections);
	sm->set("Network", "SeedRatio", str(seed_ratio->get_value(), 1));

	sm->set("Network", "ProxyIp", proxy_ip->get_text());
	sm->set("Network", "ProxyPort", (int)proxy_port->get_value());
	sm->set("Network", "ProxyLogin", proxy_user->get_text());
	sm->set("Network", "ProxyPass", proxy_pass->get_text());
	/* UI */
	sm->set("UI", "Interval", (int)update_interval->get_value());
	sm->set("UI", "AutoExpand", auto_expand->get_active());
	sm->set("UI", "TrunkateNames", trunkate_names->get_active());
	sm->set("UI", "MaxNameWidth", (int)name_width->get_value());
	sm->set("UI", "ColorDownloading", hex_str(color_downloading->get_color()));
	sm->set("UI", "ColorFinished", hex_str(color_finished->get_color()));
	sm->set("UI", "ColorSeeding", hex_str(color_seeding->get_color()));
	sm->set("UI", "ColorAnnouncing", hex_str(color_announcing->get_color()));
	sm->set("UI", "ColorStopped", hex_str(color_stopped->get_color()));
	sm->set("UI", "ColorQueued", hex_str(color_queued->get_color()));
	sm->set("UI", "ColorCheckQueue", hex_str(color_check_queue->get_color()));
	sm->set("UI", "ColorChecking", hex_str(color_checking->get_color()));
	sm->set("UI", "ColorAllocating", hex_str(color_allocating->get_color()));
	sm->set("UI", "ColorError", hex_str(color_error->get_color()));
	/* Plugins */
	Gtk::TreeNodeChildren children = model_plugins->children();
	std::list<Glib::ustring> plugins;
	for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
	{
		Gtk::TreeRow row = *iter;
		if (row[plugin_columns.load])
			plugins.push_back(row[plugin_columns.name]);
	}
	sm->set("UI", "Plugins", UStringArray(plugins));
	/* Files */
	sm->set("Files", "UseDefaultPath", default_path->get_active());
	sm->set("Files", "DefaultPath", button_default_path->get_filename());
	sm->set("Files", "MoveFinished", move_finished->get_active());
	sm->set("Files", "FinishedPath", button_move_finished->get_filename());
	sm->set("Files", "Allocate", allocate->get_active());
	sm->set("Files", "MaxOpen", (int)max_open->get_value());
	/* Groups */
	sm->remove_group("Groups");
	std::list<GroupRow*> rows = groups_view->children();
	for (std::list<GroupRow*>::iterator iter = rows.begin();
				iter != rows.end(); ++iter)
	{
		GroupRow* row = *iter;
		Glib::ustring name = Glib::Markup::escape_text(row->get_name());

		std::list<Glib::ustring> info;
		std::list<Group::Filter> filters = row->get_filters();

		for (std::list<Group::Filter>::iterator iter = filters.begin();
					iter != filters.end(); ++iter)
		{
			Group::Filter f = *iter;
			info.push_back(f.filter);
			info.push_back(str(f.eval));
			info.push_back(str(f.tag));
		}

		sm->set("Groups", name, UStringArray(info));
	}

	Engine::get_settings_manager()->update();
}

void SettingsWin::on_show()
{
	Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();

	/* Network */
	Glib::ustring interface = sm->get_string("Network", "Interface");
	min_port->set_value((double)sm->get_int("Network", "MinPort"));
	max_port->set_value((double)sm->get_int("Network", "MaxPort"));
	tracker_timeout->set_value((double)sm->get_int("Network", "TrackerTimeout"));
	enable_dht->set_active(sm->get_bool("Network", "UseDHT"));
	dht_fallback->set_active(sm->get_bool("Network", "DHTFallback"));
	enable_pex->set_active(sm->get_bool("Network", "UsePEX"));

	up_rate->set_value((double)sm->get_int("Network", "MaxUpRate"));
	down_rate->set_value((double)sm->get_int("Network", "MaxDownRate"));
	max_uploads->set_value((double)sm->get_int("Network", "MaxUploads"));
	max_connections->set_value((double)sm->get_int("Network", "MaxConnections"));
	max_active->set_value((double)sm->get_int("Network", "MaxActive"));

	max_torrent_uploads->set_value((double)sm->get_int("Network", "MaxTorrentUploads"));
	max_torrent_connections->set_value((double)sm->get_int("Network", "MaxTorrentConnections"));
	double ratio;
	std::istringstream(sm->get_string("Network", "SeedRatio")) >> ratio;
	seed_ratio->set_value(ratio);

	proxy_ip->set_text(sm->get_string("Network", "ProxyIp"));
	proxy_port->set_value((double)sm->get_int("Network", "ProxyPort"));
	proxy_user->set_text(sm->get_string("Network", "ProxyLogin"));
	proxy_pass->set_text(sm->get_string("Network", "ProxyPass"));
	if (!interface.empty())
		interfaces->set_active_text(interface);
	else
		interfaces->set_active(0);
	/* UI */
	update_interval->set_value((double)sm->get_int("UI", "Interval"));
	auto_expand->set_active(sm->get_bool("UI", "AutoExpand"));
	trunkate_names->set_active(sm->get_bool("UI", "TrunkateNames"));
	name_width->set_value((double)sm->get_int("UI", "MaxNameWidth"));
	Glib::RefPtr<Gdk::Colormap> colormap = get_screen()->get_default_colormap();
	Gdk::Color color = Gdk::Color(sm->get_string("UI", "ColorDownloading"));
	colormap->alloc_color(color);
	color_downloading->set_color(color);
	colormap->free_color(color);
	color = Gdk::Color(sm->get_string("UI", "ColorFinished"));
	colormap->alloc_color(color);
	color_finished->set_color(color);
	colormap->free_color(color);
	color = Gdk::Color(sm->get_string("UI", "ColorSeeding"));
	colormap->alloc_color(color);
	color_seeding->set_color(color);
	colormap->free_color(color);
	color = Gdk::Color(sm->get_string("UI", "ColorAnnouncing"));
	colormap->alloc_color(color);
	color_announcing->set_color(color);
	colormap->free_color(color);
	color = Gdk::Color(sm->get_string("UI", "ColorStopped"));
	colormap->alloc_color(color);
	color_stopped->set_color(color);
	colormap->free_color(color);
	color = Gdk::Color(sm->get_string("UI", "ColorQueued"));
	colormap->alloc_color(color);
	color_queued->set_color(color);
	colormap->free_color(color);
	color = Gdk::Color(sm->get_string("UI", "ColorCheckQueue"));
	colormap->alloc_color(color);
	color_check_queue->set_color(color);
	colormap->free_color(color);
	color = Gdk::Color(sm->get_string("UI", "ColorChecking"));
	colormap->alloc_color(color);
	color_checking->set_color(color);
	colormap->free_color(color);
	color = Gdk::Color(sm->get_string("UI", "ColorAllocating"));
	colormap->alloc_color(color);
	color_allocating->set_color(color);
	colormap->free_color(color);
	color = Gdk::Color(sm->get_string("UI", "ColorError"));
	colormap->alloc_color(color);
	color_error->set_color(color);
	colormap->free_color(color);
	/* Files */
	default_path->set_active(sm->get_bool("Files", "UseDefaultPath"));
	button_default_path->set_filename(sm->get_string("Files", "DefaultPath"));
	move_finished->set_active(sm->get_bool("Files", "MoveFinished"));
	button_move_finished->set_filename(sm->get_string("Files", "FinishedPath"));
	allocate->set_active(sm->get_bool("Files", "Allocate"));
	max_open->set_value((double)sm->get_int("Files", "MaxOpen"));
	/* Plugins */
	if (model_plugins->children().empty())
	{
		std::list<PluginInfo> plugins = Engine::get_plugin_manager()->list_plugins();

		for (std::list<PluginInfo>::iterator iter = plugins.begin();
				 iter != plugins.end(); ++iter)
		{
			PluginInfo& info = *iter;
			Gtk::TreeRow row = *(model_plugins->append());
			row[plugin_columns.name] = info.get_name();
			row[plugin_columns.description] = info.get_description();
			row[plugin_columns.author] = info.get_author();
			row[plugin_columns.website] = info.get_website();
			row[plugin_columns.file] = info.get_file();
			row[plugin_columns.load] = info.get_loaded();
		}
	}
	/* Groups */
	if (groups_view->children().empty())
	{
		std::list<Glib::ustring> groups = sm->get_keys("Groups");
		for (std::list<Glib::ustring>::iterator iter = groups.begin();
					iter != groups.end(); ++iter)
		{
			std::vector<Glib::ustring> info = sm->get_string_list("Groups", *iter);

			if ((info.size() % 3) != 0)
				continue;

			std::list<Group::Filter> filters;
			for (int i = 0; i < info.size(); i+=3)
			{
				Glib::ustring filter = info[i];
				Group::EvalType eval = Group::EvalType(std::atoi(info[i+1].c_str()));
				Group::TagType tag = Group::TagType(std::atoi(info[i+2].c_str()));
				filters.push_back(Group::Filter(filter, tag, eval));
			}
			GroupRow* row = new GroupRow(*iter, filters);
			groups_view->append(row);
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
