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

#include <vector>

#include <gtkmm/menuitem.h>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/cellrendererprogress.h>
#include <gtkmm/cellrenderertoggle.h>

#include "FileList.hh"
#include "CellRendererPieceMap.hh"
#include "linkage/Utils.hh"
#include "linkage/Engine.hh"

FileList::FileList(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::TreeView(cobject)
{
	model = Gtk::ListStore::create(columns);

	set_model(model);

	Gtk::CellRendererToggle* trender = new Gtk::CellRendererToggle();
	trender->signal_toggled().connect(sigc::mem_fun(*this, &FileList::on_filter_toggled));
	unsigned int cols_count = append_column("Filter", *Gtk::manage(trender));
	get_column(0)->add_attribute(*trender, "active", cols_count - 1);
	append_column("Name", columns.name);

	CellRendererPieceMap *renderer = new CellRendererPieceMap();
	Gtk::TreeViewColumn *column = new Gtk::TreeViewColumn("Progress", *Gtk::manage(renderer));
	append_column(*Gtk::manage(column));
	column->add_attribute(renderer->property_map(), columns.map);
	model->set_sort_func(columns.map, sigc::mem_fun(this, &FileList::compare_piece_map));
	
	cols_count = append_column("Done", columns.done);
	column = get_column(cols_count - 1);
	Gtk::CellRendererText* cell = dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell_renderer());
	column->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(this, &FileList::format_data), columns.done));
	cols_count = append_column("Size", columns.size);
	column = get_column(cols_count - 1);
	cell = dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell_renderer());
	column->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(this, &FileList::format_data), columns.size));

	for(unsigned int i = 0; i < 5; i++)
	{
		Gtk::TreeView::Column* column = get_column(i);
		column->set_sort_column_id(i);
		column->set_resizable(true);
	}

	/*menu = Gtk::manage(new Gtk::Menu());
	Gtk::Menu* submenu_priority = Gtk::manage(new Gtk::Menu());
	Gtk::MenuItem* item = Gtk::manage(new Gtk::MenuItem("Maximum"));
	item->signal_activate().connect(sigc::bind(sigc::mem_fun(this, &FileList::on_set_priority), P_MAX));
	submenu_priority->append(*item);
	item = Gtk::manage(new Gtk::MenuItem("High"));
	item->signal_activate().connect(sigc::bind(sigc::mem_fun(this, &FileList::on_set_priority), P_HIGH));
	submenu_priority->append(*item);
	item = Gtk::manage(new Gtk::MenuItem("Normal"));
	item->signal_activate().connect(sigc::bind(sigc::mem_fun(this, &FileList::on_set_priority), P_NORMAL));
	submenu_priority->append(*item);
	item = Gtk::manage(new Gtk::MenuItem("Priority"));
	item->set_submenu(*submenu_priority);
	menu->append(*item);
	checkitem = Gtk::manage(new Gtk::CheckMenuItem("Filter"));
	check->signal_toggled().connect(this, &FileList::on_menu_filter_toggled);
	check->set_inconsistent(true);
	menu->append(*checkitem);
	menu->show_all_children();*/
}

FileList::~FileList()
{
}

void FileList::clear()
{
	model->clear();
}

/*bool FileList::on_button_press_event(GdkEventButton *event)
{
	if (event->button == 3)
	{
		WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(current_hash);
		menu->set_sensitive(!torrent->is_stopped());

		Gtk::TreeSelection::ListHandle_Path paths = get_selection().get_selected_rows();
		bool ret = false;
		if (paths.empty())
		{
			ret = TreeView::on_button_press_event(event);
			paths = get_selection().get_selected_rows();
		}

		bool all_false = true;
		bool all_true = true;
		Gtk::TreeSelection::ListHandle_Path::iterator iter = paths.begin();
		while (iter != paths.end())
		{
			Gtk::TreeRow row = *(model->get_iter(*iter));
			all_false = (!row[columns.filter] && all_false);
			all_true = (row[columns.filter] && all_true);
			iter++;
		}
		if (!all_true && !all_false)
			checkitem->set_inconsistent(true);
		else if (all_true)
			checkitem->set_active(true);
		else if (all_false)
			checkitem->set_active(false);

		menu->popup(event->button, event->time);

		return ret;
	}
	else
		return TreeView::on_button_press_event(event);
}

void FileList::on_set_priority(Priority p)
{
	WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(current_hash);

	if (torrent)
	{
		std::vector<int> priorities =	torrent->get_priorities();
		Gtk::TreeSelection::ListHandle_Path paths = get_selection().get_selected_rows();
		Gtk::TreeSelection::ListHandle_Path::iterator iter = paths.begin();
		while (iter != paths.end())
		{
			Gtk::TreeRow row = *(model->get_iter(*iter));
			priorities[row[columns.index]] = (int)p;
			iter++;
		}
		torrent->set_priorities(priorities);
	}
}

void FileList::on_menu_filter_toggled()
{
	WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(current_hash);

	if (torrent)
	{
		bool filter = checkitem->get_active();
		std::vector<int> priorities =	torrent->get_priorities();
		Gtk::TreeSelection::ListHandle_Path paths = get_selection().get_selected_rows();
		Gtk::TreeSelection::ListHandle_Path::iterator iter = paths.begin();
		while (iter != paths.end())
		{
			Gtk::TreeRow row = *(model->get_iter(*iter));
			row[columns.filter] = filter;
			priorities[row[columns.index]] = filter ? 0 : 1;
			iter++;
		}
		torrent->set_priorities(priorities);
	}
}*/

void FileList::on_filter_toggled(const Glib::ustring& path)
{
	Gtk::TreeRow row = *(model->get_iter(path));
	WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(current_hash);

	if (torrent)
	{
		row[columns.filter] = !row[columns.filter];
		torrent->filter_file(row[columns.index], row[columns.filter]);
	}
}

void FileList::format_data(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter, const Gtk::TreeModelColumn<size_type>& column)
{
	Gtk::TreeRow row = *iter;
	Gtk::CellRendererText* cell_text = dynamic_cast<Gtk::CellRendererText*>(cell);
	cell_text->property_text() = suffix_value(row[column]);
}

void FileList::update(const WeakPtr<Torrent>& torrent)
{
	current_hash = torrent->get_hash();

	torrent_handle handle = torrent->get_handle();
	torrent_info info = torrent->get_info();
	torrent_status status = torrent->get_status();
	std::vector<bool> pieces(info.num_pieces(), false);
	if (status.pieces)
		pieces = *status.pieces;
	std::vector<float> fp = torrent->get_file_progress();
	std::vector<bool> filter = torrent->get_filter();

	Gtk::CellRendererToggle* cell = dynamic_cast<Gtk::CellRendererToggle*>(get_column(0)->get_first_cell_renderer());
	Torrent::State state = torrent->get_state();
	cell->property_activatable() = (state != Torrent::SEEDING && state != Torrent::STOPPED);

	/* Sorting messes up iteration */
	int id;
	Gtk::SortType order;
	model->get_sort_column_id(id, order);
	model->set_sort_column(columns.index, Gtk::SORT_ASCENDING);

	Gtk::TreeNodeChildren children = model->children();
	if (children.size() != info.num_files())
	{
		model->clear();
		for (int i = 0; i < info.num_files(); i++)
		{
			Gtk::TreeRow row = *(model->append());
			row[columns.index] = i;
		}
	}

	for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
	{
		Gtk::TreeRow row = *iter;
		
		std::vector<bool> map;

		file_entry file = info.file_at(row[columns.index]);
		peer_request file_info = info.map_file(row[columns.index], 0, file.size);

		unsigned int byte_pos_in_file = 0;
		unsigned int piece_index = file_info.piece;
		while (byte_pos_in_file < file.size)
		{
			if (pieces[piece_index])
				map.push_back(true);
			else
				map.push_back(false);

			byte_pos_in_file += info.piece_size(piece_index);
			piece_index++;
		}

		row[columns.filter] = filter[row[columns.index]];
		row[columns.name] = file.path.string();
		row[columns.map] = map;
		row[columns.done] = (size_type)(fp[row[columns.index]] * file.size);
		row[columns.size] = file.size;
	}
	model->set_sort_column(id, order);
}

int FileList::compare_piece_map(const Gtk::TreeIter& a,
																const Gtk::TreeIter& b)
{
	Gtk::TreeRow row_a, row_b;
	row_a = *a;
	row_b = *b;
	
	double complete_a = 0, complete_b = 0;

	if (row_a[columns.size] != 0)
		complete_a = (double)row_a[columns.done]/row_a[columns.size];
	if (row_b[columns.size] != 0)
		complete_b = (double)row_b[columns.done]/row_b[columns.size];
	
	if (complete_a < complete_b)
		return -1;
	else if (complete_a > complete_b)
		return 1;
	else
		return 0;
}
