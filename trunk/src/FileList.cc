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
#include <numeric>

#include <gtkmm/icontheme.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/cellrenderertext.h>
#include <glibmm/i18n.h>

#include "FileList.hh"
#include "CellRendererPieceMap.hh"

#include "linkage/Engine.hh"
#include "linkage/TorrentManager.hh"
#include "linkage/Utils.hh"

#include <algorithm>
#include <iostream>

#define ICON_DIR "gnome-fs-directory"
#define ICON_FILE "gnome-fs-regular"

#define INDEX_FOLDER -1

using namespace Linkage;

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
	column->add_attribute(*piece_render, "map", columns.map);
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


	text_render = manage(new Gtk::CellRendererText());
	column = manage(new Gtk::TreeViewColumn(_("Priority"), *text_render));
	append_column(*column);
	column->set_cell_data_func(*text_render, sigc::mem_fun(this, &FileList::format_priority));
	column->set_sort_column(columns.priority);
	column->set_resizable(true);

	Gtk::Label* label = Gtk::manage(new Gtk::Label());
	label->set_markup(_("<i>Priority</i>"));
	Gtk::MenuItem* item = Gtk::manage(new Gtk::MenuItem(*label));
	m_menu.append(*item);
	m_menu.append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
	Gtk::RadioButtonGroup group;
	m_radio_max = Gtk::manage(new Gtk::RadioMenuItem(group, _("Max")));
	m_menu.append(*m_radio_max);
	m_radio_high = Gtk::manage(new Gtk::RadioMenuItem(group, _("High")));
	m_menu.append(*m_radio_high);
	m_radio_normal = Gtk::manage(new Gtk::RadioMenuItem(group, _("Normal")));
	m_menu.append(*m_radio_normal);
	m_menu.append(*Gtk::manage(new Gtk::SeparatorMenuItem()));
	m_radio_skip = Gtk::manage(new Gtk::RadioMenuItem(group, _("Skip")));
	m_menu.append(*m_radio_skip);
	m_menu.show_all_children();

	m_conn_max = m_radio_max->signal_toggled().connect(sigc::bind(sigc::mem_fun(this, &FileList::on_set_priority), P_MAX));
	m_conn_high = m_radio_high->signal_toggled().connect(sigc::bind(sigc::mem_fun(this, &FileList::on_set_priority), P_HIGH));
	m_conn_normal = m_radio_normal->signal_toggled().connect(sigc::bind(sigc::mem_fun(this, &FileList::on_set_priority), P_NORMAL));
	m_conn_skip = m_radio_skip->signal_toggled().connect(sigc::bind(sigc::mem_fun(this, &FileList::on_set_priority), P_SKIP));
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

		m_conn_max.block();
		m_conn_high.block();
		m_conn_normal.block();
		m_conn_skip.block();

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
				m_radio_normal->set_active(true);
				break;
			default:
				/* hack to set all radio inactive at once */
				Gtk::RadioButtonGroup group = m_radio_skip->get_group();
				Gtk::RadioMenuItem item(group);
				item.set_active(true);
				break;
		}

		m_conn_max.unblock();
		m_conn_high.unblock();
		m_conn_normal.unblock();
		m_conn_skip.unblock();

		m_menu.popup(event->button, event->time);
	}

	return (event->button != 1);
}

void FileList::prioritize_row(const Gtk::TreeRow& row, Priority priority)
{
	g_return_if_fail(!m_cur_torrent.expired());

	TorrentPtr torrent = m_cur_torrent.lock();

	int index = row[columns.index];
	if (index != INDEX_FOLDER)
	{
		torrent->set_file_priority(index, (int)priority);
		row[columns.priority] = priority;
	}
	else
	{
		Gtk::TreeNodeChildren children = row.children();
		std::for_each(children.begin(), children.end(), 
			sigc::bind(sigc::mem_fun(this, &FileList::prioritize_row), priority));
	}
}

void FileList::on_set_priority(Priority priority)
{
	g_return_if_fail(!m_cur_torrent.expired());

	TorrentPtr torrent = m_cur_torrent.lock();

	/* FIXME: this should be internal Torrent stuff */
	if (torrent->is_completed() && priority != P_SKIP)
		torrent->set_completed(false);

	Gtk::TreeSelection::ListHandle_Path paths = get_selection()->get_selected_rows();
	Gtk::TreeSelection::ListHandle_Path::iterator iter = paths.begin();
	while (iter != paths.end())
	{
		Gtk::TreeRow row = *(model->get_iter(*iter));
		prioritize_row(row, priority);
		iter++;
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

/*
 * This assumes that the contents of a subdir is sorted next to
 * each other in regards to their piece range.
 */
std::pair<int,int> FileList::get_piece_range(const Gtk::TreeRow& row)
{
	int p_begin = G_MAXINT, p_end = G_MININT;

	int index = row[columns.index];
	if (index == INDEX_FOLDER)
	{
		Gtk::TreeNodeChildren children = row.children();
		for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
		{
			Gtk::TreeRow child_row = *iter;
			std::pair<int,int> range = get_piece_range(child_row);
			p_begin = std::min(p_begin, range.first);
			p_end = std::max(p_end, range.second);
		}
	}
	else
	{
		TorrentPtr torrent = m_cur_torrent.lock();
		libtorrent::size_type size = row[columns.size];
		p_begin = torrent->get_info()->map_file(index, 0, 0).piece;
		p_end = torrent->get_info()->map_file(index, size, 0).piece;
	}

	
	return std::make_pair(p_begin, p_end);
}

bool FileList::on_foreach(const Gtk::TreeIter& iter, const FileData& data)
{
	Gtk::TreeRow row = *iter;

	std::pair<int,int> range = get_piece_range(row);
	int start = range.first;
	int stop = range.second;

	std::vector<bool> map;
	for (int i = start; i <= stop; i++)
		map.push_back((bool)data.pieces[i]);

	// catch parent folders
	int index = row[columns.index];
	if (index == INDEX_FOLDER)
	{
		libtorrent::size_type done;
		Glib::ustring s = row[columns.name];
		// add up all pieces in range except the last
		done = std::accumulate(map.begin(), map.end(), -map.back())*data.info->piece_length();
		// add last piece (might be smaller than piece_length)
		done += (map.back())*data.info->piece_size(stop);
		row[columns.map] = map;
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

	return false;
}

void FileList::update(const TorrentPtr& torrent)
{
	FileData data;

	data.info = torrent->get_info();
	libtorrent::torrent_status status = torrent->get_status();
	if (status.pieces)
		data.pieces = *status.pieces;
	else
		return;
	data.file_progress = torrent->get_file_progress();
	data.priorities = torrent->get_priorities();

	Gtk::TreeNodeChildren children = model->children();
	if (m_cur_torrent.lock() != torrent)
	{
		m_cur_torrent = WeakTorrentPtr(torrent);
		model->clear();
		refill_tree(data.info);
	}

	// sorting mess up iteration when we change the values in the sort column
	Gtk::SortType order;
	int col;
	model->get_sort_column_id(col, order);
	model->set_sort_column_id(Gtk::TreeSortable::DEFAULT_UNSORTED_COLUMN_ID, order);

	model->foreach_iter(sigc::bind(sigc::mem_fun(this, &FileList::on_foreach), data));

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

		boost::filesystem::path base = file.path.branch_path();
		boost::filesystem::path path = base.root_path();
		boost::filesystem::path::iterator iter;
		for (iter = base.begin(); iter != base.end(); iter++)
		{
			path /= *iter;

			Glib::ustring name = path.string();
			if (!tree[name])
			{
				if (!parent)
					tree[name] = model->append();
				else
					tree[name] = model->append(parent->children());
			}
			row = *tree[name];
			row[columns.name] = *iter;
			row[columns.index] = INDEX_FOLDER;
			row[columns.priority] = -1; /* non valid, to catch in default case */
			row[columns.size] = row[columns.size] + file.size;

			parent = tree[name];
		}
		row = *(model->append(parent->children()));
		row[columns.name] = file.path.leaf();
		row[columns.index] = i;
		row[columns.size] = file.size;
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
