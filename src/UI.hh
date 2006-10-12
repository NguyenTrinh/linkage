/*
Copyright (C) 2006  Christian Lundgren

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifndef UI_HH
#define UI_HH

#include <list>
#include <map>

#include <gtkmm/statusbar.h>
#include <gtkmm/filefilter.h>
#include <gtkmm/textview.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/notebook.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/expander.h>
#include <gtkmm/label.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/messagedialog.h>
#include <gdkmm/cursor.h>

#include "PieceMap.hh"
#include "linkage/SessionManager.hh"
#include "TorrentList.hh"
#include "PeerList.hh"
#include "FileList.hh"
#include "SettingsWin.hh"
#include "TorrentCreator.hh"
#include "linkage/PluginManager.hh"

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

class UI : public Gtk::Window
{
  Glib::RefPtr<Gtk::ActionGroup> action_group;
  Glib::RefPtr<Gtk::UIManager> manager;
  
  Gtk::Statusbar* statusbar;
  
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
  Gtk::Label* label_time_elapsed;
  Gtk::Label* label_time_eta;
  Gtk::Label* label_ratio;
  Gtk::Label* label_seeds;
  Gtk::Label* label_peers;
  Gtk::Label* label_wasted;
  Gtk::Button* button_tracker; //TOD: Gtkmm 2.10 has Gtk::LinkButton
  Gtk::Label* label_save_path;
  Gtk::Label* label_creator;
  Gtk::Label* label_comment;
  Gtk::Label* label_size;
  Gtk::Label* label_pieces;
  Gtk::Label* label_piece_size;
  Gtk::Label* label_next_announce;
  Gtk::Label* label_files;
  Gtk::Label* label_response;
  Gtk::Label* label_progress;
  
  Gtk::Menu* menu_trackers;
  
  Gtk::Expander* expander_details;
  Gtk::Notebook* notebook_main;
  Gtk::Notebook* notebook_details;
  
  Gtk::SpinButton* spinbutton_down;
  Gtk::SpinButton* spinbutton_up;
  
  OpenDialog* file_chooser;
  SaveDialog* path_chooser;
  
  PieceMap* piecemap;
  TorrentList* torrent_list;
  FileList* file_list;
  PeerList* peer_list;
  
  SettingsWin* settings_win;
  TorrentCreator* torrent_win;
  
  sigc::connection connection_switch_page;  /* This must be disconnected before UI is destroy to avoid segfault */
  
  enum { PAGE_INFO, PAGE_STATUS, PAGE_PEERS, PAGE_FILES };
  
  friend class FileList;
  friend class IOManager;
  
protected:
  void on_spin_down();
  void on_spin_up();
  
  void on_add();
  void on_remove();
  void on_start();
  void on_stop();
  void on_up();
  void on_down();
  
  void on_new();
  void on_info();
  void on_prefs();
  void on_about();
  void on_quit();
  
  void on_details_expanded();
  
  void on_torrent_list_selection_changed();
  
  void on_torrent_added(Torrent* torrent);

  bool on_timeout();

  void update(Torrent* torrent);
  
  virtual bool on_delete_event(GdkEventAny*);
  
  bool on_tracker_update(GdkEventButton* e);
  void on_tracker_enter();
  void on_tracker_leave();
  
  void on_switch_page(GtkNotebookPage* child, int page);
  
  void save_state();
  void reset_views();
  
  void build_tracker_menu(Torrent* torrent);
  void on_popup_tracker_selected(const Glib::ustring& tracker, int tier);
  
  void add_torrent(const Glib::ustring& file);
  void on_dnd_received(const Glib::RefPtr<Gdk::DragContext>& context, 
                       int x, int y, 
                       const Gtk::SelectionData& selection_data, 
                       guint info, 
                       guint time);
                       
  void notify(const Glib::ustring& title, 
              const Glib::ustring& msg,
              NotifyType type,
              Torrent* torrent = 0);
  
  void on_plugin_load(Plugin* plugin);
  void on_plugin_unload(Plugin* plugin);
  void on_add_widget(Plugin* plugin, Gtk::Widget* widget, Plugin::PluginParent parent);
  
  bool on_toggle_visible();
  
  void on_settings();
  
  static UI* smInstance;
  
  void on_invalid_bencoding(const Glib::ustring& msg, const Glib::ustring& file);
  void on_missing_file(const Glib::ustring& msg, const Glib::ustring& file);
  void on_duplicate_torrent(const Glib::ustring& msg, const sha1_hash& hash);

  void on_listen_failed(const Glib::ustring& msg);
  void on_tracker_failed(const sha1_hash& hash, const Glib::ustring& msg, int code, int times);
  void on_tracker_reply(const sha1_hash& hash, const Glib::ustring& msg);
  void on_tracker_warning(const sha1_hash& hash, const Glib::ustring& msg);
  void on_tracker_announce(const sha1_hash& hash, const Glib::ustring& msg);
  void on_torrent_finished(const sha1_hash& hash, const Glib::ustring& msg);
  void on_file_error(const sha1_hash& hash, const Glib::ustring& msg);
  void on_fastresume_rejected(const sha1_hash& hash, const Glib::ustring& msg);
  void on_hash_failed(const sha1_hash& hash, const Glib::ustring& msg, int piece);
  void on_peer_ban(const sha1_hash& hash, const Glib::ustring& msg, const Glib::ustring& ip);
  
public: 
  static UI* instance();
  static void goodnight();
  
  UI();
  ~UI();
};

#endif /* UI_HH */
