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

#ifndef TORRENTCREATOR_HH
#define TORRENTCREATOR_HH

#include <gtkmm/window.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/entry.h>
#include <gtkmm/button.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/filechooserbutton.h>
#include <glibmm/fileutils.h>

#include "libtorrent/torrent_info.hpp"

using namespace libtorrent;

class TorrentCreator : public Gtk::Window
{
	class TorrentSaveDialog : public Gtk::FileChooserDialog
	{
	public:
		TorrentSaveDialog(Gtk::Window *parent);
		virtual ~TorrentSaveDialog();
	};

	Gtk::Entry *entry_tracker, *entry_comment;
	Gtk::RadioButton *radio_file, *radio_folder;
	Gtk::FileChooserButton *button_files;
	Gtk::CheckButton *check_seed, *check_private;
	Gtk::ComboBoxText *combo_size;
	Gtk::ProgressBar *progress_hashing;
	
	TorrentSaveDialog *save_dialog;
	
	Gtk::Button* button_save;
	
	void add_files(torrent_info& info, const Glib::ustring& root, const Glib::ustring& child);

	void on_button_save();
	void on_radio_toggled();

	bool on_delete_event(GdkEventAny*);
	
public:
	bool get_finished();
	
	TorrentCreator(Gtk::Window *parent);
	virtual ~TorrentCreator();
};

#endif
