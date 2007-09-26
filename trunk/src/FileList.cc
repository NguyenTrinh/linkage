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
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/cellrenderertext.h>
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

	get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);

	Gtk::TreeViewColumn* column = manage(new Gtk::TreeViewColumn(_("Name")));
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

	int cols_count = append_column(_("Done"), columns.done);
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

	cols_count = append_column(_("Priority"), columns.priority);
	column = get_column(cols_count - 1);
	cell = dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell_renderer());
	//FIXME: this works but spouts g_warnings, should'nt this override the default func?
	column->set_cell_data_func(*cell, sigc::mem_fun(this, &FileList::format_priority));
	column->set_sort_column(columns.priority);
	column->set_resizable(true);


	Gtk::Label* label = Gtk::manage(new Gtk::Label());
	label->set_markup(_("<i>Priority</i>"));
	Gtk::MenuItem* item = Gtk::manage(new Gtk::MenuItem(*label));
	m_menu.append(*item);
	m_menu.append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
	Gtk::RadioButtonGroup group;
	m_radio_max = Gtk::manage(new Gtk::RadioMenuItem(group, _("Max")));
	m_radio_max->signal_toggled().connect(sigc::bind(sigc::mem_fun(this, &FileList::on_set_priority), P_MAX));
	m_menu.append(*m_radio_max);
	m_radio_high = Gtk::manage(new Gtk::RadioMenuItem(group, _("High")));
	m_radio_high->signal_toggled().connect(sigc::bind(sigc::mem_fun(this, &FileList::on_set_priority), P_HIGH));
	m_menu.append(*m_radio_high);
	m_radio_normal = Gtk::manage(new Gtk::RadioMenuItem(group, _("Normal")));
	m_radio_normal->signal_toggled().connect(sigc::bind(sigc::mem_fun(this, &FileList::on_set_priority), P_NORMAL));
	m_menu.append(*m_radio_normal);
	m_menu.append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
	m_radio_skip = Gtk::manage(new Gtk::RadioMenuItem(group, _("Skip")));
	m_radio_skip->signal_toggled().connect(sigc::bind(sigc::mem_fun(this, &FileList::on_set_priority), P_SKIP));
	m_menu.append(*m_radio_skip);
	m_menu.show_all_children();
}

FileList::~FileList()
{
}

void FileList::clear()
{
	model->clear();
}

bool FileList::on_button_press_event(GdkEventButton *event)
{
	Gtk::TreePath path;
	Gtk::TreeViewColumn* column;
	int cell_x, cell_y;
	if (!get_path_at_pos((int)event->x, (int)event->y, path, column, cell_x, cell_y))
		return false;

	bool selected = get_selection()->is_selected(path);
	int selected_rows = get_selection()->count_selected_rows();
	if (event->button == 1 || selected_rows <= 1 ||	!selected)
		TreeView::on_button_press_event(event);

	if (event->button == 3)
	{
		Gtk::TreeRow row = *(model->get_iter(path));
		switch (row[columns.priority])
		{
			case P_SKIP:
				m_radio_skip->set_active(true);
				break;
			case P_HIGH:
				m_radio_high->set_active(true);
				break;
			case P_MAX:
				m_radio_max->set_active(true);
				break;
			case P_NORMAL:
			default:
				m_radio_normal->set_active(true);
				break;
		}
		m_menu.popup(event->button, event->time);
	}

	return (event->button != 1);
}

void FileList::on_set_priority(Priority priority)
{
	if (m_cur_torrent && !m_menu.is_visible())
	{
		std::vector<int> priorities =	m_cur_torrent->get_priorities();
		Gtk::TreeSelection::ListHandle_Path paths = get_selection()->get_selected_rows();
		Gtk::TreeSelection::ListHandle_Path::iterator iter = paths.begin();
		while (iter != paths.end())
		{
			Gtk::TreeRow row = *(model->get_iter(*iter));
			int index = row[columns.index];
			g_return_if_fail(index != INDEX_FOLDER);
			priorities[index] = (int)priority;
			row[columns.priority] = priority;
			iter++;
		}
		m_cur_torrent->set_priorities(priorities);
	}
}

void FileList::format_data(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter,
	const Gtk::TreeModelColumn<libtorrent::size_type>& column)
{
	Gtk::TreeRow row = *iter;
	Gtk::CellRendererText* cell_text = dynamic_cast<Gtk::CellRendererText*>(cell);
	cell_text->property_text() = suffix_value(row[column]);
}

void FileList::format_priority(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter)
{
	Gtk::TreeRow row = *iter;
	Gtk::CellRendererText* cell_text = dynamic_cast<Gtk::CellRendererText*>(cell);
	Glib::ustring priority;
	if (row[columns.index] != INDEX_FOLDER)
	{
		switch (row[columns.priority])
		{
			case P_MAX:
				priority = _("Max");
				break;
			case P_HIGH:
				priority = _("High");
				break;
			case P_SKIP:
				priority = _("Skip");
				break;
			default:
			case P_NORMAL:
				priority = _("Normal");
				break;
		}
	}
	cell_text->property_text() = priority;
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
		libtorrent::file_entry file = data.info->file_at(index);
		libtorrent::peer_request file_info = data.info->map_file(index, 0, file.size);

		std::vector<bool> map;
		unsigned int byte_pos_in_file = 0;
		unsigned int piece_index = file_info.piece;
		while (byte_pos_in_file < file.size)
		{
			if (data.pieces[piece_index])
				map.push_back(true);
			else
				map.push_back(false);

			byte_pos_in_file += data.info->piece_size(piece_index);
			piece_index++;
		}

		row[columns.priority] = (Priority)data.priorities[index];
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

void FileList::update(const Glib::RefPtr<Torrent>& torrent)
{
	FileData data;

	data.info = torrent->get_info();
	libtorrent::torrent_status status = torrent->get_status();
	if (status.pieces)
		data.pieces = *status.pieces;
	else
		data.pieces.assign(data.info->num_pieces(), false);
	data.file_progress = torrent->get_file_progress();
	data.priorities = torrent->get_priorities();

	Gtk::TreeNodeChildren children = model->children();
	if (m_cur_torrent != torrent)
	{
		m_cur_torrent = torrent;
		model->clear();
		refill_tree(data.info);
	}
	
	Torrent::State state = torrent->get_state();

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

void FileList::refill_tree(const boost::intrusive_ptr<libtorrent::torrent_info>& info)
{
	std::map<Glib::ustring, Gtk::TreeIter> tree;

	for (int i = 0; i < info->num_files(); i++)
	{
		libtorrent::file_entry file = info->file_at(i);

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
