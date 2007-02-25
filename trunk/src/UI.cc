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

#include <curl/curl.h>

#include <glib/gstdio.h>

#include <gtkmm/main.h>
#include <gtkmm/menu.h>
#include <gtkmm/menubar.h>
#include <gtkmm/accelgroup.h>
#include <gtkmm/stock.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/separatortoolitem.h>
#include <gtkmm/table.h>
#include <gtkmm/frame.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/scrollbar.h>
#include <gtkmm/box.h>
#include <gtkmm/paned.h>

#include "UI.hh"
#include "linkage/Engine.hh"

Glib::ustring ui_info = "<ui>"
												"	<menubar name='MenuBar'>"
												"		<menu action='FileMenu'>"
												"			<menuitem action='New'/>"
												"			<menuitem action='Open'/>"
												"			<separator/>"
												"			<menuitem action='Quit'/>"
												"		</menu>"
												"		<menu action='ViewMenu'>"
												"			<menuitem action='Information'/>"
												"			<separator/>"
												"			<menuitem action='Preferences'/>"
												"		</menu>"
												"		<menu action='HelpMenu'>"
												"			<menuitem action='About'/>"
												"		</menu>"
												"	</menubar>"
												"	<toolbar name='ToolBar'>"
												"		<toolitem action='Add'/>"
												"		<toolitem action='Remove'/>"
												"		<separator/>"
												"		<toolitem action='Start'/>"
												"		<toolitem action='Stop'/>"
												"		<separator/>"
												"		<toolitem action='Up'/>"
												"		<toolitem action='Down'/>"
												"	</toolbar>"
												"</ui>";


UI::UI()
{
	settings_win = new SettingsWin(this);
	torrent_win = new TorrentCreator(this);

	set_title("Linkage");

	menu_trackers = manage(new Gtk::Menu());

	action_group = Gtk::ActionGroup::create();

	action_group->add(Gtk::Action::create("FileMenu", "_File"));
	action_group->add(Gtk::Action::create("ViewMenu", "_View"));
	action_group->add(Gtk::Action::create("HelpMenu", "_Help"));

	action_group->add(Gtk::Action::create("New", Gtk::Stock::NEW, "_New", "Create a new torrent file"),
										Gtk::AccelKey("<control>N"),
										sigc::mem_fun(this, &TorrentCreator::show));
	action_group->add(Gtk::Action::create("Open", Gtk::Stock::OPEN, "_Open", "Open a torrent file"),
										Gtk::AccelKey("<control>O"),
										sigc::mem_fun(this, &UI::on_add));
	action_group->add(Gtk::Action::create("Quit", Gtk::Stock::QUIT, "_Quit", "Quit"),
										Gtk::AccelKey("<control>Q"),
										sigc::mem_fun(this, &UI::on_quit));
	action_group->add(Gtk::Action::create("Information", Gtk::Stock::DIALOG_INFO, "_Information", "Show detailed information"),
										sigc::mem_fun(this, &UI::on_info));
	action_group->add(Gtk::Action::create("Preferences", Gtk::Stock::PREFERENCES, "Prefere_nces", "Configure linkage"),
										sigc::mem_fun(this, &UI::on_prefs));
	action_group->add(Gtk::Action::create("About", Gtk::Stock::ABOUT, "_About", "About"),
										sigc::mem_fun(this, &UI::on_about));

	action_group->add(Gtk::Action::create("Add", Gtk::Stock::ADD, "Add", "Add torrent"),
										sigc::mem_fun(this, &UI::on_add));
	action_group->add(Gtk::Action::create("Remove", Gtk::Stock::REMOVE, "Remove", "Remove torrent"),
										sigc::mem_fun(this, &UI::on_remove));
	action_group->add(Gtk::Action::create("Start", Gtk::Stock::APPLY, "Start", "Start torrent"),
										sigc::mem_fun(this, &UI::on_start));
	action_group->add(Gtk::Action::create("Stop", Gtk::Stock::STOP, "Stop", "Stop torrent"),
										sigc::mem_fun(this, &UI::on_stop));
	action_group->add(Gtk::Action::create("Up", Gtk::Stock::GO_UP, "Up", "Move up"),
										sigc::mem_fun(this, &UI::on_up));
	action_group->add(Gtk::Action::create("Down", Gtk::Stock::GO_DOWN, "Down", "Move down"),
										sigc::mem_fun(this, &UI::on_down));

	manager = Gtk::UIManager::create();
	manager->insert_action_group(action_group);
	add_accel_group(manager->get_accel_group());
	manager->add_ui_from_string(ui_info);

	Gtk::VBox* main_vbox = manage(new Gtk::VBox);
	add(*main_vbox);

	Gtk::Widget* menubar = manager->get_widget("/MenuBar");
	main_vbox->pack_start(*menubar, false, false, 0);

	Gtk::Toolbar* toolbar = dynamic_cast<Gtk::Toolbar*>(manager->get_widget("/ToolBar"));

	switch (Engine::instance()->get_settings_manager()->get_int("UI", "SortOrder"))
	{
		case 1:
			tb_sort = manage(new Gtk::MenuToolButton(Gtk::Stock::SORT_DESCENDING));
			break;
		case 0:
		default:
			tb_sort = manage(new Gtk::MenuToolButton(Gtk::Stock::SORT_ASCENDING));
			break;
	}
	Gtk::Menu* tb_sort_menu = manage(new Gtk::Menu());
	Gtk::MenuItem* item = manage(new Gtk::MenuItem("Group size"));
	item->signal_activate().connect(sigc::bind(sigc::mem_fun(
			this, &UI::on_sort_item_selected), TorrentList::COL_CHILDREN));
	tb_sort_menu->append(*item);
	item = manage(new Gtk::MenuItem("Position"));
	item->signal_activate().connect(sigc::bind(sigc::mem_fun(
			this, &UI::on_sort_item_selected), TorrentList::COL_POSITION));
	tb_sort_menu->append(*item);
	item = manage(new Gtk::MenuItem("Name"));
	item->signal_activate().connect(sigc::bind(sigc::mem_fun(
			this, &UI::on_sort_item_selected), TorrentList::COL_NAME));
	tb_sort_menu->append(*item);
	item = manage(new Gtk::MenuItem("Status"));
	item->signal_activate().connect(sigc::bind(sigc::mem_fun(
			this, &UI::on_sort_item_selected), TorrentList::COL_STATUS));
	tb_sort_menu->append(*item);
	item = manage(new Gtk::MenuItem("Downloaded"));
	item->signal_activate().connect(sigc::bind(sigc::mem_fun(
			this, &UI::on_sort_item_selected), TorrentList::COL_DOWNLOADED));
	tb_sort_menu->append(*item);
	item = manage(new Gtk::MenuItem("Uploaded"));
	item->signal_activate().connect(sigc::bind(sigc::mem_fun(
			this, &UI::on_sort_item_selected), TorrentList::COL_UPLOADED));
	tb_sort_menu->append(*item);
	item = manage(new Gtk::MenuItem("Download rate"));
	item->signal_activate().connect(sigc::bind(sigc::mem_fun(
			this, &UI::on_sort_item_selected), TorrentList::COL_DOWNRATE));
	tb_sort_menu->append(*item);
	item = manage(new Gtk::MenuItem("Upload rate"));
	item->signal_activate().connect(sigc::bind(sigc::mem_fun(
			this, &UI::on_sort_item_selected), TorrentList::COL_UPRATE));
	tb_sort_menu->append(*item);
	item = manage(new Gtk::MenuItem("Seeds"));
	item->signal_activate().connect(sigc::bind(sigc::mem_fun(
			this, &UI::on_sort_item_selected), TorrentList::COL_SEEDS));
	tb_sort_menu->append(*item);
	item = manage(new Gtk::MenuItem("Peers"));
	item->signal_activate().connect(sigc::bind(sigc::mem_fun(
			this, &UI::on_sort_item_selected), TorrentList::COL_PEERS));
	tb_sort_menu->append(*item);
	item = manage(new Gtk::MenuItem("Progress"));
	item->signal_activate().connect(sigc::bind(sigc::mem_fun(
			this, &UI::on_sort_item_selected), TorrentList::COL_PROGRESS));
	tb_sort_menu->append(*item);
	tb_sort_menu->show_all_children();
	tb_sort->set_menu(*tb_sort_menu);
	tb_sort->signal_clicked().connect(sigc::mem_fun(this, &UI::on_sort));
	toolbar->append(*manage(new Gtk::SeparatorToolItem()));
	toolbar->append(*tb_sort);

	main_vbox->pack_start(*toolbar, false, false, 0);

	notebook_main = manage(new Gtk::Notebook());
	notebook_main->set_show_tabs(false);
	Gtk::ScrolledWindow* scrollwin = manage(new Gtk::ScrolledWindow());
	scrollwin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	torrent_list = new TorrentList();
	torrent_list->signal_changed().connect(sigc::mem_fun(this, &UI::on_torrent_list_selection_changed));
	torrent_list->signal_double_click().connect(sigc::mem_fun(this, &UI::on_torrent_list_double_clicked));
	torrent_list->signal_right_click().connect(sigc::mem_fun(this, &UI::on_torrent_list_right_clicked));

	std::list<Gtk::TargetEntry> targets;
	targets.push_back(Gtk::TargetEntry("STRING", Gtk::TargetFlags(0)));
	targets.push_back(Gtk::TargetEntry("text/plain", Gtk::TargetFlags(0)));
	targets.push_back(Gtk::TargetEntry("text/uri-list", Gtk::TargetFlags(0)));
	torrent_list->drag_dest_set(targets, Gtk::DEST_DEFAULT_ALL, Gdk::DragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE));
	torrent_list->signal_drag_data_received().connect(sigc::mem_fun(this, &UI::on_dnd_received));

	scrollwin->add(*torrent_list);
	notebook_main->append_page(*scrollwin, "Torrents");
	Gtk::VPaned* vpan = new Gtk::VPaned();
	main_vbox->pack_start(*vpan, true, true, 0);
	vpan->pack1(*notebook_main, true, false);

	expander_details = manage(new Gtk::Expander("<b>Details</b>"));
	expander_details->set_use_markup(true);
	expander_details->property_expanded().signal_changed().connect(sigc::mem_fun(this, &UI::on_details_expanded));
	vpan->pack2(*expander_details, false, true);

	notebook_details = manage(new Gtk::Notebook());
	connection_switch_page = notebook_details->signal_switch_page().connect(sigc::mem_fun(this, &UI::on_switch_page));
	expander_details->add(*notebook_details);


	Gtk::VBox* general_box = manage(new Gtk::VBox());

	Gtk::Frame* frame_pieces = manage(new Gtk::Frame());
	Gtk::Label* label = manage(new AlignedLabel());
	label->set_use_markup(true);
	label->set_markup("<b>Pieces</b>");
	frame_pieces->set_label_widget(*label);

	Glib::RefPtr<Gdk::Colormap> colormap = get_default_colormap();

	std::vector<int> rgb = Engine::instance()->get_settings_manager()->get_int_list("UI", "ColorDark");
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

	piecemap = new PieceMap(dark, mid, light);

	frame_pieces->add(*piecemap);
	general_box->pack_start(*frame_pieces, false, false, 0);

	Gtk::Frame* frame_tracker = manage(new Gtk::Frame());
	label = manage(new AlignedLabel());
	label->set_use_markup(true);
	label->set_markup("<b>Tracker</b>");
	frame_tracker->set_label_widget(*label);

	Gtk::Table* table_tracker = manage(new Gtk::Table(2, 4));
	table_tracker->set_spacings(10);
	table_tracker->set_border_width(5);
	label = manage(new AlignedLabel("Active tracker:"));
	table_tracker->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
	label = manage(new AlignedLabel("Next announce:"));
	table_tracker->attach(*label, 2, 3, 0, 1, Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
	label = manage(new AlignedLabel("Response:"));
	table_tracker->attach(*label, 0, 1, 1, 2, Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
	label = manage(new AlignedLabel("Private:"));
	table_tracker->attach(*label, 2, 3, 1, 2, Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
	button_tracker = manage(new Gtk::Button());
	button_tracker->set_relief(Gtk::RELIEF_NONE);
	button_tracker->add_events(Gdk::BUTTON_RELEASE_MASK);
	button_tracker->signal_button_release_event().connect(sigc::mem_fun(this, &UI::on_tracker_update), false);
	button_tracker->signal_enter().connect(sigc::mem_fun(this, &UI::on_tracker_enter));
	button_tracker->signal_leave().connect(sigc::mem_fun(this, &UI::on_tracker_leave));
	Gtk::HBox* tracker_box = manage(new Gtk::HBox());
	tracker_box->pack_start(*button_tracker, false, false);
	table_tracker->attach(*tracker_box, 1, 2, 0, 1, Gtk::SHRINK, Gtk::EXPAND|Gtk::FILL);
	label_next_announce = manage(new AlignedLabel());
	table_tracker->attach(*label_next_announce, 3, 4, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
	label_response = manage(new AlignedLabel());
	table_tracker->attach(*label_response, 1, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
	label_private = manage(new AlignedLabel());
	table_tracker->attach(*label_private, 3, 4, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
	
	frame_tracker->add(*table_tracker);
	general_box->pack_start(*frame_tracker, false, false, 0);

	Gtk::Frame* frame_origin = manage(new Gtk::Frame());
	label = manage(new AlignedLabel());
	label->set_use_markup(true);
	label->set_markup("<b>Origin</b>");
	frame_origin->set_label_widget(*label);

	Gtk::Table* table_origin = manage(new Gtk::Table(2, 4));
	table_origin->set_spacings(10);
	table_origin->set_border_width(5);
	label = manage(new AlignedLabel("Creator:"));
	table_origin->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
	label = manage(new AlignedLabel("Creation date:"));
	table_origin->attach(*label, 2, 3, 0, 1, Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
	label = manage(new AlignedLabel("Comment:"));
	table_origin->attach(*label, 0, 1, 1, 2, Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
	label_creator = manage(new AlignedLabel());
	table_origin->attach(*label_creator, 1, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
	label_date = manage(new AlignedLabel());
	table_origin->attach(*label_date, 3, 4, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
	label_comment = manage(new AlignedLabel());
	table_origin->attach(*label_comment, 1, 4, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL);

	frame_origin->add(*table_origin);
	general_box->pack_start(*frame_origin, false, false, 0);

	notebook_details->append_page(*general_box, "General");

	Gtk::VBox* transfer_box = manage(new Gtk::VBox());

	Gtk::Table* table_transfer = manage(new Gtk::Table(2, 8));
	table_transfer->set_spacings(10);
	table_transfer->set_border_width(5);
	label = manage(new AlignedLabel("Download rate:"));
	table_transfer->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Download limit:"));
	table_transfer->attach(*label, 2, 3, 0, 1, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Upload rate:"));
	table_transfer->attach(*label, 4, 5, 0, 1, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Upload limit:"));
	table_transfer->attach(*label, 6, 7, 0, 1, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Downloaded:"));
	table_transfer->attach(*label, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Uploaded:"));
	table_transfer->attach(*label, 2, 3, 1, 2, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Share ratio:"));
	table_transfer->attach(*label, 4, 5, 1, 2, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Wasted:"));
	table_transfer->attach(*label, 6, 7, 1, 2, Gtk::FILL, Gtk::SHRINK);
	label_down_rate = manage(new AlignedLabel());
	table_transfer->attach(*label_down_rate, 1, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);
	spinbutton_down = manage(new AlignedSpinButton(0.0, 1000.0));
	spinbutton_down->signal_value_changed().connect(sigc::mem_fun(this, &UI::on_spin_down));
	table_transfer->attach(*spinbutton_down, 3, 4, 0, 1, Gtk::FILL, Gtk::SHRINK);
	label_up_rate = manage(new AlignedLabel());
	table_transfer->attach(*label_up_rate, 5, 6, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);
	spinbutton_up = manage(new AlignedSpinButton(0.0, 1000.0));
	spinbutton_up->signal_value_changed().connect(sigc::mem_fun(this, &UI::on_spin_up));
	table_transfer->attach(*spinbutton_up, 7, 8, 0, 1, Gtk::FILL, Gtk::SHRINK);
	label_down = manage(new AlignedLabel());
	table_transfer->attach(*label_down, 1, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);
	label_up = manage(new AlignedLabel());
	table_transfer->attach(*label_up, 3, 4, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);
	label_ratio = manage(new AlignedLabel());
	table_transfer->attach(*label_ratio, 5, 6, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);
	label_wasted = manage(new AlignedLabel());
	table_transfer->attach(*label_wasted, 7, 8, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);

	transfer_box->pack_start(*table_transfer, false, false, 0);

	scrollwin = manage(new Gtk::ScrolledWindow());
	scrollwin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	peer_list = new PeerList();
	scrollwin->add(*peer_list);
	transfer_box->pack_start(*scrollwin, true, true, 0);
	notebook_details->append_page(*transfer_box, "Transfer");

	Gtk::VBox* files_box = manage(new Gtk::VBox());

	Gtk::Table* table_files = manage(new Gtk::Table(2, 6));
	table_files->set_spacings(10);
	table_files->set_border_width(5);
	label = manage(new AlignedLabel("Saving as:"));
	table_files->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Files:"));
	table_files->attach(*label, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Total size:"));
	table_files->attach(*label, 2, 3, 1, 2, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Pieces:"));
	table_files->attach(*label, 4, 5, 1, 2, Gtk::FILL, Gtk::SHRINK);
	label_path = manage(new AlignedLabel());
	table_files->attach(*label_path, 1, 6, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);
	label_files = manage(new AlignedLabel());
	table_files->attach(*label_files, 1, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);
	label_size = manage(new AlignedLabel());
	table_files->attach(*label_size, 3, 4, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);
	label_pieces = manage(new AlignedLabel());
	table_files->attach(*label_pieces, 5, 6, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);

	files_box->pack_start(*table_files, false, false, 0);

	scrollwin = manage(new Gtk::ScrolledWindow());
	scrollwin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	file_list = new FileList();
	scrollwin->add(*file_list);
	files_box->pack_start(*scrollwin, true, true, 0);
	notebook_details->append_page(*files_box, "Files");

	statusbar = manage(new Statusbar());
	main_vbox->pack_start(*statusbar, false, false, 0);

	show_all_children();

	file_chooser = new OpenDialog(this);
	path_chooser = new SaveDialog(this);

	Engine::instance()->get_session_manager()->resume_session();

	Glib::RefPtr<SettingsManager> sm = Engine::instance()->get_settings_manager();

	resize(sm->get_int("UI", "WinWidth"), sm->get_int("UI", "WinHeight"));

	notebook_details->set_current_page(sm->get_int("UI", "Page"));
	/* this makes expander insensitive even if we have a valid selection,
			if we shutdown with expander not expanded */
	expander_details->set_sensitive(sm->get_bool("UI", "Expanded"));
	expander_details->set_expanded(sm->get_bool("UI", "Expanded"));

	int max_up = sm->get_int("Network", "MaxUpRate");
	if (max_up == 0)
		max_up = 1000;
	spinbutton_up->set_range(0, max_up);
	int max_down = sm->get_int("Network", "MaxDownRate");
	if (max_down == 0)
		max_down = 1000;
	spinbutton_down->set_range(0, max_down);

	Engine::instance()->get_settings_manager()->signal_update_settings().connect(sigc::mem_fun(this, &UI::on_settings));

	Engine::instance()->get_plugin_manager()->signal_plugin_load().connect(sigc::mem_fun(this, &UI::on_plugin_load));
	Engine::instance()->get_plugin_manager()->signal_plugin_unload().connect(sigc::mem_fun(this, &UI::on_plugin_unload));
	Engine::instance()->get_plugin_manager()->signal_add_widget().connect(sigc::mem_fun(this, &UI::on_add_widget));

	Engine::instance()->get_session_manager()->signal_invalid_bencoding().connect(sigc::mem_fun(this, &UI::on_invalid_bencoding));
	Engine::instance()->get_session_manager()->signal_missing_file().connect(sigc::mem_fun(this, &UI::on_missing_file));
	Engine::instance()->get_session_manager()->signal_duplicate_torrent().connect(sigc::mem_fun(this, &UI::on_duplicate_torrent));

	Engine::instance()->get_alert_manager()->signal_listen_failed().connect(sigc::mem_fun(this, &UI::on_listen_failed));
	Engine::instance()->get_alert_manager()->signal_tracker_failed().connect(sigc::mem_fun(this, &UI::on_tracker_failed));
	Engine::instance()->get_alert_manager()->signal_tracker_reply().connect(sigc::mem_fun(this, &UI::on_tracker_reply));
	Engine::instance()->get_alert_manager()->signal_tracker_warning().connect(sigc::mem_fun(this, &UI::on_tracker_warning));
	Engine::instance()->get_alert_manager()->signal_tracker_announce().connect(sigc::mem_fun(this, &UI::on_tracker_announce));
	Engine::instance()->get_alert_manager()->signal_torrent_finished().connect(sigc::mem_fun(this, &UI::on_torrent_finished));
	Engine::instance()->get_alert_manager()->signal_file_error().connect(sigc::mem_fun(this, &UI::on_file_error));
	Engine::instance()->get_alert_manager()->signal_fastresume_rejected().connect(sigc::mem_fun(this, &UI::on_fastresume_rejected));
	Engine::instance()->get_alert_manager()->signal_hash_failed().connect(sigc::mem_fun(this, &UI::on_hash_failed));
	Engine::instance()->get_alert_manager()->signal_peer_ban().connect(sigc::mem_fun(this, &UI::on_peer_ban));

	Engine::instance()->get_dbus_manager()->signal_open().connect(sigc::mem_fun(this, &UI::add_torrent));
	Engine::instance()->get_dbus_manager()->signal_quit().connect(sigc::mem_fun(this, &UI::on_quit));
	Engine::instance()->get_dbus_manager()->signal_toggle_visible().connect(sigc::mem_fun(this, &UI::on_toggle_visible));

	//Glib::signal_timeout().connect(sigc::mem_fun(this, &UI::on_timeout), sm->get_int("UI", "Interval")*1000);
	connection_tick = Engine::instance()->signal_tick().connect(sigc::mem_fun(this, &UI::on_tick));
}

UI::~UI()
{
	/* This seems to cause a segfault if connected */
	connection_switch_page.disconnect();

	delete torrent_list;
	delete piecemap;
	delete peer_list;
	delete file_list;
	delete settings_win;
	delete torrent_win;
}

void UI::save_state()
{
	Glib::RefPtr<SettingsManager> sm = Engine::instance()->get_settings_manager();

	int w, h;
	get_size(w, h);

	sm->set("UI", "WinWidth", w);
	sm->set("UI", "WinHeight", h);
	sm->set("UI", "Page", notebook_details->get_current_page());
	sm->set("UI", "Expanded", expander_details->get_expanded());
}

void UI::on_plugin_load(Plugin* plugin)
{
}

void UI::on_plugin_unload(Plugin* plugin)
{
	Gtk::Widget* widget = plugin->get_widget();
	Plugin::PluginParent parent = plugin->get_parent();
	switch (parent)
	{
		case Plugin::PARENT_MAIN:
			notebook_main->remove_page(*widget);
			break;
		case Plugin::PARENT_DETAILS:
			notebook_details->remove_page(*widget);
			break;
	}
}

void UI::on_add_widget(Plugin* plugin, Gtk::Widget* widget, Plugin::PluginParent parent)
{
	//FIXME: Add PARENT_MENU, PARENT_TOOLBAR
	Glib::ustring name = plugin->get_name();
	switch (parent)
	{
		case Plugin::PARENT_MAIN:
			notebook_main->append_page(*widget, name);
			notebook_main->set_show_tabs((notebook_main->get_n_pages() > 1));
			widget->show();
			break;
		case Plugin::PARENT_DETAILS:
			notebook_details->append_page(*widget, name);
			widget->show();
			break;
	}
}

void UI::notify(const Glib::ustring& title,
								const Glib::ustring& msg)
{
	statusbar->pop();
	statusbar->push(msg);
}

void UI::on_tick()
{
	/* Only update lists every 3rd tick, should be configurable */
	static int tick;
	tick = (tick + 1) % 3;

	if (!is_visible())
	{
		connection_tick.block();
		return;
	}

	TorrentManager::TorrentList torrents = Engine::instance()->get_torrent_manager()->get_torrents();
	for (TorrentManager::TorrentList::iterator iter = torrents.begin();
				iter != torrents.end(); ++iter)
	{
		if (*iter)
			update(*iter, (tick == 0));
	}

	session_status status = Engine::instance()->get_session_manager()->status();
	statusbar->set_connections_label(str(status.num_peers));
	statusbar->set_download_label(suffix_value(status.payload_download_rate) + "/s");
	statusbar->set_upload_label(suffix_value(status.payload_upload_rate) + "/s");

	torrent_list->update_groups();
}

bool UI::on_visibility_notify_event(GdkEventVisibility* event)
{
	if (event->state != GDK_VISIBILITY_FULLY_OBSCURED)
		connection_tick.unblock();

	return false;
}

void UI::update(const WeakPtr<Torrent>& torrent, bool update_lists)
{
	sha1_hash hash = torrent->get_hash();
	bool selected = torrent_list->is_selected(hash);
	torrent_status stats;
	if (torrent->is_running())
	{
		stats = torrent->get_status();
	}
	else if (selected)
		reset_views();

	torrent_list->update_row(torrent);

	if (selected && expander_details->get_expanded() && torrent->is_running())
	{
		int down = torrent->get_total_downloaded();
		int up = torrent->get_total_uploaded();
		double ratio = 0;
		switch (notebook_details->get_current_page())
		{
			case PAGE_GENERAL:
				if (stats.pieces)
					piecemap->set_map(*stats.pieces);
				if (stats.current_tracker.size())
					button_tracker->set_label(stats.current_tracker);
				label_next_announce->set_text(to_simple_string(stats.next_announce));
				label_response->set_text(torrent->get_tracker_reply());
				break;
			case PAGE_PEERS:
				label_down->set_text(suffix_value(down));
				label_down_rate->set_text(suffix_value((int)stats.download_payload_rate) + "/s");
				label_up->set_text(suffix_value(up));
				label_up_rate->set_text(suffix_value((int)stats.upload_payload_rate) + "/s");
				if (down)
					ratio = (double)up/down;
				label_ratio->set_text(str(ratio, 3));
				label_wasted->set_text(suffix_value((int)stats.total_failed_bytes));
				if (update_lists)
					peer_list->update(torrent);
				break;
			case PAGE_FILES:
				if (stats.pieces && update_lists)
					file_list->update(torrent);
				break;

		}
	}
}

void UI::update_statics(const WeakPtr<Torrent>& torrent)
{
	if (torrent->is_running())
	{
		torrent_info info = torrent->get_info();

		label_creator->set_text(info.creator());
		label_comment->set_text(info.comment());
		label_date->set_text(to_simple_string(*info.creation_date()));
		label_path->set_text(Glib::build_filename(torrent->get_path(), info.name()));
		label_size->set_text(suffix_value((int)info.total_size()));
		label_files->set_text(str(info.num_files()));
		label_pieces->set_text(str(info.num_pieces()) + " x " + suffix_value((int)info.piece_length()));
		label_private->set_text(info.priv() ? "Yes" : "No");
	}
}

void UI::reset_views()
{
	file_list->clear();
	peer_list->clear();
	piecemap->set_map(std::vector<bool>(1, false));
	label_down->set_text("");
	label_up->set_text("");
	label_down_rate->set_text("");
	label_up_rate->set_text("");
	label_ratio->set_text("");
	label_wasted->set_text("");
	label_response->set_text("");
	label_next_announce->set_text("");
	label_path->set_text("");
	label_comment->set_text("");
	label_size->set_text("");
	label_files->set_text("");
	label_pieces->set_text("");
	label_creator->set_text("");
	label_date->set_text("");
	label_private->set_text("");
	button_tracker->set_label("");
}

void UI::build_tracker_menu(const WeakPtr<Torrent>& torrent)
{
	menu_trackers->items().clear();

	std::vector<announce_entry> trackers = torrent->get_handle().get_torrent_info().trackers();

	for (int i = 0; i < trackers.size(); i++)
	{
		Glib::ustring tracker = trackers[i].url;
		Gtk::MenuItem* item = manage(new Gtk::MenuItem(tracker));
		item->signal_activate().connect(sigc::bind(sigc::mem_fun(
			this, &UI::on_popup_tracker_selected), tracker, trackers[i].tier));
		menu_trackers->append(*item);
	}
	menu_trackers->show_all_children();
}

void UI::add_torrent(const Glib::ustring& file)
{
	if (!Glib::file_test(file, Glib::FILE_TEST_EXISTS))
	{
		Engine::instance()->get_session_manager()->signal_missing_file().emit("File not found, \"" + file + "\"", file);
		return;
	}
	
	Glib::ustring save_path;

	if (!Engine::instance()->get_settings_manager()->get_bool("Files", "UseDefPath"))
		if (path_chooser->run() == Gtk::RESPONSE_OK)
		{
			save_path = path_chooser->get_filename();
			path_chooser->hide();
		}
		else
		{
			path_chooser->hide();
			return;
		}
	else
		save_path = Engine::instance()->get_settings_manager()->get_string("Files", "DefPath");

	sha1_hash hash = Engine::instance()->get_session_manager()->open_torrent(file, save_path);
	WeakPtr<Torrent> torrent = Engine::instance()->get_torrent_manager()->get_torrent(hash);
	if (torrent)
		update(torrent, expander_details->get_expanded());
}

OpenDialog::OpenDialog(Gtk::Window *parent)
: Gtk::FileChooserDialog(*parent, "Open torrent")
{
	torrent_filter = new Gtk::FileFilter();
	torrent_filter->add_mime_type("application/x-bittorrent");
	torrent_filter->set_name("BitTorrent files");

	no_filter = new Gtk::FileFilter();
	no_filter->add_pattern("*");
	no_filter->set_name("All files");

	add_filter(*torrent_filter);
	add_filter(*no_filter);

	Gtk::Button *b = add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	b->signal_clicked().connect(sigc::mem_fun(this, &OpenDialog::hide));
	add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
}

OpenDialog::~OpenDialog()
{
	delete torrent_filter;
	delete no_filter;
}

SaveDialog::SaveDialog(Gtk::Window *parent)
: Gtk::FileChooserDialog(*parent, "Select path", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER)
{
	Gtk::Button *b = add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	b->signal_clicked().connect(sigc::mem_fun(this, &SaveDialog::hide));
	add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
}

SaveDialog::~SaveDialog()
{
}

/* CALLBACKS */

void UI::on_info()
{
	if (expander_details->is_sensitive())
	{
		expander_details->set_expanded(true);
		notebook_details->set_current_page(0);
	}
}

void UI::on_prefs()
{
	settings_win->show();
}

void UI::on_about()
{
	std::cout << "About" << std::endl;
}

void UI::on_spin_down()
{
	HashList list = torrent_list->get_selected_list();
	if (list.size() == 1) /* FIXME: This check _shouldn't_ be/(isn't?) needed! */
	{
		WeakPtr<Torrent> torrent = Engine::instance()->get_torrent_manager()->get_torrent(*list.begin());
		torrent->set_down_limit((int)spinbutton_down->get_value());
	}
}

void UI::on_spin_up()
{
	HashList list = torrent_list->get_selected_list();
	if (list.size() == 1) /* FIXME: This check _shouldn't_ be/(isn't?) needed! */
	{
		WeakPtr<Torrent> torrent = Engine::instance()->get_torrent_manager()->get_torrent(*list.begin());
		torrent->set_up_limit((int)spinbutton_up->get_value());
	}
}

void UI::on_settings()
{
	int max_up = Engine::instance()->get_settings_manager()->get_int("Network", "MaxUpRate");
	if (max_up == 0)
		max_up = 1000;
	spinbutton_up->set_range(0, max_up);
	int max_down = Engine::instance()->get_settings_manager()->get_int("Network", "MaxDownRate");
	if (max_down == 0)
		max_down = 1000;
	spinbutton_down->set_range(0, max_down);
}

void UI::on_add()
{
	if (file_chooser->run() == Gtk::RESPONSE_OK)
	{
		file_chooser->hide();
		Glib::ustring file = file_chooser->get_filename();
		add_torrent(file);
	}
	else
		file_chooser->hide();
}

void UI::on_remove()
{
	HashList list = torrent_list->get_selected_list();

	for (HashList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		sha1_hash hash = *iter;
		Engine::instance()->get_session_manager()->erase_torrent(hash);
	}

	if (list.size())
	{
		expander_details->set_expanded(false);
		expander_details->set_sensitive(false);
	}
}

void UI::on_start()
{
	HashList list = torrent_list->get_selected_list();

	for (HashList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		sha1_hash hash = *iter;
		WeakPtr<Torrent> torrent = Engine::instance()->get_torrent_manager()->get_torrent(hash);
		if (!torrent->is_running())
		{
			Engine::instance()->get_session_manager()->resume_torrent(hash);
			update(torrent, (list.size() == 1));
		}
	}
}

void UI::on_stop()
{
	HashList list = torrent_list->get_selected_list();

	for (HashList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		sha1_hash hash = *iter;
		Engine::instance()->get_session_manager()->stop_torrent(hash);
	}

	if (list.size())
	{
		button_tracker->set_sensitive(false);
		reset_views();
	}
}

void UI::on_up()
{
	HashList list = torrent_list->get_selected_list();

	for (HashList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		sha1_hash hash = *iter;
		WeakPtr<Torrent> torrent = Engine::instance()->get_torrent_manager()->get_torrent(hash);
		int position = torrent->get_position();
		if (position > 1)
			torrent->set_position(position - 1);
	}
}

void UI::on_down()
{
	HashList list = torrent_list->get_selected_list();

	for (HashList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		sha1_hash hash = *iter;
		WeakPtr<Torrent> torrent = Engine::instance()->get_torrent_manager()->get_torrent(hash);
		int position = torrent->get_position();
		if (position < Engine::instance()->get_torrent_manager()->get_torrents_count())
			torrent->set_position(position + 1);
	}
}

void UI::on_sort()
{
	if (tb_sort->get_stock_id() == "gtk-sort-ascending")
	{
			tb_sort->set_stock_id(Gtk::Stock::SORT_DESCENDING);
			torrent_list->set_sort_order(Gtk::SORT_DESCENDING);
	}
	else if (tb_sort->get_stock_id() == "gtk-sort-descending")
	{
			tb_sort->set_stock_id(Gtk::Stock::SORT_ASCENDING);
			torrent_list->set_sort_order(Gtk::SORT_ASCENDING);
	}
}

void UI::on_sort_item_selected(TorrentList::Column col)
{
	torrent_list->set_sort_column(col);
}

bool UI::on_delete_event(GdkEventAny*)
{
	on_quit();
	return false;
}

void UI::on_quit()
{
	if (torrent_win->get_finished())
	{
		save_state();
		hide();
		Gtk::Main::quit();
	}
}

void UI::on_toggle_visible()
{
	is_visible() ? hide() : show();
}

void UI::on_details_expanded()
{
	if (expander_details->get_expanded())
	{
		HashList list = torrent_list->get_selected_list();
		if (list.size() == 1) /* FIXME: This shouldn't be needed */
		{
			WeakPtr<Torrent> torrent = Engine::instance()->get_torrent_manager()->get_torrent(*list.begin());
			update(torrent, true);
		}
	}
	else
	{
		/* FIXME: store vpan pointer in UI */
		Gtk::VPaned* vpan = dynamic_cast<Gtk::VPaned*>(expander_details->get_parent());
		vpan->set_position(-1);
	}
}

void UI::on_torrent_list_selection_changed()
{
	HashList list = torrent_list->get_selected_list();
	if (list.size() == 1)
	{
		sha1_hash hash = *list.begin();

		expander_details->set_sensitive(true);
		if (Engine::instance()->get_settings_manager()->get_bool("UI", "AutoExpand"))
			expander_details->set_expanded(true);

		WeakPtr<Torrent> torrent = Engine::instance()->get_torrent_manager()->get_torrent(hash);
		spinbutton_down->set_value((double)torrent->get_down_limit());
		spinbutton_up->set_value((double)torrent->get_up_limit());

		button_tracker->set_sensitive(torrent->is_running());

		update_statics(torrent);
		update(torrent, expander_details->get_expanded());
	}
	else /* Multiple torrents selected */
	{
		expander_details->set_expanded(false);
		expander_details->set_sensitive(false);
	}
}

void UI::on_torrent_list_double_clicked(const sha1_hash& hash)
{
	if (expander_details->is_sensitive())
	{
		if (!expander_details->get_expanded())
		{
			if (Engine::instance()->get_torrent_manager()->exists(hash)) /* Unnecessary? */
				expander_details->set_expanded(true);
		}
		else
			expander_details->set_expanded(false);
	}
}

void UI::on_torrent_list_right_clicked(const sha1_hash& hash)
{
	/* FIXME: add popup menu */
}

bool UI::on_tracker_update(GdkEventButton* e)
{
	HashList list = torrent_list->get_selected_list();
	if (list.size() == 1)
	{
		sha1_hash hash = *list.begin();
		WeakPtr<Torrent> torrent = Engine::instance()->get_torrent_manager()->get_torrent(hash);
		if (torrent->is_running())
		{
			switch (e->button)
			{
				case 1:
					/* TODO: add timeout to prevent hammering */
					torrent->get_handle().force_reannounce();
					break;
				case 3:
					build_tracker_menu(torrent);
					menu_trackers->popup(e->button, e->time);
					break;
			}
		}
	}
	return true;
}

void UI::on_popup_tracker_selected(const Glib::ustring& tracker, int tier)
{
	/* TODO: Save info to next session */
	HashList list = torrent_list->get_selected_list();
	if (list.size() == 1)
	{
		sha1_hash hash = *list.begin();
		torrent_handle handle = Engine::instance()->get_torrent_manager()->get_handle(hash);
		std::vector<announce_entry> trackers = handle.get_torrent_info().trackers();
		std::vector<announce_entry> new_trackers;
		for (int i = 0; i < trackers.size(); i++)
		{
			if (trackers[i].url == tracker)
			{
				trackers[i].tier = 0;
				new_trackers.insert(new_trackers.begin(),trackers[i]);
			}
			else
			{
				if (trackers[i].tier < tier)
					trackers[i].tier++;
				new_trackers.push_back(trackers[i]);
			}
		}
		handle.replace_trackers(new_trackers);
		handle.force_reannounce();
	}
}

void UI::on_tracker_enter()
{
/*	Pointer<Gdk::Cursor> cursor = new Gdk::Cursor(GDK_HAND2);
	button_tracker->get_window()->set_cursor(*cursor);
	button_tracker->set_relief(Gtk::RELIEF_NORMAL); */
	return;
}

void UI::on_tracker_leave()
{
/*	Pointer<Gdk::Cursor> cursor = new Gdk::Cursor(GDK_LEFT_PTR);
	button_tracker->get_window()->set_cursor(*cursor);
	button_tracker->set_relief(Gtk::RELIEF_NONE); */
	return;
}

void UI::on_switch_page(GtkNotebookPage*, int page_num)
{
	HashList list = torrent_list->get_selected_list();
	if (list.size() == 1)
	{
		WeakPtr<Torrent> torrent = Engine::instance()->get_torrent_manager()->get_torrent(*list.begin());
		update(torrent, true);
	}
}

void UI::on_dnd_received(const Glib::RefPtr<Gdk::DragContext>& context,
												 int x, int y,
												 const Gtk::SelectionData& selection_data,
												 guint info, guint time)
{
	std::string data = selection_data.get_data_as_string();
	Glib::StringArrayHandle a = context->get_targets();

	bool is_file_uri = false;
	for (Glib::StringArrayHandle::iterator ai = a.begin(); ai != a.end(); ++ai)
		if (*ai == "text/uri-list")
			is_file_uri = true;

	if (is_file_uri)
	{
		std::list<std::string> uri_list;
		int pos, offset = 0;
		while ((pos = data.find("\n", offset)) != std::string::npos)
		{
			std::string s = data.substr(0, pos-1);
			uri_list.push_back(s);
			offset = pos+1;
		}
		for (std::list<std::string>::iterator li = uri_list.begin(); li != uri_list.end(); ++li)
		{
			std::string file = *li;
			pos = file.find("file://");
			if (pos != std::string::npos)
				file.erase(0, 7); /* Get rid of leading file:// */
			add_torrent(file);
		}
	}
	else
	{
		/* TODO: Add a progress bar */	
		notify("Downloading torrent", "downloading " + data);
 
		CURL *curl;
		CURLcode res;

		char err[CURL_ERROR_SIZE];
		FILE* curl_file = g_fopen("/tmp/.linkage-tmp.torrent", "wb");

		curl = curl_easy_init();
		if(curl)
		{
			curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, err);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
			curl_easy_setopt(curl, CURLOPT_HEADER, 0);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, curl_file);

			curl_easy_setopt(curl, CURLOPT_URL, data.c_str());

			int res = curl_easy_perform(curl);

			curl_easy_cleanup(curl);

			fclose(curl_file);

			if (res == CURLE_OK)
				add_torrent("/tmp/.linkage-tmp.torrent");
			else
				std::cerr << err << std::endl;
		}
	}
	context->drag_finish(false, false, time);
}

void UI::on_invalid_bencoding(const Glib::ustring& msg, const Glib::ustring& file)
{
	notify("Invalid bencoding", msg);
}

void UI::on_missing_file(const Glib::ustring& msg, const Glib::ustring& file)
{
	notify("Missing file", msg);
}

void UI::on_duplicate_torrent(const Glib::ustring& msg, const sha1_hash& hash)
{
	notify("Duplicate torrent", msg);
}

void UI::on_listen_failed(const Glib::ustring& msg)
{
	notify("Listen failed", msg);
}

void UI::on_tracker_failed(const sha1_hash& hash, const Glib::ustring& msg, int code, int times)
{
	notify("Tracker failed", msg);
}

void UI::on_tracker_reply(const sha1_hash& hash, const Glib::ustring& msg, int peers)
{
	notify("Tracker response", msg);
}

void UI::on_tracker_warning(const sha1_hash& hash, const Glib::ustring& msg)
{
	notify("Tracker warning", msg);
}

void UI::on_tracker_announce(const sha1_hash& hash, const Glib::ustring& msg)
{
	notify("Tracker announce", msg);
}

void UI::on_torrent_finished(const sha1_hash& hash, const Glib::ustring& msg)
{
	WeakPtr<Torrent> torrent = Engine::instance()->get_torrent_manager()->get_torrent(hash);
	notify("Torrent finished", torrent->get_name() + " is_complete");
}

void UI::on_file_error(const sha1_hash& hash, const Glib::ustring& msg)
{
	notify("File error", msg);
}

void UI::on_fastresume_rejected(const sha1_hash& hash, const Glib::ustring& msg)
{
	notify("Fastresume failed", msg);
}

void UI::on_hash_failed(const sha1_hash& hash, const Glib::ustring& msg, int piece)
{
	notify("Hash failed", msg);
}

void UI::on_peer_ban(const sha1_hash& hash, const Glib::ustring& msg, const Glib::ustring& ip)
{
	notify("Peer banned", msg);
}
