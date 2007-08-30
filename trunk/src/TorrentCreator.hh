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

#include <gtkmm/dialog.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/entry.h>
#include <gtkmm/button.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/filechooserbutton.h>
#include <glibmm/fileutils.h>

#include <libglademm.h>

#include "libtorrent/torrent_info.hpp"

class TorrentCreator : public Gtk::Dialog
{
	Glib::RefPtr<Gnome::Glade::Xml> glade_xml;

	class TorrentSaveDialog : public Gtk::FileChooserDialog
	{
	public:
		TorrentSaveDialog(Gtk::Window *parent);
		~TorrentSaveDialog();
	};

	class ModelColumns : public Gtk::TreeModelColumnRecord
	{
	public:
		ModelColumns()
			{
				add(size);
			}
		Gtk::TreeModelColumn<int> size;
	};
	ModelColumns columns;

	Gtk::Entry *entry_tracker, *entry_comment;
	Gtk::FileChooserButton *button_files;
	Gtk::CheckButton *check_seed, *check_private;
	Gtk::ComboBox *combo_pieces;
	Gtk::ProgressBar *progress_hashing;
	Gtk::RadioButton *radio_file, *radio_folder;

	TorrentSaveDialog* save_dialog;

	void add_files(libtorrent::torrent_info& info, const Glib::ustring& root, const Glib::ustring& child);

	void on_radio_toggled();

	bool on_delete_event(GdkEventAny*);

public:
	void run();

	TorrentCreator(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
	~TorrentCreator();
};

#endif
