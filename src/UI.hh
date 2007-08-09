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
#include <gtkmm/uimanager.h>
#include <gtkmm/notebook.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/expander.h>
#include <gtkmm/messagedialog.h>
#include <gdkmm/cursor.h>
#include <gtkmm/menutoolbutton.h>
#include <gtkmm/paned.h>

#include <libglademm.h>

#if HAVE_GNOME
#include <libgnomeuimm/client.h>
#endif

#if HAVE_EXO
#include <exo/exo.h>
#endif

#include "linkage/Torrent.hh"
#include "linkage/WeakPtr.hh"
#include "linkage/Plugin.hh"
#include "linkage/Interface.hh"

#include "Statusbar.hh"
#include "PieceMap.hh"
#include "TorrentList.hh"
#include "GroupList.hh"
#include "GroupsWin.hh"
#include "PeerList.hh"
#include "FileList.hh"
#include "SettingsWin.hh"
#include "TorrentCreator.hh"
#include "TorrentMenu.hh"
#include "StateFilter.hh"

class OpenDialog : public Gtk::FileChooserDialog
{
	Gtk::FileFilter *torrent_filter;
	Gtk::FileFilter *no_filter;

public:
	OpenDialog(Gtk::Window *parent);
	virtual ~OpenDialog();
};

class SaveDialog : public Gtk::FileChooserDialog
{
public:
	SaveDialog(Gtk::Window *parent);
	virtual ~SaveDialog();
};

class UI : public Gtk::Window, public Interface
{
	Glib::RefPtr<Gtk::ActionGroup> action_group;
	Glib::RefPtr<Gtk::UIManager> manager;
	Glib::RefPtr<Gnome::Glade::Xml> glade_xml;

	Gtk::MenuToolButton* tb_sort;

	Statusbar* statusbar;

	Gtk::Button* button_add;
	Gtk::Button* button_remove;
	Gtk::Button* button_start;
	Gtk::Button* button_stop;
	Gtk::Button* button_up;
	Gtk::Button* button_down;

	Gtk::Label* label_down;
	Gtk::Label* label_down_rate;
	Gtk::Label* label_up;
	Gtk::Label* label_up_rate;
	Gtk::Label* label_ratio;
	Gtk::Label* label_copies;
	Gtk::Button* button_tracker;
	Gtk::Label* label_tracker;
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

	Gtk::Menu* menu_trackers;

	Gtk::VPaned* main_vpane;
	Gtk::HPaned* main_hpane;

	Gtk::Expander* expander_details;
	Gtk::Notebook* notebook_main;
	Gtk::Notebook* notebook_details;

	Gtk::SpinButton* spinbutton_down;
	Gtk::SpinButton* spinbutton_up;

	OpenDialog* file_chooser;
	SaveDialog* path_chooser;

	TorrentMenu* torrent_menu;

	PieceMap* piecemap;
	TorrentList* torrent_list;
	GroupList* group_list;
	GroupsWin* groups_win;
	StateFilter* state_filter;
	FileList* file_list;
	PeerList* peer_list;

	SettingsWin* settings_win;
	TorrentCreator* torrent_win;

	sigc::connection m_conn_tick;
	sigc::connection m_conn_switch_page;

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

	void update(const WeakPtr<Torrent>& torrent, bool update_lists = false);
	void update_statics(const WeakPtr<Torrent>& torrent);

	bool on_delete_event(GdkEventAny*);

	bool on_tracker_update(GdkEventButton* e);
	void on_tracker_enter();
	void on_tracker_leave();

	void on_switch_page(GtkNotebookPage* child, int page);

	void build_tracker_menu(const WeakPtr<Torrent>& torrent);
	void on_popup_tracker_selected(const Glib::ustring& tracker);

	void add_torrent(const Glib::ustring& file);
	void on_dnd_received(const Glib::RefPtr<Gdk::DragContext>& context,
											 int x, int y,
											 const Gtk::SelectionData& selection_data,
											 guint info,
											 guint time);

	void notify(const Glib::ustring& title,
							const Glib::ustring& msg);

	void on_toggle_visible();

	void on_settings();

	void on_invalid_bencoding(const Glib::ustring& msg, const Glib::ustring& file);
	void on_missing_file(const Glib::ustring& msg, const Glib::ustring& file);
	void on_duplicate_torrent(const Glib::ustring& msg, const libtorrent::sha1_hash& hash);

	void on_listen_failed(const Glib::ustring& msg);
	void on_tracker_failed(const libtorrent::sha1_hash& hash, const Glib::ustring& msg, int code, int times);
	void on_tracker_reply(const libtorrent::sha1_hash& hash, const Glib::ustring& msg, int peers);
	void on_tracker_warning(const libtorrent::sha1_hash& hash, const Glib::ustring& msg);
	void on_tracker_announce(const libtorrent::sha1_hash& hash, const Glib::ustring& msg);
	void on_torrent_finished(const libtorrent::sha1_hash& hash, const Glib::ustring& msg);
	void on_file_error(const libtorrent::sha1_hash& hash, const Glib::ustring& msg);
	void on_fastresume_rejected(const libtorrent::sha1_hash& hash, const Glib::ustring& msg);
	void on_hash_failed(const libtorrent::sha1_hash& hash, const Glib::ustring& msg, int piece);
	void on_peer_ban(const libtorrent::sha1_hash& hash, const Glib::ustring& msg, const Glib::ustring& ip);

	#if HAVE_GNOME
	void on_die_gnome();
	bool on_save_yourself_gnome(int phase, Gnome::UI::SaveStyle save_style,
		bool shutdown, Gnome::UI::InteractStyle interact_style, bool fast);
	#endif

	#if HAVE_EXO
	ExoXsessionClient* exo_client;
	static void on_save_yourself_exo(ExoXsessionClient *client, gpointer data);
	#endif
 
public:
	// Interface stuff
	HashList get_selected_list();
	bool get_visible();
	void set_visible(bool visible);
	Gtk::Container* get_container(Plugin::PluginParent parent);
	void open(const Glib::ustring& uri);
	void quit();

	UI(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	~UI();
};

#endif /* UI_HH */
