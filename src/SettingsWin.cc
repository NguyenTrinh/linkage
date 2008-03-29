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

#include <gtkmm/aboutdialog.h>
#include <gtkmm/stock.h>
#include <gtkmm/box.h>
#include <gtkmm/notebook.h>
#include <gtkmm/table.h>
#include <gtkmm/expander.h>
#include <gtkmm/button.h>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/cellrenderertoggle.h>
#include <glibmm/i18n.h>

#include "SettingsWin.hh"

#include "linkage/Engine.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/PluginManager.hh"
#include "linkage/Utils.hh"

using namespace Linkage;

void SettingsWin::ComboBoxTextGlade::init(const Glib::ustring& key_)
{
	if (key.empty())
		set_key(key_);
	g_return_if_fail(!key.empty());

	set_active_text(Engine::get_settings_manager()->get_string(key));

	signal_changed().connect(
		sigc::mem_fun(this, &SettingsWidget<Glib::ustring>::on_changed));
}

SettingsWin::SettingsWin(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::Window(cobject),
		glade_xml(refGlade)
{
	glade_xml->get_widget_derived("update_interval", update_interval);
	glade_xml->get_widget_derived("name_width", name_width);
	glade_xml->get_widget_derived("auto_expand", auto_expand);
	glade_xml->get_widget_derived("trunkate_names", trunkate_names);
	glade_xml->get_widget_derived("allow_linebreaks", allow_linebreaks);
	glade_xml->get_widget_derived("color_downloading", color_downloading);
	glade_xml->get_widget_derived("color_seeding", color_seeding);
	glade_xml->get_widget_derived("color_queued", color_queued);
	glade_xml->get_widget_derived("color_error", color_error);
	glade_xml->get_widget_derived("color_check_queue", color_check_queue);
	glade_xml->get_widget_derived("color_checking", color_checking);
	glade_xml->get_widget_derived("color_finished", color_finished);
	glade_xml->get_widget_derived("color_allocating", color_allocating);
	glade_xml->get_widget_derived("color_announcing", color_announcing);
	glade_xml->get_widget_derived("color_stopped", color_stopped);

	glade_xml->get_widget_derived("interfaces", interfaces);
	glade_xml->get_widget_derived("port", port);
	glade_xml->get_widget_derived("enc_policy", enc_policy);
	glade_xml->get_widget_derived("enc_level", enc_level);
	glade_xml->get_widget_derived("enable_dht", enable_dht);
	glade_xml->get_widget_derived("dht_fallback", dht_fallback);
	glade_xml->get_widget_derived("enable_pex", enable_pex);
	glade_xml->get_widget_derived("multiple_connections", multiple_connections);

	glade_xml->get_widget_derived("max_connections", max_connections);
	glade_xml->get_widget_derived("max_uploads", max_uploads);
	glade_xml->get_widget_derived("up_rate", up_rate);
	glade_xml->get_widget_derived("down_rate", down_rate);

	glade_xml->get_widget_derived("max_torrent_connections", max_torrent_connections);
	glade_xml->get_widget_derived("max_torrent_uploads", max_torrent_uploads);

	glade_xml->get_widget_derived("proxy_port", proxy_port);
	glade_xml->get_widget_derived("proxy_host", proxy_host);
	glade_xml->get_widget_derived("proxy_username", proxy_username);
	glade_xml->get_widget_derived("proxy_password", proxy_password);
	glade_xml->get_widget_derived("proxy_type", proxy_type);

	glade_xml->get_widget_derived("max_active", max_active);
	glade_xml->get_widget_derived("desired_ratio", desired_ratio);
	glade_xml->get_widget_derived("stop_ratio", stop_ratio);
	glade_xml->get_widget_derived("lazy_bitfields", lazy_bitfields);

	glade_xml->get_widget_derived("move_finished", move_finished);
	glade_xml->get_widget_derived("allocate", allocate);
	glade_xml->get_widget_derived("prio_firstlast", prio_firstlast);
	glade_xml->get_widget_derived("use_default_path", default_path);
	glade_xml->get_widget_derived("button_default_path", button_default_path);
	glade_xml->get_widget_derived("button_move_finished", button_move_finished);
	glade_xml->get_widget_derived("max_open", max_open);

	glade_xml->get_widget("treeview_plugins", treeview_plugins);
	glade_xml->get_widget("plugin_about", about_plugin);
	about_plugin->set_sensitive(false);
	about_plugin->signal_clicked().connect(sigc::mem_fun(this, &SettingsWin::on_about_plugin));
	glade_xml->get_widget("plugin_configure", configure_plugin);
	configure_plugin->signal_clicked().connect(sigc::mem_fun(this, &SettingsWin::on_configure_plugin));
	configure_plugin->set_sensitive(false);

	// connect callbacks
	Gtk::Button* button;
	glade_xml->get_widget("button_close", button);
	button->signal_clicked().connect(sigc::mem_fun(this, &SettingsWin::on_button_close));

	// populate the comboboxes
	interfaces->set_row_separator_func(sigc::mem_fun(this, &SettingsWin::is_separator));
	interfaces->append_text(_("None specified"));
	interfaces->append_text("-");
	std::list<Glib::ustring> devices = get_interfaces();
	for (std::list<Glib::ustring>::iterator iter = devices.begin();
		iter != devices.end(); ++iter)
	{
		interfaces->append_text(*iter);
	}
	enc_policy->append_pair(_("Forced"), "POLICY_FORCED");
	enc_policy->append_pair(_("Enabled"), "POLICY_ENABLED");
	enc_policy->append_pair(_("Disabled"), "POLICY_DISABLED");
	enc_level->append_pair(_("Plain text"), "LEVEL_PLAINTEXT");
	enc_level->append_pair(_("RC4"), "LEVEL_RC4");
	enc_level->append_pair(_("Both"), "LEVEL_BOTH");
	proxy_type->append_pair(_("None"), "PROXY_NONE");
	proxy_type->append_pair(_("SOCKS4"), "PROXY_SOCKS4");
	proxy_type->append_pair(_("SOCKS5"), "PROXY_SOCKS5");
	proxy_type->append_pair(_("SOCKS5 with password"), "PROXY_SOCKS5_PW");
	proxy_type->append_pair(_("HTTP"), "PROXY_HTTP");
	proxy_type->append_pair(_("HTTP with password"), "PROXY_HTTP_PW");

	// setup the plugins listview
	model_plugins = Gtk::ListStore::create(plugin_columns);
	treeview_plugins->get_selection()->signal_changed().connect(
		sigc::mem_fun(this, &SettingsWin::on_plugin_changed));
	treeview_plugins->set_model(model_plugins);
	Gtk::CellRendererToggle* trender = new Gtk::CellRendererToggle();
	trender->signal_toggled().connect(sigc::mem_fun(this, &SettingsWin::on_plugin_toggled));
	treeview_plugins->append_column(_("Load"), *manage(trender));
	treeview_plugins->get_column(0)->add_attribute(*trender, "active", 0);
	treeview_plugins->append_column(_("Name"), plugin_columns.name);
	treeview_plugins->append_column(_("Description"), plugin_columns.description);
	for(unsigned int i = 0; i < 3; i++)
	{
		Gtk::TreeView::Column* column = treeview_plugins->get_column(i);
		column->set_sort_column_id(i);
		column->set_resizable(true);
	}

	setup_settings_widgets();

	show_all_children();
}

SettingsWin::~SettingsWin()
{
	delete interfaces;
}

void SettingsWin::setup_settings_widgets()
{
	interfaces->init("network/interface");
	port->init("network/port");
	enc_policy->init("network/encryption/policy");
	enc_level->init("network/encryption/level");
	enable_dht->init("network/use_dht");
	dht_fallback->init("network/dht_fallback");
	enable_pex->init("network/use_pex");
	multiple_connections->init("network/multiple_connections_per_ip");
	up_rate->init("network/max_up_rate");
	down_rate->init("network/max_down_rate");
	max_uploads->init("network/max_uploads");
	max_connections->init("network/max_connections");
	proxy_host->init("network/proxy/ip");
	proxy_port->init("network/proxy/port");
	proxy_username->init("network/proxy/login");
	proxy_password->init("network/proxy/pass");
	proxy_type->init("network/proxy/type");
	max_torrent_uploads->init("torrent/max_uploads");
	max_torrent_connections->init("network/max_connections");
	max_active->init("torrent/queue/max_active");
	desired_ratio->init("torrent/desired_ratio");
	stop_ratio->init("torrent/stop_ratio");
	lazy_bitfields->init("torrent/lazy_bitfields");
	color_downloading->init("ui/colors/downloading");
	color_finished->init("ui/colors/finished");
	color_seeding->init("ui/colors/seeding");
	color_announcing->init("ui/colors/announcing");
	color_stopped->init("ui/colors/stopped");
	color_queued->init("ui/colors/queued");
	color_check_queue->init("ui/colors/check_queue");
	color_checking->init("ui/colors/checking");
	color_allocating->init("ui/colors/allocating");
	color_error->init("ui/colors/error");
	update_interval->init("ui/interval");
	auto_expand->init("ui/auto_expand");
	trunkate_names->init("ui/torrent_view/trunkate_names");
	auto_expand->init("ui/auto_expand");
	allow_linebreaks->init("ui/allow_linebreak_comments");
	name_width->init("ui/torrent_view/max_name_width");
	default_path->init("files/use_default_path");
	button_default_path->init("files/default_path");
	move_finished->init("files/move_finished");
	button_move_finished->init("files/finished_path");
	allocate->init("files/allocate");
	prio_firstlast->init("files/prioritize_firstlast");
	max_open->init("files/max_open");
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

void SettingsWin::on_plugin_toggled(const Glib::ustring& path)
{
	Gtk::TreeRow row = *(model_plugins->get_iter(path));
	row[plugin_columns.load] = !row[plugin_columns.load];

	SettingsManagerPtr sm = Engine::get_settings_manager();

	Gtk::TreeNodeChildren children = model_plugins->children();
	std::list<Glib::ustring> plugins = sm->get_string_list("ui/plugins");
	if (row[plugin_columns.load])
		plugins.push_back(row[plugin_columns.name]);
	else
		plugins.erase(std::find(plugins.begin(), plugins.end(), row[plugin_columns.name]));
	sm->set("ui/plugins", Glib::SListHandle<Glib::ustring>(plugins));
}

void SettingsWin::on_about_plugin()
{
	Gtk::TreeIter iter = treeview_plugins->get_selection()->get_selected();
	if (iter)
	{
		Gtk::TreeRow row = *iter;

		Gtk::AboutDialog about;
		std::list<Glib::ustring> people;
		people.push_back(row[plugin_columns.author]);
		about.set_authors(people);
		about.set_comments(row[plugin_columns.description]);
		about.set_version(row[plugin_columns.version]);
		about.set_website(row[plugin_columns.website]);
		about.set_name(row[plugin_columns.name]);
		about.set_transient_for(*this);
		about.run();
	}
}

void SettingsWin::on_configure_plugin()
{
	Gtk::TreeIter iter = treeview_plugins->get_selection()->get_selected();
	if (iter)
	{
		Gtk::TreeRow row = *iter;

		PluginPtr plugin = Engine::get_plugin_manager()->get_plugin(row[plugin_columns.name]);
		Glib::ustring s = row[plugin_columns.name];
		if (plugin)
		{
			Gtk::Dialog* dialog = plugin->get_config_dialog();
			if (dialog)
			{
				dialog->set_transient_for(*this);
				dialog->run();
			}
		}
	}
}

void SettingsWin::on_plugin_changed()
{
	Gtk::TreeIter iter = treeview_plugins->get_selection()->get_selected();
	if (iter)
	{
		Gtk::TreeRow row = *iter;
		configure_plugin->set_sensitive(row[plugin_columns.configurable]);
		if (!about_plugin->is_sensitive())
			about_plugin->set_sensitive(true);
	}
}

void SettingsWin::on_show()
{
	SettingsManagerPtr sm = Engine::get_settings_manager();

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
			row[plugin_columns.version] = info.version;
			row[plugin_columns.configurable] = info.has_options;
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

