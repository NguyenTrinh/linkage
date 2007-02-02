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

#include <fstream>
#include <sys/stat.h>

#include <glib/gstdio.h>
#include <gtkmm/main.h>
#include <gtkmm/box.h>
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/stock.h>
#include <gtkmm/image.h>
#include <gtkmm/radiobuttongroup.h>
#include <gtkmm/messagedialog.h>

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/file.hpp"
#include "libtorrent/storage.hpp"
#include "libtorrent/hasher.hpp"

#include "TorrentCreator.hh"

TorrentCreator::TorrentCreator(Gtk::Window *parent)
{
	set_title("Create torrent");
	set_transient_for(*parent);
	set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
	set_skip_taskbar_hint(true);
	
	save_dialog = new TorrentSaveDialog(parent);
	
	Gtk::VBox* main_box = manage(new Gtk::VBox());
	
	Gtk::HBox* title_box = manage(new Gtk::HBox());
	Gtk::Image* image = manage(new Gtk::Image(Gtk::Stock::NEW, Gtk::ICON_SIZE_DIALOG));
	title_box->pack_start(*image, false, false);
	Gtk::Label* label = manage(new Gtk::Label("", 0.05, 0.5));
	label->set_use_markup(true);
	label->set_markup("<big><b>Create torrent</b></big>");
	title_box->pack_start(*label, true, true);
	
	main_box->pack_start(*title_box, false, false);
	
	Gtk::Table* table = manage(new Gtk::Table(7, 2));
	table->set_spacings(10);
	table->set_border_width(5);
	
	label = manage(new Gtk::Label("Tracker url:", 0.0, 0.5));
	table->attach(*label, 0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK);
	entry_tracker = manage(new Gtk::Entry());
	table->attach(*entry_tracker, 1, 2, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK);
	label = manage(new Gtk::Label("Comment:", 0.0, 0.5));
	table->attach(*label, 0, 1, 1, 2, Gtk::FILL, Gtk::SHRINK);
	entry_comment = manage(new Gtk::Entry());
	table->attach(*entry_comment, 1, 2, 1, 2, Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK);
	
	Gtk::RadioButtonGroup group;
	button_files = manage(new Gtk::FileChooserButton("Select file"));
	
	radio_file = manage(new Gtk::RadioButton("Single file"));
	radio_file->set_group(group);
	radio_file->signal_toggled().connect(sigc::mem_fun(this, &TorrentCreator::on_radio_toggled));
	table->attach(*radio_file, 0, 1, 2, 3, Gtk::FILL, Gtk::SHRINK);
	radio_folder = manage(new Gtk::RadioButton("Folder"));
	radio_folder->set_group(group);
	radio_folder->signal_toggled().connect(sigc::mem_fun(this, &TorrentCreator::on_radio_toggled));
	table->attach(*radio_folder, 1, 2, 2, 3, Gtk::FILL, Gtk::SHRINK);
	
	label = manage(new Gtk::Label("Piece size (kB):", 0.0, 0.5));
	table->attach(*label, 0, 1, 3, 4, Gtk::FILL, Gtk::SHRINK);
	combo_size = manage(new Gtk::ComboBoxText());
	//Do we need more? TODO: add user-defined option and auto detect
	combo_size->append_text("32");
	combo_size->append_text("64");
	combo_size->append_text("128");
	combo_size->append_text("256");
	combo_size->append_text("512");
	combo_size->append_text("1024");
	combo_size->append_text("2048");
	combo_size->set_active(3);
	table->attach(*combo_size, 1, 2, 3, 4, Gtk::FILL, Gtk::SHRINK);
	
	table->attach(*button_files, 0, 2, 4, 5, Gtk::FILL, Gtk::SHRINK);
	
	check_seed = manage(new Gtk::CheckButton("Open for seeding"));
	table->attach(*check_seed, 0, 2, 5, 6, Gtk::FILL, Gtk::SHRINK);
	
	progress_hashing = manage(new Gtk::ProgressBar());
	table->attach(*progress_hashing, 0, 2, 6, 7, Gtk::FILL, Gtk::SHRINK);
	
	main_box->pack_start(*table, true, true);
	
	Gtk::HBox* hbox = manage(new Gtk::HBox());
	button_save = manage(new Gtk::Button(Gtk::Stock::SAVE));
	button_save->signal_clicked().connect(sigc::mem_fun(this, &TorrentCreator::on_button_save));
	hbox->pack_end(*button_save, false, false);
	main_box->pack_start(*hbox, false, false);
	
	add(*main_box);
	
	show_all_children();
}

TorrentCreator::~TorrentCreator()
{
	delete save_dialog;
}

bool TorrentCreator::on_delete_event(GdkEventAny*)
{
	double frac = progress_hashing->get_fraction();
	/*Only hide if hashing is complete or if creation cancelled */
	if (frac == 1.0 || frac == 0.0)
		hide();
	return true;
}

void TorrentCreator::on_radio_toggled()
{
	if (button_files->get_filename() != "")
		button_files->unselect_all();
		
	if (radio_file->get_active())
	{ 
		button_files->set_action(Gtk::FILE_CHOOSER_ACTION_OPEN);
		button_files->set_title("Select file");
	}
	else if (radio_folder->get_active())
	{
		button_files->set_action(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
		button_files->set_title("Select folder");
	}
}

void TorrentCreator::on_button_save()
{
	Glib::ustring tracker = entry_tracker->get_text();
	Glib::ustring comment = entry_comment->get_text();
	Glib::ustring content = button_files->get_filename();
	
	unsigned int piece_size = std::atoi(combo_size->get_active_text().c_str())*1024;
	
	if (tracker == "" || content == "")
	{
		Gtk::MessageDialog dialog(*this, "Incomplete information");
		dialog.set_secondary_text("You must specify the tracker url and a file/folder.");
		dialog.run();
	}
	else
	{
		torrent_info info;
		
		Glib::ustring root = content.substr(0, content.rfind("/"));
		content.erase(0, root.size()+1);
		add_files(info, root, content);

		info.set_creator("Linkage / 0.17");
		info.set_piece_size(piece_size);
		info.add_tracker(tracker.c_str());
		info.set_comment(comment.c_str());
		
		progress_hashing->set_text("Hashing...");

		entry_tracker->set_sensitive(false);
		entry_comment->set_sensitive(false);
		radio_file->set_sensitive(false);
		radio_folder->set_sensitive(false);
		button_files->set_sensitive(false);
		combo_size->set_sensitive(false);
		check_seed->set_sensitive(false);
		button_save->set_sensitive(false);

		storage st(info, root.c_str());
		unsigned int num = info.num_pieces();
		std::vector<char> buf(piece_size);
		
		for (unsigned int i = 0; i < num; ++i)
		{
			double progress = (double)i/num;
			progress_hashing->set_fraction(progress);
			while (Gtk::Main::events_pending())
			 Gtk::Main::iteration(false);
			st.read(&buf[0], i, 0, info.piece_size(i));
			hasher h(&buf[0], info.piece_size(i));
			info.set_hash(i, h.final());
		}
		
		entry e = info.create_torrent();
		/* TODO: add private flag option
		e["private"] = true; 
		
		0.10 has set_priv()*/
		
		entry_tracker->set_sensitive(true);
		entry_comment->set_sensitive(true);
		radio_file->set_sensitive(true);
		radio_folder->set_sensitive(true);
		button_files->set_sensitive(true);
		combo_size->set_sensitive(true);
		check_seed->set_sensitive(true);
		button_save->set_sensitive(true);
		
		save_dialog->set_current_name(content + ".torrent");
		if (save_dialog->run() == Gtk::RESPONSE_OK)
		{
			Glib::ustring file = save_dialog->get_filename();
			if (file != "")
			{
				std::ofstream fout;
				fout.open(file.c_str(), std::ios_base::binary);
				bencode(std::ostream_iterator<char>(fout), e);
			}
		}
		save_dialog->hide();
		hide();
		entry_tracker->set_text("");
		entry_comment->set_text("");
		button_files->unselect_all();
		progress_hashing->set_text("");
		progress_hashing->set_fraction(0.0);
	}
}

void TorrentCreator::add_files(torrent_info& info, 
															 const Glib::ustring& root, 
															 const Glib::ustring& child)
{
	Glib::ustring path = root;
	if (root != child)
		path = Glib::build_filename(root, child);
	
	if (!Glib::file_test(path, Glib::FILE_TEST_IS_DIR))
	{
		struct stat results;
		if (g_stat(path.c_str(), &results) == 0)
			info.add_file(child.c_str(), results.st_size);
	}
	else
	{
		Glib::Dir dir(path);
		for (Glib::DirIterator iter = dir.begin(); iter != dir.end(); ++iter)
		{
			add_files(info, root, Glib::build_filename(child, *iter));
		}
	}
}

bool TorrentCreator::get_finished()
{
	double frac = progress_hashing->get_fraction();
	return (frac == 1.0 || frac == 0.0);
}

TorrentCreator::TorrentSaveDialog::TorrentSaveDialog(Gtk::Window *parent)
: Gtk::FileChooserDialog(*parent, "Save torrent", Gtk::FILE_CHOOSER_ACTION_SAVE)
{
	Gtk::Button *b = add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	b->signal_clicked().connect(sigc::mem_fun(this, &TorrentSaveDialog::hide));
	add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
}

TorrentCreator::TorrentSaveDialog::~TorrentSaveDialog()
{
}
