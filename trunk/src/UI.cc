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

#include "config.h"

#include <curl/curl.h>
#include <string.h>
#include <glib/gstdio.h>

#include <iostream>

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

#include <libglademm.h>

#include "UI.hh"
#include "linkage/Engine.hh"
#include "linkage/Utils.hh"


UI::UI(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::Window(cobject),
	glade_xml(refGlade)
{
	Engine::register_interface(this);

	/* FIXME: Move somewhere more proper, it's just here to wake up manager and plugins */
	//Engine::get_plugin_manager();

	set_icon(Gdk::Pixbuf::create_from_file(PIXMAP_DIR "/linkage.svg"));

	//FIXME: merge all glade files to linkage.glade ?

	Glib::RefPtr<Gnome::Glade::Xml> settings_xml;
	try
	{
		settings_xml = Gnome::Glade::Xml::create(DATA_DIR "/settings.glade");
	}
	catch (const Gnome::Glade::XmlError& ex)
	{
		g_warning(ex.what().c_str());
	}
	settings_xml->get_widget_derived("settings_win", settings_win);
	settings_win->set_transient_for(*this);
	
	glade_xml->get_widget_derived("torrent_creator", torrent_win);
	torrent_win->set_transient_for(*this);

	Glib::RefPtr<Gnome::Glade::Xml> groups_xml;
	try
	{
		groups_xml = Gnome::Glade::Xml::create(DATA_DIR "/groups.glade");
	}
	catch (const Gnome::Glade::XmlError& ex)
	{
		g_warning(ex.what().c_str());
	}
	groups_xml->get_widget_derived("groups_win", groups_win);
	groups_win->set_transient_for(*this);

	menu_trackers = manage(new Gtk::Menu());
	
	// get the widgets we work with
	glade_xml->get_widget_derived("torrent_list", torrent_list);
	glade_xml->get_widget_derived("state_combobox", state_filter);
	glade_xml->get_widget_derived("groups_treeview", group_list);
	glade_xml->get_widget_derived("statusbar", statusbar);
	glade_xml->get_widget_derived("piecemap", piecemap);
	glade_xml->get_widget_derived("file_list", file_list);
	glade_xml->get_widget_derived("peer_list", peer_list);
	glade_xml->get_widget_derived("torrent_menu", torrent_menu);

	glade_xml->get_widget("main_vpane", main_vpane);
	main_vpane->set_position(-1);
	glade_xml->get_widget("main_hpane", main_hpane);

	glade_xml->get_widget("notebook_details", notebook_details);
	glade_xml->get_widget("expander_details", expander_details);
	glade_xml->get_widget("button_tracker", button_tracker);
	glade_xml->get_widget("label_tracker", label_tracker);
	glade_xml->get_widget("label_private", label_private);
	glade_xml->get_widget("label_response", label_response);
	glade_xml->get_widget("label_next_announce", label_next_announce);
	glade_xml->get_widget("label_creator", label_creator);
	glade_xml->get_widget("label_next_announce", label_next_announce);
	glade_xml->get_widget("label_comment", label_comment);
	glade_xml->get_widget("label_date", label_date);
	glade_xml->get_widget("label_down", label_down);
	glade_xml->get_widget("label_down_rate", label_down_rate);
	glade_xml->get_widget("label_up", label_up);
	glade_xml->get_widget("label_up_rate", label_up_rate);
	glade_xml->get_widget("spinbutton_down", spinbutton_down);
	glade_xml->get_widget("spinbutton_up", spinbutton_up);
	glade_xml->get_widget("label_copies", label_copies);
	glade_xml->get_widget("label_ratio", label_ratio);

	glade_xml->get_widget("label_files", label_files);
	glade_xml->get_widget("label_size", label_size);
	glade_xml->get_widget("label_done", label_done);
	glade_xml->get_widget("label_pieces", label_pieces);
	glade_xml->get_widget("label_remaining", label_remaining);

	glade_xml->get_widget("label_path", label_path);
	glade_xml->get_widget("toolb_sort", tb_sort);

 	// attach menu and toolbar signal handlers coz glademm sux:(
 	glade_xml->connect_clicked
 		("mnu_file_new", sigc::mem_fun(torrent_win, &TorrentCreator::show));
 	glade_xml->connect_clicked
 		("mnu_file_open", sigc::mem_fun(this, &UI::on_add));
	glade_xml->connect_clicked
		("mnu_file_quit", sigc::mem_fun(this, &UI::on_quit));
	glade_xml->connect_clicked
		("mnu_view_details", sigc::mem_fun(this, &UI::on_info));
	glade_xml->connect_clicked
		("mnu_edit_groups", sigc::mem_fun(groups_win, &GroupsWin::show));
	glade_xml->connect_clicked
		("mnu_view_prefs", sigc::mem_fun(this, &UI::on_prefs));
 	glade_xml->connect_clicked
 		("mnu_help_about", sigc::mem_fun(this, &UI::on_about));

	Gtk::ToolButton* toolb = 0;
	glade_xml->get_widget("toolb_add", toolb);
	toolb->signal_clicked().connect(sigc::mem_fun(this, &UI::on_add));
	glade_xml->get_widget("toolb_remove", toolb);
	toolb->signal_clicked().connect(sigc::bind(sigc::mem_fun(this, &UI::on_remove), false));
	glade_xml->get_widget("toolb_start", toolb);
	toolb->signal_clicked().connect(sigc::mem_fun(this, &UI::on_start));
	glade_xml->get_widget("toolb_stop", toolb);
	toolb->signal_clicked().connect(sigc::mem_fun(this, &UI::on_stop));
	glade_xml->get_widget("toolb_up", toolb);
	toolb->signal_clicked().connect(sigc::mem_fun(this, &UI::on_up));
	glade_xml->get_widget("toolb_down", toolb);
	toolb->signal_clicked().connect(sigc::mem_fun(this, &UI::on_down));
	tb_sort->signal_clicked().connect(sigc::mem_fun(this, &UI::on_sort));
	
	// to be uncommented when/if my patch makes it into libglademm
	/*
	glade_xml->connect_clicked
		("toolb_add", sigc::mem_fun(this, &UI::on_add));
	glade_xml->connect_clicked
		("toolb_remove", sigc::bind(sigc::mem_fun(this, &UI::on_remove), false));
	glade_xml->connect_clicked
		("toolb_start", sigc::mem_fun(this, &UI::on_start));
	glade_xml->connect_clicked
		("toolb_stop", sigc::mem_fun(this, &UI::on_stop));
	glade_xml->connect_clicked
		("toolb_up", sigc::mem_fun(this, &UI::on_up));
	glade_xml->connect_clicked
		("toolb_down", sigc::mem_fun(this, &UI::on_down));
	glade_xml->connect_clicked
		("toolb_sort", sigc::mem_fun(this, &UI::on_sort));
	*/

	// attach sort menu to button
	//Gtk::MenuToolButton* btn = 0;
	Gtk::Menu* menu = 0;
	glade_xml->get_widget("sort_menu", menu);

	// attach the sort menu signals
	glade_xml->connect_clicked
		("sort_menu_pos", sigc::bind(sigc::mem_fun
			(this, &UI::on_sort_item_selected), TorrentList::COL_POSITION));
	glade_xml->connect_clicked
		("sort_menu_name", sigc::bind(sigc::mem_fun
			(this, &UI::on_sort_item_selected), TorrentList::COL_NAME));
	glade_xml->connect_clicked
		("sort_menu_status", sigc::bind(sigc::mem_fun
			(this, &UI::on_sort_item_selected), TorrentList::COL_STATUS));
	glade_xml->connect_clicked
		("sort_menu_down", sigc::bind(sigc::mem_fun
			(this, &UI::on_sort_item_selected), TorrentList::COL_DOWNLOADED));
	glade_xml->connect_clicked
		("sort_menu_up", sigc::bind(sigc::mem_fun
			(this, &UI::on_sort_item_selected), TorrentList::COL_UPLOADED));
	glade_xml->connect_clicked
		("sort_menu_down_rate", sigc::bind(sigc::mem_fun
			(this, &UI::on_sort_item_selected), TorrentList::COL_DOWNRATE));
	glade_xml->connect_clicked
		("sort_menu_up_rate", sigc::bind(sigc::mem_fun
			(this, &UI::on_sort_item_selected), TorrentList::COL_UPRATE));
	glade_xml->connect_clicked
		("sort_menu_seeds", sigc::bind(sigc::mem_fun
			(this, &UI::on_sort_item_selected), TorrentList::COL_SEEDS));
	glade_xml->connect_clicked
		("sort_menu_peers", sigc::bind(sigc::mem_fun
			(this, &UI::on_sort_item_selected), TorrentList::COL_PEERS));
	glade_xml->connect_clicked
		("sort_menu_progress", sigc::bind(sigc::mem_fun
			(this, &UI::on_sort_item_selected), TorrentList::COL_PROGRESS));



	tb_sort->set_menu(*menu);
	// 	btn->signal_clicked().connect(sigc::mem_fun(this, &UI::on_sort));
	
	// set the sort image
	if (Engine::get_settings_manager()->get_int("ui/torrent_view/sort_order") != 0)
		tb_sort->set_stock_id(Gtk::Stock::SORT_DESCENDING);
	
	// torrent list signals
	torrent_list->signal_changed().connect(sigc::mem_fun(this, &UI::on_torrent_list_selection_changed));
	torrent_list->signal_double_click().connect(sigc::mem_fun(this, &UI::on_torrent_list_double_clicked));
	torrent_list->signal_right_click().connect(sigc::mem_fun(this, &UI::on_torrent_list_right_clicked));

	// setup drag and drop
	std::list<Gtk::TargetEntry> targets;
	targets.push_back(Gtk::TargetEntry("STRING", Gtk::TargetFlags(0)));
	targets.push_back(Gtk::TargetEntry("text/plain", Gtk::TargetFlags(0)));
	targets.push_back(Gtk::TargetEntry("text/uri-list", Gtk::TargetFlags(0)));
	torrent_list->drag_dest_set(targets, Gtk::DEST_DEFAULT_ALL, Gdk::DragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE));
	torrent_list->signal_drag_data_received().connect(sigc::mem_fun(this, &UI::on_dnd_received));

	/* setup group and state stuff, this is ugly and should be changed */
	groups_win->signal_groups_changed().connect(sigc::mem_fun(group_list, &GroupList::on_groups_changed));
	groups_win->signal_groups_changed().connect(sigc::mem_fun(torrent_menu, &TorrentMenu::on_groups_changed));
	group_list->signal_filter_set().connect(sigc::mem_fun(torrent_list, &TorrentList::on_filter_set));
	state_filter->signal_state_filter_changed().connect(sigc::mem_fun(group_list, &GroupList::on_state_filter_changed));
	state_filter->signal_state_filter_changed().connect(sigc::mem_fun(torrent_list, &TorrentList::on_state_filter_changed));

	// hack to emit groups_changed signal
	groups_win->notify();

	// reselect from previous session
	state_filter->reselect();
	
	expander_details->property_expanded().signal_changed().connect(sigc::mem_fun(this, &UI::on_details_expanded));

	// setup the tracker button
	button_tracker->add_events(Gdk::BUTTON_RELEASE_MASK);
	button_tracker->signal_button_release_event().connect(sigc::mem_fun(this, &UI::on_tracker_update), false);
	button_tracker->signal_enter().connect(sigc::mem_fun(this, &UI::on_tracker_enter));
	button_tracker->signal_leave().connect(sigc::mem_fun(this, &UI::on_tracker_leave));

	spinbutton_down->signal_value_changed().connect(sigc::mem_fun(this, &UI::on_spin_down));
	spinbutton_up->signal_value_changed().connect(sigc::mem_fun(this, &UI::on_spin_up));

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

	Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();

	resize(sm->get_int("ui/win_width"), sm->get_int("ui/win_width"));

	notebook_details->set_current_page(sm->get_int("ui/page"));
	/* this makes expander insensitive even if we have a valid selection,
			if we shutdown with expander not expanded */
	expander_details->set_sensitive(sm->get_bool("ui/expanded"));
	expander_details->set_expanded(sm->get_bool("ui/expanded"));
	Gtk::HPaned* hpan;
	glade_xml->get_widget("main_hpane",hpan);
	hpan->set_position(sm->get_int("ui/groups_width"));

	int max_up = sm->get_int("network/max_up_rate");
	if (max_up == 0)
		max_up = 1000;
	spinbutton_up->set_range(0, max_up);
	int max_down = sm->get_int("network/max_down_rate");
	if (max_down == 0)
		max_down = 1000;
	spinbutton_down->set_range(0, max_down);

	Engine::get_settings_manager()->signal_update_settings().connect(sigc::mem_fun(this, &UI::on_settings));

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

	sm->set("ui/win_width", w);
	sm->set("ui/win_height", h);
	sm->set("ui/page", notebook_details->get_current_page());
	sm->set("ui/expanded", expander_details->get_expanded());
	sm->set("ui/groups_width",main_hpane->get_position());

	// FIXME: sort out what auto-deleted through glade
	// seems like derived widgets must be deleted to get destructors in order
	delete state_filter;
	delete torrent_list;
	delete piecemap;
	delete peer_list;
	delete file_list;
	delete settings_win;
	delete torrent_win;
	delete group_list;
	delete groups_win;
	delete torrent_menu;
	delete file_chooser;
	delete path_chooser;
}

Gtk::Container* UI::get_container(Plugin::PluginParent parent)
{
	switch (parent)
	{
		case Plugin::PARENT_DETAILS:
			return dynamic_cast<Gtk::Container*>(notebook_details);
		case Plugin::PARENT_MAIN:
			return dynamic_cast<Gtk::Container*>(notebook_main);
		case Plugin::PARENT_TOOLBAR:
			return dynamic_cast<Gtk::Container*>(manager->get_widget("/ToolBar"));
		case Plugin::PARENT_MENU:
			return dynamic_cast<Gtk::Container*>(torrent_menu);
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

	libtorrent::session_status status = Engine::get_session_manager()->status();
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

	libtorrent::torrent_status stats = torrent->get_status();

	torrent_list->update(torrent);

	if (selected && expander_details->get_expanded())
	{
		libtorrent::size_type down = torrent->get_total_downloaded();
		libtorrent::size_type up = torrent->get_total_uploaded();
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
	libtorrent::torrent_info info = torrent->get_info();

	label_creator->set_text(info.creator());
	label_comment->set_text(info.comment());

	if (info.creation_date())
		label_date->set_text(Glib::ustring(to_simple_string(*info.creation_date())) );
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

	std::vector<libtorrent::announce_entry> trackers = torrent->get_handle().trackers();

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
	if (!Engine::get_settings_manager()->get_bool("files/use_default_path"))
	{
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
	}
	else
		save_path = Engine::get_settings_manager()->get_string("files/default_path");

	libtorrent::sha1_hash hash = Engine::get_session_manager()->open_torrent(file, save_path);
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
	people.push_back("Dave Moore");
	about.set_authors(people);
	people.clear();
	people.push_back("Brian William Davis");
	people.push_back("Ludvig Aleman");
	about.set_artists(people);
	people.clear();
	about.set_comments("A BitTorrent client");
	about.set_logo(Gdk::Pixbuf::create_from_file(PIXMAP_DIR "/linkage.svg"));
	about.set_version(PACKAGE_VERSION);
	about.set_copyright("Copyright \u00A9 2006-2007 Christian Lundgren and Dave Moore");
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
	int max_up = Engine::get_settings_manager()->get_int("network/max_up_rate");
	if (max_up == 0)
		max_up = 10000;
	spinbutton_up->set_range(0, max_up);
	int max_down = Engine::get_settings_manager()->get_int("network/max_down_rate");
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
		libtorrent::sha1_hash hash = *iter;

		if (erase_content)
		{
			WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
			Glib::ustring title = "Are you sure you wish to remove \"" +
				torrent->get_name() + "\" and it's content?";
			Gtk::MessageDialog dialog(*this, title, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
			Glib::ustring files;
			int n = torrent->get_info().num_files();
			if (n != 1)
				files = "(" + str(n) + " files).";
			else
				files = "(1 file).";
			Glib::ustring msg = "This will permanently remove the torrent and all of it's content " + files;
			dialog.set_secondary_text(msg);

			if (dialog.run() == Gtk::RESPONSE_OK)
				Engine::get_session_manager()->erase_torrent(hash, erase_content);
		}
		else
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
		libtorrent::sha1_hash hash = *iter;
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
		libtorrent::sha1_hash hash = *iter;
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
		libtorrent::sha1_hash hash = *iter;
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
		libtorrent::sha1_hash hash = *iter;
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
		libtorrent::sha1_hash hash = *iter;
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
		libtorrent::sha1_hash hash = *iter;
		Engine::get_session_manager()->recheck_torrent(hash);
	}
}

void UI::on_open_location()
{
	HashList list = torrent_list->get_selected_list();

	for (HashList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		libtorrent::sha1_hash hash = *iter;
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
		main_vpane->set_position(-1);
	}
}

void UI::on_torrent_list_selection_changed()
{
	HashList list = torrent_list->get_selected_list();
	if (list.size() == 1)
	{
		libtorrent::sha1_hash hash = *list.begin();

		expander_details->set_sensitive(true);
		if (Engine::get_settings_manager()->get_bool("ui/auto_expand"))
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
		libtorrent::sha1_hash hash = *list.begin();
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
		libtorrent::sha1_hash hash = *list.begin();
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

void UI::on_duplicate_torrent(const Glib::ustring& msg, const libtorrent::sha1_hash& hash)
{
	notify("Duplicate torrent", msg);
}

void UI::on_listen_failed(const Glib::ustring& msg)
{
	notify("Listen failed", msg);
}

void UI::on_tracker_failed(const libtorrent::sha1_hash& hash, const Glib::ustring& msg, int code, int times)
{
	notify("Tracker failed", msg);
}

void UI::on_tracker_reply(const libtorrent::sha1_hash& hash, const Glib::ustring& msg, int peers)
{
	notify("Tracker response", msg);
}

void UI::on_tracker_warning(const libtorrent::sha1_hash& hash, const Glib::ustring& msg)
{
	notify("Tracker warning", msg);
}

void UI::on_tracker_announce(const libtorrent::sha1_hash& hash, const Glib::ustring& msg)
{
	notify("Tracker announce", msg);
}

void UI::on_torrent_finished(const libtorrent::sha1_hash& hash, const Glib::ustring& msg)
{
	WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
	notify("Torrent finished", torrent->get_name() + " is_complete");
}

void UI::on_file_error(const libtorrent::sha1_hash& hash, const Glib::ustring& msg)
{
	notify("File error", msg);
}

void UI::on_fastresume_rejected(const libtorrent::sha1_hash& hash, const Glib::ustring& msg)
{
	notify("Fastresume failed", msg);
}

void UI::on_hash_failed(const libtorrent::sha1_hash& hash, const Glib::ustring& msg, int piece)
{
	notify("Hash failed", msg);
}

void UI::on_peer_ban(const libtorrent::sha1_hash& hash, const Glib::ustring& msg, const Glib::ustring& ip)
{
	notify("Peer banned", msg);
}
