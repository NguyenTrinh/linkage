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

#ifndef UI_HH
#define UI_HH

#include <list>
#include <map>

#include <gtkmm/filefilter.h>
#include <gtkmm/textview.h>
#include <gtkmm/notebook.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/expander.h>
#include <gtkmm/messagedialog.h>
#include <gdkmm/cursor.h>
#include <gtkmm/menutoolbutton.h>
#include <gtkmm/paned.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/comboboxtext.h>

#include <libglademm.h>

#include "linkage/Torrent.hh"
#include "linkage/Plugin.hh"
#include "linkage/Interface.hh"

#include "TorrentList.hh"

class Statusbar;
class PieceMap;
class GroupList;
class GroupsWin;
class PeerList;
class FileList;
class SettingsWin;
class TorrentCreator;
class TorrentMenu;
class StateFilter;
class AddDialog;

class SessionClient;

class Value;

class UI : public Gtk::Window, public Interface
{
	//Hack to let us use comboboxtext with glade
	class ComboBoxTextGlade : public Gtk::ComboBoxText
	{
	public:
		ComboBoxTextGlade(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& xml)
			: Gtk::ComboBoxText(cobject) {}
	};

	Glib::RefPtr<Gnome::Glade::Xml> glade_xml;

	Gtk::MenuToolButton* tb_sort;

	Gtk::Button* button_add;
	Gtk::Button* button_remove;
	Gtk::Button* button_start;
	Gtk::Button* button_stop;
	Gtk::Button* button_up;
	Gtk::Button* button_down;
	Gtk::Button* button_announce;

	Gtk::Label* label_down;
	Gtk::Label* label_down_rate;
	Gtk::Label* label_up;
	Gtk::Label* label_up_rate;
	Gtk::Label* label_ratio;
	Gtk::Label* label_copies;
	Gtk::Label* label_path;
	Gtk::Label* label_creator;
	Gtk::Label* label_comment;
	Gtk::Label* label_size;
	Gtk::Label* label_pieces;
	Gtk::Label* label_done;
	Gtk::Label* label_remaining;
	Gtk::Label* label_next_announce;
	Gtk::Label* label_files;
	Gtk::Label* label_response;
	Gtk::Label* label_private;
	Gtk::Label* label_date;

	Gtk::VPaned* main_vpane;
	Gtk::HPaned* main_hpane;

	Gtk::Expander* expander_details;
	Gtk::Notebook* notebook_details;

	Gtk::SpinButton* spinbutton_down;
	Gtk::SpinButton* spinbutton_up;

	ComboBoxTextGlade* combo_trackers;
	TorrentMenu* torrent_menu;
	PieceMap* piecemap;
	TorrentList* torrent_list;
	GroupList* group_list;
	GroupsWin* groups_win;
	StateFilter* state_filter;
	FileList* file_list;
	PeerList* peer_list;
	AddDialog* add_dialog;
	TorrentCreator* new_dialog;
	Statusbar* statusbar;
	SettingsWin* settings_win;

	SessionClient* session_client;

	sigc::connection m_conn_tick;
	sigc::connection m_conn_switch_page;

	Glib::RefPtr<Gtk::WindowGroup> group;

	enum { PAGE_GENERAL, PAGE_PEERS, PAGE_FILES };

	bool on_visibility_notify_event(GdkEventVisibility* event);
	void on_hide();
	void on_show();
	void on_spin_down();
	void on_spin_up();

	void on_add();
	void on_remove(bool erase_content = false);
	void on_start();
	void on_stop();
	void on_up();
	void on_down();

	void on_new();
	void on_info();
	void on_prefs();
	void on_about();
	void on_quit();

	void on_view_groups_toggled();
	void on_main_hpane_changed();

	void on_open_location();
	void on_check();
	void on_set_group(const Glib::ustring& group);

	void on_details_expanded();

	void on_torrent_list_selection_changed();
	void on_torrent_list_double_clicked(GdkEventButton* event);
	void on_torrent_list_right_clicked(GdkEventButton* event);

	void on_tick();

	void on_sort_item_selected(TorrentList::Column col);
	void on_sort();

	void update(const Glib::RefPtr<Torrent>& torrent, bool update_lists = false);
	void update_statics(const Glib::RefPtr<Torrent>& torrent);

	bool on_delete_event(GdkEventAny*);

	void on_announce_clicked();
	void on_tracker_changed();

	void on_switch_page(GtkNotebookPage* child, int page);

	void on_dnd_received(const Glib::RefPtr<Gdk::DragContext>& context,
											 int x, int y,
											 const Gtk::SelectionData& selection_data,
											 guint info,
											 guint time);

	void notify(const Glib::ustring& title,
							const Glib::ustring& msg);

	void on_key_changed(const Glib::ustring& key, const Value& value);

	//asserts that num selected torrents are one only
	inline Glib::RefPtr<Torrent> get_selected_single();

public:
	// Interface stuff
	SelectionList get_selected()  const;
	bool get_visible()  const;
	void set_visible(bool visible);
	Gtk::Container* get_container(Plugin::PluginParent parent) const;
	void open(const Glib::ustring& uri = Glib::ustring());
	void quit();

	UI(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	~UI();
};

#endif /* UI_HH */
