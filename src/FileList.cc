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

#include <gtkmm/icontheme.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/cellrenderertext.h>
#include <gtkmm/cellrenderertoggle.h>
#include <glibmm/i18n.h>

#include "FileList.hh"
#include "CellRendererPieceMap.hh"

#include "linkage/Engine.hh"
#include "linkage/TorrentManager.hh"
#include "linkage/Utils.hh"

#define ICON_DIR "gnome-fs-directory"
#define ICON_FILE "gnome-fs-regular"

#define INDEX_FOLDER -1

FileList::FileList(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::TreeView(cobject)
{
	model = Gtk::TreeStore::create(columns);

	set_model(model);

	Gtk::CellRendererToggle* toggle_render = manage(new Gtk::CellRendererToggle());
	toggle_render->signal_toggled().connect(sigc::mem_fun(*this, &FileList::on_filter_toggled));
	int cols_count = append_column(_("Filter"), *toggle_render);
	Gtk::TreeViewColumn* column = get_column(cols_count - 1);
	column->add_attribute(*toggle_render, "active", columns.filter);
	column->add_attribute(*toggle_render, "inconsistent", columns.inconsistent);
	column->set_sort_column(columns.filter);
	column->set_resizable(true);

	column = manage(new Gtk::TreeViewColumn(_("Name")));
	append_column(*column);
	Gtk::CellRendererText* text_render = manage(new Gtk::CellRendererText());
	Gtk::CellRendererPixbuf* icon_render = manage(new Gtk::CellRendererPixbuf());
	column->pack_start(*icon_render, false);
	column->pack_start(*text_render, true);
	column->add_attribute(*icon_render, "pixbuf", columns.icon);
	column->add_attribute(*text_render, "text", columns.name);
	column->set_sort_column(columns.name);
	column->set_resizable(true);

	CellRendererPieceMap *piece_render = manage(new CellRendererPieceMap());
	column = manage(new Gtk::TreeViewColumn(_("Pieces"), *piece_render));
	append_column(*column);
	column->add_attribute(piece_render->property_map(), columns.map);
	model->set_sort_func(columns.map, sigc::mem_fun(this, &FileList::compare_piece_map));
	column->set_sort_column(columns.map);
	column->set_resizable(true);

	cols_count = append_column(_("Done"), columns.done);
	column = get_column(cols_count - 1);
	Gtk::CellRendererText* cell = dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell_renderer());
	column->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(this, &FileList::format_data), columns.done));
	column->set_sort_column(columns.done);
	column->set_resizable(true);

	cols_count = append_column(_("Size"), columns.size);
	column = get_column(cols_count - 1);
	cell = dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell_renderer());
	column->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(this, &FileList::format_data), columns.size));
	column->set_sort_column(columns.size);
	column->set_resizable(true);

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
	WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(current_hash);
	if (!torrent)
		return;

	Gtk::TreeRow row = *(model->get_iter(path));

	row[columns.filter] = !row[columns.filter];
	int index = row[columns.index];
	if (index == INDEX_FOLDER)
	{
		filter_children(row.children(), row[columns.filter]);
		row[columns.inconsistent] = false;
	}
	else
		torrent->filter_file(index, row[columns.filter]);

	Gtk::TreeIter parent = row.parent();
	while (parent)
	{
		Gtk::TreeRow parent_row = *parent;
		bool filter = parent_row[columns.filter];
		Glib::ustring n = parent_row[columns.name];
		Gtk::TreeNodeChildren children = parent_row.children();
		int n_consistent = 0;
		for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
		{
			Gtk::TreeRow child_row = *iter;
			if (child_row[columns.filter] == filter)
				n_consistent++;
		}
		parent_row[columns.inconsistent] = (n_consistent && (n_consistent != children.size()));
		parent = parent_row.parent();
	}
}

void FileList::filter_children(const Gtk::TreeNodeChildren& children, bool filter)
{
	WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(current_hash);
	if (!torrent)
		return;

	for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
	{
		Gtk::TreeRow row = *iter;
		row[columns.filter] = filter;
		int index = row[columns.index];
		if (index == INDEX_FOLDER)
			filter_children(row.children(), filter);
		else
			torrent->filter_file(index, row[columns.filter]);
	}
}

void FileList::format_data(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter, const Gtk::TreeModelColumn<libtorrent::size_type>& column)
{
	Gtk::TreeRow row = *iter;
	Gtk::CellRendererText* cell_text = dynamic_cast<Gtk::CellRendererText*>(cell);
	cell_text->property_text() = suffix_value(row[column]);
}

void FileList::on_reverse_foreach(const Gtk::TreeIter& iter, const FileData& data)
{
	if (!iter)
		return;

	Gtk::TreeRow row = *iter;

	// catch parent folders
	int index = row[columns.index];
	if (index == INDEX_FOLDER)
	{
		std::vector<bool> map = row[columns.map];
		if (map.empty())
			map.assign(row.children().size(), false);

		int map_index = 0;
		libtorrent::size_type size = 0, done = 0;
		Gtk::TreeNodeChildren children = row.children();
		for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
		{
			Gtk::TreeRow child_row = *iter;

			map[map_index] = (child_row[columns.size] == child_row[columns.done]);
			map_index++;

			size += child_row[columns.size];
			done += child_row[columns.done];
		}
		row[columns.map] = map;
		row[columns.size] = size;
		row[columns.done] = done;
		Glib::RefPtr<Gdk::Pixbuf> icon = row[columns.icon];
		if (!icon)
		{
			Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::get_default();
			row[columns.icon] = theme->load_icon(ICON_DIR, Gtk::ICON_SIZE_MENU, Gtk::ICON_LOOKUP_USE_BUILTIN);
		}
	}
	else
	{
		libtorrent::file_entry file = data.info.file_at(index);
		libtorrent::peer_request file_info = data.info.map_file(index, 0, file.size);

		std::vector<bool> map;
		unsigned int byte_pos_in_file = 0;
		unsigned int piece_index = file_info.piece;
		while (byte_pos_in_file < file.size)
		{
			if (data.pieces[piece_index])
				map.push_back(true);
			else
				map.push_back(false);

			byte_pos_in_file += data.info.piece_size(piece_index);
			piece_index++;
		}

		row[columns.filter] = data.filter[index];
		row[columns.map] = map;
		row[columns.done] = (libtorrent::size_type)(data.file_progress[index] * file.size);
		Glib::RefPtr<Gdk::Pixbuf> icon = row[columns.icon];
		if (!icon)
		{
			Glib::RefPtr<Gtk::IconTheme> theme = Gtk::IconTheme::get_default();
			row[columns.icon] = theme->load_icon(ICON_FILE, Gtk::ICON_SIZE_MENU, Gtk::ICON_LOOKUP_USE_BUILTIN);
		}
	}
}

bool FileList::on_foreach(const Gtk::TreeModel::iterator& iter, IterList* list)
{
	Gtk::TreeIter iter_copy(iter);
	list->push_back(iter_copy);
	return false;
}

void FileList::update(const WeakPtr<Torrent>& torrent)
{
	FileData data;

	data.info = torrent->get_info();
	libtorrent::torrent_status status = torrent->get_status();
	if (status.pieces)
		data.pieces = *status.pieces;
	else
		data.pieces.assign(data.info.num_pieces(), false);
	data.file_progress = torrent->get_file_progress();
	data.filter = torrent->get_filter();

	Gtk::TreeNodeChildren children = model->children();
	if (current_hash != torrent->get_hash())
	{
		model->clear();
		refill_tree(data.info);
	}
	current_hash = torrent->get_hash();
	Gtk::CellRendererToggle* cell = dynamic_cast<Gtk::CellRendererToggle*>
		(get_column(0)->get_first_cell_renderer());
	Torrent::State state = torrent->get_state();
	cell->property_activatable() = (state != Torrent::SEEDING && state != Torrent::STOPPED);

	// sorting mess up iteration when we change the values in the sort column
	Gtk::SortType order;
	int col;
	model->get_sort_column_id(col, order);
	model->set_sort_column_id(Gtk::TreeSortable::DEFAULT_UNSORTED_COLUMN_ID, order);

	// FIXME: this is pretty inefficient
	IterList list;
	model->foreach_iter(sigc::bind(sigc::mem_fun(this, &FileList::on_foreach), &list));
	for (IterList::reverse_iterator iter = list.rbegin(); iter != list.rend(); ++iter)
		on_reverse_foreach(*iter, data);

	model->set_sort_column_id(col, order);
}

void FileList::refill_tree(const libtorrent::torrent_info& info)
{
	std::map<Glib::ustring, Gtk::TreeIter> tree;

	for (int i = 0; i < info.num_files(); i++)
	{
		libtorrent::file_entry file = info.file_at(i);

		Gtk::TreeIter parent = tree[*file.path.begin()];
		Gtk::TreeRow row;

		boost::filesystem::path::iterator iter;
		for (iter = file.path.begin(); iter != file.path.end(); ++iter)
		{
			Glib::ustring name = *iter;
			if (!tree[name])
			{
				if (!parent)
					tree[name] = *(model->append());
				else
					tree[name] = *(model->append((*parent).children()));
			}
			row = *tree[name];
			row[columns.name] = name;
			row[columns.index] = INDEX_FOLDER;

			parent = tree[name];
		}
		row[columns.index] = i;
		row[columns.size] = file.size;
	}

	Gtk::TreeNodeChildren children = model->children();
	for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
	{
		Gtk::TreeRow row = *iter;
		if (row[columns.index] == INDEX_FOLDER)
		{
			std::vector<bool> map(row.children().size(), false);
			row[columns.map] = map;
		}
	}

	expand_all();
}

int FileList::compare_piece_map(const Gtk::TreeIter& a, const Gtk::TreeIter& b)
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
