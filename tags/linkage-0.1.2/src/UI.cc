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

#include "config.h"

#include <curl/curl.h>
#include <string.h>
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
#include <gtkmm/aboutdialog.h>

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
												"			<menuitem action='Details'/>"
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
	set_title("Linkage");
	set_icon(Gdk::Pixbuf::create_from_file(PIXMAP_DIR "/linkage.svg"));

	settings_win = new SettingsWin(this);
	torrent_win = new TorrentCreator(this);

	menu_trackers = manage(new Gtk::Menu());

	action_group = Gtk::ActionGroup::create();

	action_group->add(Gtk::Action::create("FileMenu", "_File"));
	action_group->add(Gtk::Action::create("ViewMenu", "_View"));
	action_group->add(Gtk::Action::create("HelpMenu", "_Help"));

	action_group->add(Gtk::Action::create("New", Gtk::Stock::NEW, "_New", "Create a new torrent file"),
										Gtk::AccelKey("<control>N"),
										sigc::mem_fun(torrent_win, &TorrentCreator::show));
	action_group->add(Gtk::Action::create("Open", Gtk::Stock::OPEN, "_Open", "Open a torrent file"),
										Gtk::AccelKey("<control>O"),
										sigc::mem_fun(this, &UI::on_add));
	action_group->add(Gtk::Action::create("Quit", Gtk::Stock::QUIT, "_Quit", "Quit"),
										Gtk::AccelKey("<control>Q"),
										sigc::mem_fun(this, &UI::on_quit));
	action_group->add(Gtk::Action::create("Details", Gtk::Stock::DIALOG_INFO, "_Details", "Show detailed information"),
										sigc::mem_fun(this, &UI::on_info));
	action_group->add(Gtk::Action::create("Preferences", Gtk::Stock::PREFERENCES, "Prefere_nces", "Configure linkage"),
										sigc::mem_fun(this, &UI::on_prefs));
	action_group->add(Gtk::Action::create("About", Gtk::Stock::ABOUT, "_About", "About"),
										sigc::mem_fun(this, &UI::on_about));

	action_group->add(Gtk::Action::create("Add", Gtk::Stock::ADD, "Add", "Add torrent"),
										sigc::mem_fun(this, &UI::on_add));
	action_group->add(Gtk::Action::create("Remove", Gtk::Stock::REMOVE, "Remove", "Remove selected torrents"),
										sigc::bind(sigc::mem_fun(this, &UI::on_remove), false));
	action_group->add(Gtk::Action::create("Start", Gtk::Stock::APPLY, "Start", "Start selected torrents"),
										sigc::mem_fun(this, &UI::on_start));
	action_group->add(Gtk::Action::create("Stop", Gtk::Stock::STOP, "Stop", "Stop selected torrents"),
										sigc::mem_fun(this, &UI::on_stop));
	action_group->add(Gtk::Action::create("Up", Gtk::Stock::GO_UP, "Up", "Move selected torrents up"),
										sigc::mem_fun(this, &UI::on_up));
	action_group->add(Gtk::Action::create("Down", Gtk::Stock::GO_DOWN, "Down", "Move selected torrents down"),
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

	switch (Engine::get_settings_manager()->get_int("UI", "SortOrder"))
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
	Gtk::MenuItem* item = manage(new Gtk::MenuItem("Position"));
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
	group_list = new GroupList();
	torrent_list->set_filter_set_signal(group_list->signal_filter_set());
	torrent_list->set_filter_unset_signal(group_list->signal_filter_unset());
	Gtk::HPaned* hpan = manage(new Gtk::HPaned());
	hpan->pack1(*group_list, false, true);
	hpan->pack2(*scrollwin, true, true);

	notebook_main->append_page(*hpan, "Torrents");
	Gtk::VPaned* vpan = manage(new Gtk::VPaned());
	main_vbox->pack_start(*vpan, true, true, 0);
	vpan->pack1(*notebook_main, true, true);

	expander_details = manage(new Gtk::Expander("<b>Details</b>"));
	expander_details->set_use_markup(true);
	expander_details->property_expanded().signal_changed().connect(sigc::mem_fun(this, &UI::on_details_expanded));
	vpan->pack2(*expander_details, false, true);

	notebook_details = manage(new Gtk::Notebook());
	connection_switch_page = notebook_details->signal_switch_page().connect(sigc::mem_fun(this, &UI::on_switch_page));
	expander_details->add(*notebook_details);

	Gtk::VBox* general_box = manage(new Gtk::VBox());

	Gtk::Frame* frame_pieces = manage(new Gtk::Frame());
	frame_pieces->set_shadow_type(Gtk::SHADOW_NONE);
	Gtk::Label* label = manage(new AlignedLabel());
	label->set_use_markup(true);
	label->set_markup("<b>Pieces</b>");
	frame_pieces->set_label_widget(*label);

	piecemap = new PieceMap();

	frame_pieces->add(*piecemap);
	general_box->pack_start(*frame_pieces, false, false, 0);

	Gtk::Frame* frame_tracker = manage(new Gtk::Frame());
	frame_tracker->set_shadow_type(Gtk::SHADOW_NONE);
	label = manage(new AlignedLabel());
	label->set_use_markup(true);
	label->set_markup("<b>Tracker</b>");
	frame_tracker->set_label_widget(*label);

	Gtk::Table* table_tracker = manage(new Gtk::Table(2, 4));
	table_tracker->set_spacings(10);
	table_tracker->set_border_width(5);
	label = manage(new AlignedLabel("Tracker:"));
	table_tracker->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
	label = manage(new AlignedLabel("Next announce:"));
	table_tracker->attach(*label, 2, 3, 0, 1, Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
	label = manage(new AlignedLabel("Response:"));
	table_tracker->attach(*label, 0, 1, 1, 2, Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
	label = manage(new AlignedLabel("Private:"));
	table_tracker->attach(*label, 2, 3, 1, 2, Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
	button_tracker = manage(new Gtk::Button());
	label_tracker = manage(new AlignedLabel());
	label_tracker->set_use_markup(true);
	button_tracker->add(*label_tracker);
	button_tracker->set_relief(Gtk::RELIEF_NONE);
	button_tracker->add_events(Gdk::BUTTON_RELEASE_MASK);
	button_tracker->signal_button_release_event().connect(sigc::mem_fun(this, &UI::on_tracker_update), false);
	button_tracker->signal_enter().connect(sigc::mem_fun(this, &UI::on_tracker_enter));
	button_tracker->signal_leave().connect(sigc::mem_fun(this, &UI::on_tracker_leave));
	Gtk::HBox* tracker_box = manage(new Gtk::HBox());
	tracker_box->pack_start(*button_tracker, false, false);
	table_tracker->attach(*tracker_box, 1, 2, 0, 1, Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
	label_next_announce = manage(new AlignedLabel());
	table_tracker->attach(*label_next_announce, 3, 4, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
	label_response = manage(new AlignedLabel());
	label_response->set_ellipsize(Pango::ELLIPSIZE_END);
	table_tracker->attach(*label_response, 1, 2, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
	label_private = manage(new AlignedLabel());
	table_tracker->attach(*label_private, 3, 4, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::EXPAND|Gtk::FILL);
	
	frame_tracker->add(*table_tracker);
	general_box->pack_start(*frame_tracker, false, false, 0);

	Gtk::Frame* frame_origin = manage(new Gtk::Frame());
	frame_origin->set_shadow_type(Gtk::SHADOW_NONE);
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
	label_comment->set_ellipsize(Pango::ELLIPSIZE_END);
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
	label = manage(new AlignedLabel("Seen copies:"));
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
	label_copies = manage(new AlignedLabel());
	table_transfer->attach(*label_copies, 7, 8, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);

	transfer_box->pack_start(*table_transfer, false, false, 0);

	scrollwin = manage(new Gtk::ScrolledWindow());
	scrollwin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	peer_list = new PeerList();
	scrollwin->add(*peer_list);
	transfer_box->pack_start(*scrollwin, true, true, 0);
	notebook_details->append_page(*transfer_box, "Transfer");

	Gtk::VBox* files_box = manage(new Gtk::VBox());

	Gtk::Table* table_files = manage(new Gtk::Table(2, 10));
	table_files->set_spacings(10);
	table_files->set_border_width(5);
	label = manage(new AlignedLabel("Files:"));
	table_files->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Total size:"));
	table_files->attach(*label, 2, 3, 0, 1, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Done:"));
	table_files->attach(*label, 4, 5, 0, 1, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Remaining:"));
	table_files->attach(*label, 6, 7, 0, 1, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Pieces:"));
	table_files->attach(*label, 8, 9, 0, 1, Gtk::FILL, Gtk::SHRINK);
	label = manage(new AlignedLabel("Saving as:"));
	table_files->attach(*label, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK);
	label_files = manage(new AlignedLabel());
	table_files->attach(*label_files, 1, 2, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);
	label_size = manage(new AlignedLabel());
	table_files->attach(*label_size, 3, 4, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);
	label_done = manage(new AlignedLabel());
	table_files->attach(*label_done, 5, 6, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);
	label_remaining = manage(new AlignedLabel());
	table_files->attach(*label_remaining, 7, 8, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);
	label_pieces = manage(new AlignedLabel());
	table_files->attach(*label_pieces, 9, 10, 0, 1, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);
	label_path = manage(new AlignedLabel());
	label_path->set_ellipsize(Pango::ELLIPSIZE_END);
	table_files->attach(*label_path, 1, 10, 1, 2, Gtk::EXPAND|Gtk::FILL, Gtk::SHRINK);

	files_box->pack_start(*table_files, false, false, 0);

	scrollwin = manage(new Gtk::ScrolledWindow());
	scrollwin->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	file_list = new FileList();
	scrollwin->add(*file_list);
	files_box->pack_start(*scrollwin, true, true, 0);
	notebook_details->append_page(*files_box, "Files");

	statusbar = manage(new Statusbar());
	main_vbox->pack_start(*statusbar, false, false, 0);
	
	torrent_menu = new TorrentMenu();
	torrent_menu->signal_open().connect(sigc::mem_fun(this, &UI::on_open_location));
	torrent_menu->signal_info().connect(sigc::mem_fun(this, &UI::on_info));
	torrent_menu->signal_up().connect(sigc::mem_fun(this, &UI::on_up));
	torrent_menu->signal_down().connect(sigc::mem_fun(this, &UI::on_down));
	torrent_menu->signal_start().connect(sigc::mem_fun(this, &UI::on_start));
	torrent_menu->signal_stop().connect(sigc::mem_fun(this, &UI::on_stop));
	torrent_menu->signal_group().connect(sigc::mem_fun(this, &UI::on_set_group));
	torrent_menu->signal_remove().connect(sigc::mem_fun(this, &UI::on_remove));
	torrent_menu->signal_check().connect(sigc::mem_fun(this, &UI::on_check));

	show_all_children();

	file_chooser = new OpenDialog(this);
	path_chooser = new SaveDialog(this);

	Engine::get_session_manager()->resume_session();

	Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();

	resize(sm->get_int("UI", "WinWidth"), sm->get_int("UI", "WinHeight"));

	notebook_details->set_current_page(sm->get_int("UI", "Page"));
	/* this makes expander insensitive even if we have a valid selection,
			if we shutdown with expander not expanded */
	expander_details->set_sensitive(sm->get_bool("UI", "Expanded"));
	expander_details->set_expanded(sm->get_bool("UI", "Expanded"));

	hpan->set_position(sm->get_int("UI", "GroupsWidth"));

	int max_up = sm->get_int("Network", "MaxUpRate");
	if (max_up == 0)
		max_up = 1000;
	spinbutton_up->set_range(0, max_up);
	int max_down = sm->get_int("Network", "MaxDownRate");
	if (max_down == 0)
		max_down = 1000;
	spinbutton_down->set_range(0, max_down);

	Engine::get_settings_manager()->signal_update_settings().connect(sigc::mem_fun(this, &UI::on_settings));

	Engine::get_plugin_manager()->signal_plugin_load().connect(sigc::mem_fun(this, &UI::on_plugin_load));
	Engine::get_plugin_manager()->signal_plugin_unload().connect(sigc::mem_fun(this, &UI::on_plugin_unload));
	Engine::get_plugin_manager()->signal_add_widget().connect(sigc::mem_fun(this, &UI::on_add_widget));

	Engine::get_session_manager()->signal_invalid_bencoding().connect(sigc::mem_fun(this, &UI::on_invalid_bencoding));
	Engine::get_session_manager()->signal_missing_file().connect(sigc::mem_fun(this, &UI::on_missing_file));
	Engine::get_session_manager()->signal_duplicate_torrent().connect(sigc::mem_fun(this, &UI::on_duplicate_torrent));

	Engine::get_alert_manager()->signal_listen_failed().connect(sigc::mem_fun(this, &UI::on_listen_failed));
	Engine::get_alert_manager()->signal_tracker_failed().connect(sigc::mem_fun(this, &UI::on_tracker_failed));
	Engine::get_alert_manager()->signal_tracker_reply().connect(sigc::mem_fun(this, &UI::on_tracker_reply));
	Engine::get_alert_manager()->signal_tracker_warning().connect(sigc::mem_fun(this, &UI::on_tracker_warning));
	Engine::get_alert_manager()->signal_tracker_announce().connect(sigc::mem_fun(this, &UI::on_tracker_announce));
	Engine::get_alert_manager()->signal_torrent_finished().connect(sigc::mem_fun(this, &UI::on_torrent_finished));
	Engine::get_alert_manager()->signal_file_error().connect(sigc::mem_fun(this, &UI::on_file_error));
	Engine::get_alert_manager()->signal_fastresume_rejected().connect(sigc::mem_fun(this, &UI::on_fastresume_rejected));
	Engine::get_alert_manager()->signal_hash_failed().connect(sigc::mem_fun(this, &UI::on_hash_failed));
	Engine::get_alert_manager()->signal_peer_ban().connect(sigc::mem_fun(this, &UI::on_peer_ban));

	Engine::get_dbus_manager()->signal_open().connect(sigc::mem_fun(this, &UI::add_torrent));
	Engine::get_dbus_manager()->signal_quit().connect(sigc::mem_fun(this, &UI::on_quit));
	Engine::get_dbus_manager()->signal_toggle_visible().connect(sigc::mem_fun(this, &UI::on_toggle_visible));

	connection_tick = Engine::signal_tick().connect(sigc::mem_fun(this, &UI::on_tick));
}

UI::~UI()
{
	connection_switch_page.disconnect();

	Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();

	int w, h;
	get_size(w, h);

	sm->set("UI", "WinWidth", w);
	sm->set("UI", "WinHeight", h);
	sm->set("UI", "Page", notebook_details->get_current_page());
	sm->set("UI", "Expanded", expander_details->get_expanded());
	Gtk::HPaned* hpan = dynamic_cast<Gtk::HPaned*>(group_list->get_parent());
	sm->set("UI", "GroupsWidth",hpan->get_position());

	delete torrent_list;
	delete piecemap;
	delete peer_list;
	delete file_list;
	delete settings_win;
	delete torrent_win;
	delete group_list;
	delete torrent_menu;
	delete file_chooser;
	delete path_chooser;
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

	TorrentManager::TorrentList torrents = Engine::get_torrent_manager()->get_torrents();
	for (TorrentManager::TorrentList::iterator iter = torrents.begin();
				iter != torrents.end(); ++iter)
	{
		if (*iter)
			update(*iter, (tick == 0));
	}

	group_list->update();

	session_status status = Engine::get_session_manager()->status();
	statusbar->set_status(status.num_peers, status.payload_download_rate, status.payload_upload_rate);
}

bool UI::on_visibility_notify_event(GdkEventVisibility* event)
{
	if (event->state == GDK_VISIBILITY_FULLY_OBSCURED)
		connection_tick.block();
	else
		connection_tick.unblock();

	return false;
}

void UI::update(const WeakPtr<Torrent>& torrent, bool update_lists)
{
	bool selected = torrent_list->is_selected(torrent->get_hash());

	torrent_status stats = torrent->get_status();

	torrent_list->update(torrent);

	if (selected && expander_details->get_expanded())
	{
		size_type down = torrent->get_total_downloaded();
		size_type up = torrent->get_total_uploaded();
		float ratio = 0;
		switch (notebook_details->get_current_page())
		{
			case PAGE_GENERAL:
			{
				if (stats.pieces)
					piecemap->set_map(*stats.pieces);
				else
					piecemap->set_map(std::vector<bool>(1, false));
				std::pair<Glib::ustring, Glib::ustring> p = torrent->get_tracker_reply();
				label_tracker->set_text(p.first);
				label_response->set_text(p.second);
				label_next_announce->set_text(to_simple_string(stats.next_announce));
				break;
			}
			case PAGE_PEERS:
				label_down->set_text(suffix_value(down));
				label_down_rate->set_text(suffix_value(stats.download_payload_rate) + "/s");
				label_up->set_text(suffix_value(up));
				label_up_rate->set_text(suffix_value(stats.upload_payload_rate) + "/s");
				if (down)
					ratio = (1.0f*up)/(1.0f*down);
				label_ratio->set_text(str(ratio, 3));
				if (stats.distributed_copies != -1)
					label_copies->set_text(str(stats.distributed_copies, 3));
				else
					label_copies->set_text("-");
				if (update_lists)
					peer_list->update(torrent);
				break;
			case PAGE_FILES:
				if (update_lists)
					file_list->update(torrent);
				label_done->set_text(suffix_value(stats.total_done));
				label_remaining->set_text(suffix_value(stats.total_wanted - stats.total_wanted_done));
				break;

		}
	}
}

void UI::update_statics(const WeakPtr<Torrent>& torrent)
{
	torrent_info info = torrent->get_info();

	label_creator->set_text(info.creator());
	label_comment->set_text(info.comment());
	if (info.creation_date())
		label_date->set_text(to_simple_string(*info.creation_date()));
	else
		label_date->set_text("");
	label_path->set_text(Glib::build_filename(torrent->get_path(), info.name()));
	label_size->set_text(suffix_value(info.total_size()));
	label_files->set_text(str(info.num_files()));
	label_pieces->set_text(str(info.num_pieces()) + " x " + suffix_value(info.piece_length()));
	label_private->set_text(info.priv() ? "Yes" : "No");
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
			this, &UI::on_popup_tracker_selected), tracker));
		menu_trackers->append(*item);
	}
	menu_trackers->show_all_children();
}

void UI::add_torrent(const Glib::ustring& file)
{
	if (!Glib::file_test(file, Glib::FILE_TEST_EXISTS))
	{
		Engine::get_session_manager()->signal_missing_file().emit("File not found, \"" + file + "\"", file);
		return;
	}

	if (!is_visible())
		show();

	Glib::ustring save_path;
	Glib::ustring name = file.substr(file.rfind("/") + 1, file.size());
	path_chooser->set_title("Select path for " + name);
	if (!Engine::get_settings_manager()->get_bool("Files", "UseDefaultPath"))
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
		save_path = Engine::get_settings_manager()->get_string("Files", "DefaultPath");

	sha1_hash hash = Engine::get_session_manager()->open_torrent(file, save_path);
	WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
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
	Gtk::AboutDialog about;
	std::list<Glib::ustring> people;
	people.push_back("Christian Lundgren");
	about.set_authors(people);
	people.clear();
	people.push_back("Brian William Davis");
	people.push_back("Ludvig Aleman");
	about.set_artists(people);
	people.clear();
	about.set_comments("A BitTorrent client");
	about.set_logo(Gdk::Pixbuf::create_from_file(PIXMAP_DIR "/linkage.svg"));
	about.set_version(PACKAGE_VERSION);
	about.set_copyright("Copyright \u00A9 2006-2007 Christian Lundgren");
	about.set_name("Linkage");
	about.run();
}

void UI::on_spin_down()
{
	HashList list = torrent_list->get_selected_list();
	if (list.size() == 1) /* FIXME: This check _shouldn't_ be/(isn't?) needed! */
	{
		WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(*list.begin());
		torrent->set_down_limit((int)spinbutton_down->get_value());
	}
}

void UI::on_spin_up()
{
	HashList list = torrent_list->get_selected_list();
	if (list.size() == 1) /* FIXME: This check _shouldn't_ be/(isn't?) needed! */
	{
		WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(*list.begin());
		torrent->set_up_limit((int)spinbutton_up->get_value());
	}
}

void UI::on_settings()
{
	int max_up = Engine::get_settings_manager()->get_int("Network", "MaxUpRate");
	if (max_up == 0)
		max_up = 10000;
	spinbutton_up->set_range(0, max_up);
	int max_down = Engine::get_settings_manager()->get_int("Network", "MaxDownRate");
	if (max_down == 0)
		max_down = 10000;
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

void UI::on_remove(bool erase_content)
{
	HashList list = torrent_list->get_selected_list();

	for (HashList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		sha1_hash hash = *iter;
		Engine::get_session_manager()->erase_torrent(hash, erase_content);
	}

	if (!list.empty())
	{
		expander_details->set_expanded(false);
		expander_details->set_sensitive(false);
	}
	on_tick();
}

void UI::on_start()
{
	HashList list = torrent_list->get_selected_list();

	for (HashList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		sha1_hash hash = *iter;
		WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
		if (torrent->is_stopped())
		{
			Engine::get_session_manager()->resume_torrent(hash);
			button_tracker->set_sensitive(true);
		}
	}
	on_tick();
}

void UI::on_stop()
{
	HashList list = torrent_list->get_selected_list();

	for (HashList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		sha1_hash hash = *iter;
		Engine::get_session_manager()->stop_torrent(hash);
	}

	if (!list.empty())
		button_tracker->set_sensitive(false);

	on_tick();
}

void UI::on_up()
{
	HashList list = torrent_list->get_selected_list();

	for (HashList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		sha1_hash hash = *iter;
		WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
		int position = torrent->get_position();
		if (position > 1)
			torrent->set_position(position - 1);
	}
	on_tick();
}

void UI::on_down()
{
	HashList list = torrent_list->get_selected_list();

	for (HashList::reverse_iterator iter = list.rbegin(); iter != list.rend(); ++iter)
	{
		sha1_hash hash = *iter;
		WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
		int position = torrent->get_position();
		if (position < Engine::get_torrent_manager()->get_torrents_count())
			torrent->set_position(position + 1);
	}
	on_tick();
}

void UI::on_set_group(const Glib::ustring& group)
{
	HashList list = torrent_list->get_selected_list();

	for (HashList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		sha1_hash hash = *iter;
		WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
		torrent->set_group(group);
	}
	on_tick();
}

void UI::on_check()
{
	HashList list = torrent_list->get_selected_list();

	for (HashList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		sha1_hash hash = *iter;
		Engine::get_session_manager()->recheck_torrent(hash);
	}
}

void UI::on_open_location()
{
	HashList list = torrent_list->get_selected_list();

	for (HashList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		sha1_hash hash = *iter;
		WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
		Glib::ustring path = torrent->get_path();
		if (torrent->get_info().num_files() > 1)
			path = Glib::build_filename(path, torrent->get_name());

		Glib::ustring cmd = "nautilus --no-desktop \"" + path + "\"";
		Glib::spawn_command_line_async(cmd);
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
		Gtk::Main::quit();
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
			WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(*list.begin());
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
		if (Engine::get_settings_manager()->get_bool("UI", "AutoExpand"))
			expander_details->set_expanded(true);

		WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
		spinbutton_down->set_value((double)torrent->get_down_limit());
		spinbutton_up->set_value((double)torrent->get_up_limit());

		button_tracker->set_sensitive(!torrent->is_stopped());

		update_statics(torrent);
		update(torrent, expander_details->get_expanded());
	}
	else /* Multiple torrents selected */
	{
		expander_details->set_expanded(false);
		expander_details->set_sensitive(false);
	}
}

void UI::on_torrent_list_double_clicked(GdkEventButton* event)
{
	if (expander_details->is_sensitive())
		expander_details->set_expanded(!expander_details->get_expanded());
}

void UI::on_torrent_list_right_clicked(GdkEventButton* event)
{
	if (!torrent_list->get_selected_list().empty())
		torrent_menu->popup(event->button, event->time);
}

bool UI::on_tracker_update(GdkEventButton* e)
{
	HashList list = torrent_list->get_selected_list();
	if (list.size() == 1)
	{
		sha1_hash hash = *list.begin();
		WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
		Torrent::State state = torrent->get_state();
		if (state == Torrent::DOWNLOADING || state == Torrent::ANNOUNCING ||
				state == Torrent::SEEDING || state == Torrent::FINISHED)
		{
			switch (e->button)
			{
				case 1:
					/* TODO: add timeout to prevent hammering */
					torrent->reannounce();
					break;
				case 3:
					build_tracker_menu(torrent);
					menu_trackers->popup(e->button, e->time);
					break;
			}
		}
	}
	return false;
}

void UI::on_popup_tracker_selected(const Glib::ustring& tracker)
{
	/* TODO: Save info to next session */
	HashList list = torrent_list->get_selected_list();
	if (list.size() == 1)
	{
		sha1_hash hash = *list.begin();
		WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
		torrent->reannounce(tracker);
	}
}

void UI::on_tracker_enter()
{
	button_tracker->set_relief(Gtk::RELIEF_NORMAL);
}

void UI::on_tracker_leave()
{
	button_tracker->set_relief(Gtk::RELIEF_NONE);
}

void UI::on_switch_page(GtkNotebookPage*, int page_num)
{
	HashList list = torrent_list->get_selected_list();
	if (list.size() == 1)
	{
		WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(*list.begin());
		update(torrent, true);
	}
}

void UI::on_dnd_received(const Glib::RefPtr<Gdk::DragContext>& context,
												 int x, int y,
												 const Gtk::SelectionData& selection_data,
												 guint info, guint time)
{
	Glib::ustring data = selection_data.get_data_as_string();
	Glib::StringArrayHandle a = context->get_targets();

	bool is_file_uri = false;
	for (Glib::StringArrayHandle::iterator ai = a.begin(); ai != a.end(); ++ai)
		if (*ai == "text/uri-list")
			is_file_uri = true;

	if (is_file_uri)
	{
		std::list<Glib::ustring> uri_list;
		int pos, offset = 0;
		while ((pos = data.find("\r\n", offset)) != Glib::ustring::npos)
		{
			Glib::ustring s = data.substr(offset, pos);
			if (Glib::str_has_suffix(s, "\r\n"))
				s = s.substr(0, s.size() - 2);
			uri_list.push_back(s);
			offset = pos + 2;
		}

		for (std::list<Glib::ustring>::iterator li = uri_list.begin(); li != uri_list.end(); ++li)
		{
			gchar* f = g_filename_from_uri(li->c_str(), NULL, NULL);
			if (f)
				add_torrent(f);
		}
	}
	else if (Glib::str_has_prefix(data, "http://"))
	{
		/* TODO: Add a progress bar */	
		notify("Downloading torrent", "downloading " + data);
 
		CURL *curl;
		CURLcode res;
		char err[CURL_ERROR_SIZE];

		gchar* name;
		GError* error;
		gint fd = g_file_open_tmp("torrent-XXXXXX", &name, &error);
		if (fd == -1)
		{
			g_warning(error->message);
			g_error_free(error);
		}
		else
		{
			FILE* file = fdopen(fd, "wb");

			curl = curl_easy_init();
			if (curl && file)
			{
				curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, err);
				curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
				curl_easy_setopt(curl, CURLOPT_HEADER, 0);
				curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

				curl_easy_setopt(curl, CURLOPT_URL, data.c_str());

				int res = curl_easy_perform(curl);

				curl_easy_cleanup(curl);

				fclose(file);

				if (res == CURLE_OK)
					add_torrent(name);
				else
					notify("Download failed", err);

				g_unlink(name);
				g_free(name);
			}
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
	WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
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