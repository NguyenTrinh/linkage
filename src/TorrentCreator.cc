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

#include <glib/gstdio.h>
#include <gtkmm/main.h>
#include <gtkmm/box.h>
#include <gtkmm/table.h>
#include <gtkmm/label.h>
#include <gtkmm/stock.h>
#include <gtkmm/image.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodel.h>
#include <glibmm/i18n.h>

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/file.hpp"
#include "libtorrent/storage.hpp"
#include "libtorrent/hasher.hpp"
#include "libtorrent/file_pool.hpp"

#include "TorrentCreator.hh"

#include "linkage/Engine.hh"
#include "linkage/SessionManager.hh"
#include "linkage/Utils.hh"
#include "linkage/compose.hpp"

using namespace Linkage;

TorrentCreator::TorrentCreator(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::Dialog(cobject),
		glade_xml(refGlade)
{
	save_dialog = new TorrentSaveDialog(this);

	glade_xml->get_widget("new_radio_file", radio_file);
	radio_file->signal_toggled().connect(sigc::mem_fun(this, &TorrentCreator::on_radio_toggled));
	glade_xml->get_widget("new_radio_folder", radio_folder);
	radio_folder->signal_toggled().connect(sigc::mem_fun(this, &TorrentCreator::on_radio_toggled));
	
	glade_xml->get_widget("new_entry_tracker", entry_tracker);
	glade_xml->get_widget("new_entry_comment", entry_comment);
	glade_xml->get_widget("new_filechooser_content", button_files);
	glade_xml->get_widget("new_check_seed", check_seed);
	glade_xml->get_widget("new_check_private", check_private);
	glade_xml->get_widget("new_combo_pieces", combo_pieces);
	glade_xml->get_widget("new_progressbar", progress_hashing);

	Glib::RefPtr<Gtk::ListStore> model = Gtk::ListStore::create(columns);
	combo_pieces->set_model(model);
	for (int i = 64; i <= 4096; i *= 2)
	{
		Gtk::TreeRow row = *(model->append());
		row[columns.size] = i;
	}
	combo_pieces->set_active(2);
}

TorrentCreator::~TorrentCreator()
{
	delete save_dialog;
}

bool TorrentCreator::on_delete_event(GdkEventAny*)
{
	double frac = progress_hashing->get_fraction();
	// Only hide if hashing is complete or if creation cancelled
	if (frac == 1.0 || frac == 0.0)
		hide();
	return true;
}

void TorrentCreator::on_radio_toggled()
{
	if (!button_files->get_filename().empty())
		button_files->unselect_all();

	bool is_file_type = radio_file->get_active();
	if (is_file_type)
	{
		button_files->set_action(Gtk::FILE_CHOOSER_ACTION_OPEN);
		button_files->set_title(_("Select file"));
	}
	else
	{
		button_files->set_action(Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
		button_files->set_title(_("Select folder"));
	}
}

void TorrentCreator::run()
{
	using namespace libtorrent;

	int response = Gtk::Dialog::run();
	if (response != Gtk::RESPONSE_ACCEPT)
		return;

	Glib::ustring tracker = entry_tracker->get_text();
	Glib::ustring comment = entry_comment->get_text();
	Glib::ustring content = button_files->get_filename();
	bool priv = check_private->get_active();
	Gtk::TreeRow row = *(combo_pieces->get_active());
	int piece_size = row[columns.size] * 1024;
	
	if (tracker.empty() || content.empty())
	{
		Gtk::MessageDialog dialog(*this, _("Incomplete information"));
		dialog.set_secondary_text(_("You must specify the tracker url and a file/folder."));
		dialog.run();
	}
	else
	{
		boost::intrusive_ptr<torrent_info> info(new torrent_info());
		
		Glib::ustring root = content.substr(0, content.rfind("/"));
		content.erase(0, root.size()+1);
		add_files(info, root, content);

		info->set_creator(PACKAGE_NAME "/" PACKAGE_VERSION);
		info->set_piece_size(piece_size);
		info->add_tracker(tracker.c_str());
		info->set_comment(comment.c_str());
		info->set_priv(priv);

		progress_hashing->set_text(_("Hashing..."));

		file_pool fp;
		boost::scoped_ptr<storage_interface> st(default_storage_constructor(info, root.c_str(), fp));

		std::vector<char> buf(piece_size);
		
		for (int i = 0; i < info->num_pieces(); i++)
		{
			double progress = (double)i/info->num_pieces();
			progress_hashing->set_fraction(progress);
			while (Gtk::Main::events_pending())
				Gtk::Main::iteration(false);
			st->read(&buf[0], i, 0, info->piece_size(i));
			hasher h(&buf[0], info->piece_size(i));
			info->set_hash(i, h.final());
		}
		
		entry e = info->create_torrent();
		
		save_dialog->set_current_name(content + ".torrent");
		if (save_dialog->run() == Gtk::RESPONSE_OK)
		{
			save_dialog->hide();
			Glib::ustring file = save_dialog->get_filename();
			if (!file.empty())
			{
				std::ofstream fout;
				fout.open(file.c_str(), std::ios_base::binary);
				bencode(std::ostream_iterator<char>(fout), e);
				fout.close();

				if (check_seed->get_active())
				{
					entry::dictionary_type er;
					er["path"] = root;
					er["downloaded"] = info->total_size();
					er["completed"] = true;
					save_entry(Glib::build_filename(get_data_dir(), String::compose("%1", info->info_hash()) + ".resume"), er);
					Engine::get_session_manager()->open_torrent(file, root);
				}
			}
		}
		hide();
		entry_tracker->set_text("");
		entry_comment->set_text("");
		button_files->unselect_all();
		progress_hashing->set_text("");
		progress_hashing->set_fraction(0.0);
	}
}

void TorrentCreator::add_files(boost::intrusive_ptr<libtorrent::torrent_info>& info, const Glib::ustring& root, const Glib::ustring& child)
{
	Glib::ustring path = root;
	if (root != child)
		path = Glib::build_filename(root, child);
	
	if (!Glib::file_test(path, Glib::FILE_TEST_IS_DIR))
	{
		struct stat results;
		if (g_stat(path.c_str(), &results) == 0)
			info->add_file(child.c_str(), results.st_size);
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

TorrentCreator::TorrentSaveDialog::TorrentSaveDialog(Gtk::Window *parent)
	: Gtk::FileChooserDialog(*parent, _("Save torrent"), Gtk::FILE_CHOOSER_ACTION_SAVE)
{
	Gtk::Button *b = add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	b->signal_clicked().connect(sigc::mem_fun(this, &TorrentSaveDialog::hide));
	add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
}

TorrentCreator::TorrentSaveDialog::~TorrentSaveDialog()
{
}
