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
#include <glibmm/i18n.h>
#include <libglademm.h>

#include <boost/date_time/posix_time/time_formatters.hpp>

#include "Statusbar.hh"
#include "PieceMap.hh"
#include "GroupList.hh"
#include "GroupsWin.hh"
#include "PeerList.hh"
#include "FileList.hh"
#include "SettingsWin.hh"
#include "TorrentCreator.hh"
#include "TorrentMenu.hh"
#include "StateFilter.hh"
#include "AddDialog.hh"
#include "EditColumnsDialog.hh"
#include "SessionClient.hh"
#include "UI.hh"

#include "linkage/Interface.hh"
#include "linkage/Engine.hh"
#include "linkage/AlertManager.hh"
#include "linkage/SessionManager.hh"
#include "linkage/TorrentManager.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/Utils.hh"
#include "linkage/compose.hpp"

/* dbus-c++ defines these ... */
#ifdef HAVE_CONFIG_H 
#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef VERSION

#include "config.h"
#endif

#if HAVE_GNOME
#include <libgnomevfsmm/utils.h>
#include <libgnomevfsmm/uri.h>
#endif

using namespace Linkage;

const char* TARGET_URI_LIST = "text/uri-list";
const char* TARGET_MOZ_URL = "text/x-moz-url-data";

//for adding separators to comboboxes, pretty retarded
static bool 
is_separator(const Glib::RefPtr<Gtk::TreeModel>& model, const Gtk::TreeIter& iter)
{
	Gtk::TreeRow row = *iter;
	Glib::ustring data;
	row.get_value(0, data);
	return (data == "-");
}

UI::UI(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::Window(cobject),
	glade_xml(refGlade)
{
	Engine::register_interface(this);

	try
	{
		set_icon(Gdk::Pixbuf::create_from_file(PIXMAP_DIR "/linkage.svg"));
	}
	catch (...) {}

	// get the widgets we work with
	glade_xml->get_widget_derived("settings_win", settings_win);
	settings_win->set_transient_for(*this);
	glade_xml->get_widget_derived("groups_win", groups_win);
	groups_win->set_transient_for(*this);
	glade_xml->get_widget_derived("add_dialog", add_dialog);
	add_dialog->set_transient_for(*this);
	glade_xml->get_widget_derived("new_dialog", new_dialog);
	new_dialog->set_transient_for(*this);
	glade_xml->get_widget_derived("columns_dialog", columns_dialog);
	columns_dialog->set_transient_for(*this);

	glade_xml->get_widget_derived("torrent_list", torrent_list);
	glade_xml->get_widget_derived("state_combobox", state_filter);
	glade_xml->get_widget_derived("groups_treeview1", group_list);
	glade_xml->get_widget_derived("statusbar", statusbar);
	glade_xml->get_widget_derived("piecemap", piecemap);
	glade_xml->get_widget_derived("file_list", file_list);
	glade_xml->get_widget_derived("peer_list", peer_list);
	glade_xml->get_widget_derived("torrent_menu", torrent_menu);
	glade_xml->get_widget_derived("combo_trackers", combo_trackers);
	glade_xml->get_widget_derived("menubar", menu);
	
	glade_xml->get_widget("main_vpane", main_vpane);
	main_vpane->set_position(-1);
	glade_xml->get_widget("main_hpane", main_hpane);

	glade_xml->get_widget("notebook_details", notebook_details);
	glade_xml->get_widget("expander_details", expander_details);
	glade_xml->get_widget("button_announce", button_announce);
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
	Gtk::Menu* sort_menu = 0;
	glade_xml->get_widget("sort_menu", sort_menu);

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

	tb_sort->set_menu(*Gtk::manage(sort_menu));
	// 	btn->signal_clicked().connect(sigc::mem_fun(this, &UI::on_sort));
	
	// set the sort image
	if (Engine::get_settings_manager()->get_string("ui/torrent_view/order") != "SORT_ASCENDING")
		tb_sort->set_stock_id(Gtk::Stock::SORT_DESCENDING);
	
	// torrent list signals
	torrent_list->signal_changed().connect(sigc::mem_fun(this, &UI::on_torrent_list_selection_changed));
	torrent_list->signal_double_click().connect(sigc::mem_fun(this, &UI::on_torrent_list_double_clicked));
	torrent_list->signal_right_click().connect(sigc::mem_fun(this, &UI::on_torrent_list_right_clicked));

	// setup drag and drop
	std::list<Gtk::TargetEntry> targets;
	targets.push_back(Gtk::TargetEntry(TARGET_URI_LIST));
	// FIXME: check target string from KTHML/WebKit/Dillo etc..
	targets.push_back(Gtk::TargetEntry(TARGET_MOZ_URL)); 
	torrent_list->drag_dest_set(targets);
	torrent_list->signal_drag_data_received().connect(sigc::mem_fun(this, &UI::on_dnd_received), false);

	columns_dialog->signal_visible_toggled().connect(sigc::mem_fun(torrent_list, &TorrentList::toggle_column));
	
	// hack to emit groups_changed signal
	groups_win->notify();

	// reselect from previous session
	state_filter->reselect();
	
	expander_details->property_expanded().signal_changed().connect(sigc::mem_fun(this, &UI::on_details_expanded));

	// setup the tracker combo and (re)announce button
	button_announce->signal_clicked().connect(sigc::mem_fun(this, &UI::on_announce_clicked));
	combo_trackers->signal_changed().connect(sigc::mem_fun(this, &UI::on_tracker_changed));
	combo_trackers->set_row_separator_func(sigc::ptr_fun(&is_separator));

	spinbutton_down->signal_value_changed().connect(sigc::mem_fun(this, &UI::on_spin_down));
	spinbutton_up->signal_value_changed().connect(sigc::mem_fun(this, &UI::on_spin_up));

	torrent_menu->signal_open().connect(sigc::mem_fun(this, &UI::on_open_location));
	torrent_menu->signal_edit_columns().connect(sigc::mem_fun(this, &UI::on_edit_columns));
	torrent_menu->signal_up().connect(sigc::mem_fun(this, &UI::on_up));
	torrent_menu->signal_down().connect(sigc::mem_fun(this, &UI::on_down));
	torrent_menu->signal_start().connect(sigc::mem_fun(this, &UI::on_start));
	torrent_menu->signal_stop().connect(sigc::mem_fun(this, &UI::on_stop));
	torrent_menu->signal_group().connect(sigc::mem_fun(this, &UI::on_set_group));
	torrent_menu->signal_remove().connect(sigc::mem_fun(this, &UI::on_remove));
	torrent_menu->signal_check().connect(sigc::mem_fun(this, &UI::on_check));

	show_all_children();

	SettingsManagerPtr sm = Engine::get_settings_manager();

	resize(sm->get_int("ui/win_width"), sm->get_int("ui/win_height"));

	notebook_details->set_current_page(sm->get_int("ui/page"));
	m_conn_switch_page = notebook_details->signal_switch_page().connect(
		sigc::mem_fun(this, &UI::on_switch_page));

	HashList list = torrent_list->get_selected_list();
	expander_details->set_sensitive(!list.empty());
	expander_details->set_expanded(sm->get_bool("ui/expanded"));
	// To make sure update_statics is run properly on startup
	if (expander_details->get_expanded())
		on_torrent_list_selection_changed();

	main_hpane->set_position(sm->get_int("ui/groups_width"));

	int max_up = sm->get_int("network/max_up_rate");
	if (max_up == 0)
		max_up = 1000;
	spinbutton_up->set_range(0, max_up);
	int max_down = sm->get_int("network/max_down_rate");
	if (max_down == 0)
		max_down = 1000;
	spinbutton_down->set_range(0, max_down);

	label_comment->set_single_line_mode(!sm->get_bool("ui/allow_linebreak_comments"));

	Engine::get_settings_manager()->signal_key_changed().connect(sigc::mem_fun(this, &UI::on_key_changed));

	m_conn_tick = Engine::signal_tick().connect(sigc::mem_fun(this, &UI::on_tick));

	add_events(Gdk::VISIBILITY_NOTIFY_MASK);

	// Set up desktop session support
	session_client = new SessionClient();
	session_client->signal_quit().connect(sigc::mem_fun(this, &UI::quit));
}

UI::~UI()
{
	// needs to be disconnect to avoid crash
	m_conn_switch_page.disconnect();

	SettingsManagerPtr sm = Engine::get_settings_manager();

	int w, h;
	get_size(w, h);

	sm->set("ui/win_width", w);
	sm->set("ui/win_height", h);
	sm->set("ui/page", notebook_details->get_current_page());
	sm->set("ui/expanded", expander_details->get_expanded());
	sm->set("ui/groups_width",main_hpane->get_position());

	// FIXME: sort out what auto-deleted through glade
	// seems like derived widgets must be deleted to get destructors in order
	// at least all top level widgets must be deleted!
	delete state_filter;
	delete torrent_list;
	delete piecemap;
	delete peer_list;
	delete file_list;
	delete settings_win;
	delete new_dialog;
	delete group_list;
	delete groups_win;
	delete torrent_menu;
	delete add_dialog;
	delete columns_dialog;
	
	delete session_client;
}

// Interface stuff
SelectionList UI::get_selected() const
{
	HashList hs = torrent_list->get_selected_list();
	SelectionList list;
	for (HashList::iterator iter = hs.begin(); iter != hs.end(); ++iter)
		list.push_back(Engine::get_torrent_manager()->get_torrent(*iter));

	return list;
}

bool UI::get_visible() const
{
	return is_visible();
}

void UI::set_visible(bool visible)
{
	if (visible)
	{
		deiconify();
		show();
	}
	else
		hide();
}

Gtk::Container* UI::get_container(Plugin::PluginParent parent) const
{
	switch (parent)
	{
		case Plugin::PARENT_DETAILS:
			return dynamic_cast<Gtk::Container*>(notebook_details);
		case Plugin::PARENT_MAIN:
			return dynamic_cast<Gtk::Container*>(glade_xml->get_widget("notebook_main"));
		case Plugin::PARENT_TOOLBAR:
			return dynamic_cast<Gtk::Container*>(glade_xml->get_widget("toolbar"));
		case Plugin::PARENT_MENU:
			return dynamic_cast<Gtk::Container*>(torrent_menu);
		case Plugin::PARENT_NONE:
		default:
			return NULL;
	}
}

void UI::open(const Glib::ustring& uri)
{
	set_visible(true);
	
	int response;
	if (!uri.empty())
		response = add_dialog->run_with_file(uri);
	else
		response = add_dialog->run();

	if (response == Gtk::RESPONSE_OK)
	{
		AddDialog::AddData data = add_dialog->get_data();
		Torrent::InfoPtr info = add_dialog->get_info();

		if (data.seed)
		{
			libtorrent::entry::dictionary_type er;
			er["path"] = data.path;
			er["downloaded"] = info->total_size();
			er["completed"] = true;
			save_entry(Glib::build_filename(get_data_dir(),
				String::compose("%1", info->info_hash()) + ".resume"), er);
		}

		/* FIXME: would make more sense to pass info instead of file */
		TorrentPtr torrent = Engine::get_session_manager()->open_torrent(data.file, data.path);

		if (data.name != info->name())
			torrent->set_name(data.name);
		if (!data.group.empty())
			torrent->set_group(data.group);
		for (unsigned int i = 0; i < data.filter.size(); i++)
		{
			if (data.filter[i])
				torrent->set_file_priority(i, 0);
		}

		//update(torrent, expander_details->get_expanded());
	}
	add_dialog->hide(); /* FIXME: needed? */
}

void UI::quit()
{
	Gtk::Main::quit();
}

void UI::notify(const Glib::ustring& title,
								const Glib::ustring& msg)
{
	statusbar->pop();
	statusbar->push(msg);
}

void UI::on_tick()
{
	/* FIXME: Only updates lists every 3rd tick, should be configurable */
	static int tick;
	tick = (tick + 1) % 3;

	if (expander_details->get_expanded())
		update(get_selected_single(), (tick == 0));

	torrent_list->update();

	group_list->update();

	libtorrent::session_status status = Engine::get_session_manager()->status();
	statusbar->set_status(status.num_peers, status.payload_download_rate, status.payload_upload_rate);
}

bool UI::on_visibility_notify_event(GdkEventVisibility* event)
{
	if (event->state == GDK_VISIBILITY_FULLY_OBSCURED)
		m_conn_tick.block();
	else
		m_conn_tick.unblock();

	return false;
}

void UI::on_hide()
{
	Gtk::Window::on_hide();
	m_conn_tick.block();
}

void UI::on_show()
{
	Gtk::Window::on_show();
	m_conn_tick.unblock();
}

void UI::update(const TorrentPtr& torrent, bool update_lists)
{
	libtorrent::torrent_status stats = torrent->get_status();

	if (expander_details->get_expanded())
	{
		libtorrent::size_type down = stats.total_payload_download +
			torrent->get_previously_downloaded();
		libtorrent::size_type up = stats.total_payload_upload +
			torrent->get_previously_uploaded();

		switch (notebook_details->get_current_page())
		{
			case PAGE_GENERAL:
			{
				if (stats.pieces)
					piecemap->set_map(*stats.pieces);
				else
					piecemap->set_map(std::vector<bool>(1, false));

				Glib::ustring tracker = combo_trackers->get_active_text();
				if (!tracker.empty())
					label_response->set_text(torrent->get_tracker_reply(tracker));
				else
					combo_trackers->set_active_text(stats.current_tracker);
				label_next_announce->set_text(boost::posix_time::to_simple_string(stats.next_announce));
				break;
			}
			case PAGE_PEERS:
			{
				label_down->set_text(suffix_value(down));
				label_down_rate->set_text(suffix_value(stats.download_payload_rate) + "/s");
				label_up->set_text(suffix_value(up));
				label_up_rate->set_text(suffix_value(stats.upload_payload_rate) + "/s");
				float ratio = 0;
				if (down)
					ratio = (1.0f*up)/(1.0f*down);
				label_ratio->set_text(String::ucompose("%1", std::fixed, std::setprecision(3), ratio));
				label_copies->set_text(String::ucompose("%1", std::fixed, std::setprecision(3), stats.distributed_copies));
				if (update_lists)
					peer_list->update(torrent);
				break;
			}
			case PAGE_FILES:
				if (update_lists)
					file_list->update(torrent);
				label_done->set_text(suffix_value(stats.total_done));
				label_remaining->set_text(suffix_value(stats.total_wanted - stats.total_wanted_done));
				break;

		}
	}
}

void UI::update_statics(const TorrentPtr& torrent)
{
	boost::intrusive_ptr<libtorrent::torrent_info> info = torrent->get_info();

	label_creator->set_text(info->creator());
	label_comment->set_text(info->comment());
	//FIXME: set tooltip to full comment

	if (info->creation_date())
		label_date->set_text(Glib::ustring(boost::posix_time::to_simple_string(*info->creation_date())));
	else
		label_date->set_text("");
	label_path->set_text(Glib::build_filename(torrent->get_path(), info->name()));
	label_size->set_text(suffix_value(info->total_size()));
	label_files->set_text(String::ucompose("%1", info->num_files()));
	label_pieces->set_text(String::ucompose("%1 x %2", info->num_pieces(), suffix_value(info->piece_length())));
	label_private->set_text(info->priv() ? _("Yes") : _("No"));
}

inline TorrentPtr UI::get_selected_single()
{
	HashList list = torrent_list->get_selected_list();

	g_assert(list.size() == 1);

	return Engine::get_torrent_manager()->get_torrent(*list.begin());
}

/* CALLBACKS */

void UI::on_spin_down()
{
	TorrentPtr torrent = get_selected_single();
	torrent->set_down_limit((int)spinbutton_down->get_value());
}

void UI::on_spin_up()
{
	TorrentPtr torrent = get_selected_single();
	torrent->set_up_limit((int)spinbutton_up->get_value());
}

void UI::on_key_changed(const Glib::ustring& key, const Value& value)
{
	if (key == "network/max_up_rate")
	{
		int max_up = value.get_int();
		if (max_up == 0)
			max_up = 10000;
		spinbutton_up->set_range(0, max_up);
	}
	else if (key == "network/max_down_rate")
	{
		int max_down = value.get_int();
		if (max_down == 0)
			max_down = 10000;
		spinbutton_down->set_range(0, max_down);
	}
	else if (key == "ui/allow_linebreak_comments")
		label_comment->set_single_line_mode(value.get_bool());
}

void UI::on_add()
{
	open();
}

void UI::on_remove(bool erase_content)
{
	HashList list = torrent_list->get_selected_list();

	for (HashList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		TorrentPtr torrent = Engine::get_torrent_manager()->get_torrent(*iter);

		if (erase_content)
		{
			Glib::ustring title = String::ucompose(
				_("Are you sure you wish to remove \"%1\" and it's content?"),
				torrent->get_name());
			Gtk::MessageDialog dialog(*this, title, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL);
			// FIXME: this is not so good for translations
			Glib::ustring files;
			int n = torrent->get_info()->num_files();
			if (n != 1)
				files = String::ucompose(_("(%1 files)."), n);
			else
				files = _("(1 file).");
			Glib::ustring msg = String::ucompose(_(
				"This will permanently remove the torrent and all of it's content %1"),
				files);
			dialog.set_secondary_text(msg);

			if (dialog.run() == Gtk::RESPONSE_OK)
				Engine::get_session_manager()->erase_torrent(torrent, erase_content);
		}
		else
			Engine::get_session_manager()->erase_torrent(torrent, erase_content);
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
		TorrentPtr torrent = Engine::get_torrent_manager()->get_torrent(hash);
		if (torrent->is_stopped() || torrent->get_state() & Torrent::ERROR)
		{
			Engine::get_session_manager()->resume_torrent(torrent);
			button_announce->set_sensitive(true);
		}
	}
	on_tick();
}

void UI::on_stop()
{
	HashList list = torrent_list->get_selected_list();

	for (HashList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		TorrentPtr torrent = Engine::get_torrent_manager()->get_torrent(*iter);
		Engine::get_session_manager()->stop_torrent(torrent);
	}

	if (!list.empty())
		button_announce->set_sensitive(false);

	on_tick();
}

void UI::on_up()
{
	HashList list = torrent_list->get_selected_list();

	for (HashList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		libtorrent::sha1_hash hash = *iter;
		TorrentPtr torrent = Engine::get_torrent_manager()->get_torrent(hash);
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
		TorrentPtr torrent = Engine::get_torrent_manager()->get_torrent(hash);
		unsigned int position = torrent->get_position();
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
		TorrentPtr torrent = Engine::get_torrent_manager()->get_torrent(*iter);
		torrent->set_group(group);
	}
	on_tick();
}

void UI::on_check()
{
	HashList list = torrent_list->get_selected_list();

	for (HashList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		TorrentPtr torrent = Engine::get_torrent_manager()->get_torrent(*iter);
		Engine::get_session_manager()->recheck_torrent(torrent);
	}
}

void UI::on_open_location()
{
	HashList list = torrent_list->get_selected_list();

	for (HashList::iterator iter = list.begin(); iter != list.end(); ++iter)
	{
		libtorrent::sha1_hash hash = *iter;
		TorrentPtr torrent = Engine::get_torrent_manager()->get_torrent(hash);
		Glib::ustring path = torrent->get_path();
		boost::intrusive_ptr<libtorrent::torrent_info> info = torrent->get_info();
		if (info->num_files() > 1)
			path = Glib::build_filename(path, info->name());

		session_client->open_location(path);
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
	quit();
	return false;
}

void UI::on_details_expanded()
{
	if (expander_details->get_expanded())
	{
		if(torrent_list->get_selected_list().size() == 1)
		{
			TorrentPtr torrent = get_selected_single();
			update(torrent, true);
			return;
		}
	}
	main_vpane->set_position(-1);
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

		TorrentPtr torrent = Engine::get_torrent_manager()->get_torrent(hash);
		spinbutton_down->set_value((double)torrent->get_down_limit());
		spinbutton_up->set_value((double)torrent->get_up_limit());

		combo_trackers->clear();
		combo_trackers->append_text(_("Add new tracker"));
		combo_trackers->append_text("-");
		std::vector<libtorrent::announce_entry> trackers = torrent->get_trackers();
		for (int i = 0; i <(int)trackers.size(); i++)
			combo_trackers->append_text(trackers[i].url);

		button_announce->set_sensitive(!torrent->is_stopped());

		label_response->set_text("");

		update_statics(torrent);
		update(torrent, expander_details->get_expanded());
	}
	else /* Multiple torrents selected */
	{
		expander_details->set_expanded(false);
		expander_details->set_sensitive(false);
		button_announce->set_sensitive(false);
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

void UI::on_announce_clicked()
{
	get_selected_single()->reannounce(combo_trackers->get_active_text());
}

void UI::on_tracker_changed()
{
	if (combo_trackers->get_active_row_number() == 0)
	{
		// FIXME: make an EntryDialog class for this
		Gtk::Dialog dialog(_("Add new tracker"), *this, true, true);
		dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		dialog.add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);

		Gtk::VBox* vbox = dialog.get_vbox();
		Gtk::HBox hbox;
		vbox->pack_start(hbox, false, false);
		Gtk::Label label(_("Tracker URL:"));
		hbox.pack_start(label, false, false);
		Gtk::Entry entry;
		hbox.pack_start(entry, true, true);
		vbox->show_all_children();

		if (dialog.run() == Gtk::RESPONSE_OK)
		{
			Glib::ustring tracker = entry.get_text();
			// FIXME: notify user if the entered url is rejected
			if (Glib::str_has_prefix(tracker, "http://") || Glib::str_has_prefix(tracker, "udp://"))
			{
				get_selected_single()->add_tracker(tracker);
				combo_trackers->append_text(tracker);
				combo_trackers->set_active_text(tracker);
			}
			else
				combo_trackers->set_active(-1);
		}
		else
			combo_trackers->set_active(-1);
	}
	else
	{
		Glib::ustring tracker = combo_trackers->get_active_text();
		TorrentPtr torrent = get_selected_single();
		label_response->set_text(torrent->get_tracker_reply(tracker));
	}
}

void UI::on_switch_page(GtkNotebookPage*, int page_num)
{
	TorrentPtr torrent = get_selected_single();
	update(torrent, true);
}

void UI::on_dnd_received(const Glib::RefPtr<Gdk::DragContext>& context,
	int x, int y,
	const Gtk::SelectionData& selection_data,
	guint info, guint time)
{
	// TreeModelFilter doesn't support DnD, this suppresses default handler/warning
	torrent_list->signal_drag_data_received().emission_stop();

	Glib::ustring data = selection_data.get_data_as_string();
	Glib::ustring target = selection_data.get_target();

	if (target == TARGET_URI_LIST)
	{
		std::list<Glib::ustring> uri_list;
		Glib::ustring::size_type pos, offset = 0;
		while ((pos = data.find("\r\n", offset)) != Glib::ustring::npos)
		{
			Glib::ustring s = data.substr(offset, pos);
			if (Glib::str_has_suffix(s, "\r\n"))
				s = s.substr(0, s.size() - 2);
			uri_list.push_back(s);
			offset = pos + 2;
		}

		for (std::list<Glib::ustring>::iterator iter = uri_list.begin();
			iter != uri_list.end(); ++iter)
		{
			bool is_url = false;
			#if HAVE_GNOME
			Glib::RefPtr<Gnome::Vfs::Uri> uri = Gnome::Vfs::Uri::create(*iter);
			is_url = !uri->is_local();
			if (!is_url)
				open(Gnome::Vfs::unescape_string(uri->get_path()));
			#else
			GError* error;
			gchar* f = g_filename_from_uri(iter->c_str(), NULL, &error);
			if (error)
			{
				is_url = (error->code == G_CONVERT_ERROR_BAD_URI);
				g_error_free(error);
			}
			else if (f)
			{
				open(f);
				g_free(f);
			}
			#endif

			// epiphany and firefox now sends TARGET_URI_LIST
			if (is_url)
			{
				std::string name = http_get(*iter);
				open(name);
				g_unlink(name.c_str());

			}
		}
	}
	else if (target == TARGET_MOZ_URL)
	{
		// seems like x-moz-url-data is in UTF-16
		GError* error = NULL;
		gchar* url = g_utf16_to_utf8((gunichar2*)selection_data.get_data(),
			(glong)selection_data.get_length(), NULL, NULL, &error);
		if (!url)
		{
			g_warning(error->message);
			g_error_free(error);
		}
		else
		{
			std::string name = http_get(url);
			open(name);
			g_unlink(name.c_str());
			g_free(url);
		}
	}

	context->drag_finish(true, false, time);
}

void UI::on_edit_columns()
{
	int response = columns_dialog->run(torrent_list->get_column_data());
}
