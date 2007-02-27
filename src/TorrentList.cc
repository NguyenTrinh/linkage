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

#include <gtkmm/treepath.h>
#include <gtkmm/treeselection.h>
#include <gtkmm/cellrenderertext.h>

#include "CellRendererProgressText.hh"
#include "TorrentList.hh"
#include "linkage/Engine.hh"
#include "linkage/Utils.hh"

typedef Gtk::TreeSelection::ListHandle_Path PathList;

TorrentList::TorrentList()
{
	model = Gtk::ListStore::create(columns);
	filter = Gtk::TreeModelFilter::create(model);

	set_model(filter);

	get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);

	append_column("#", columns.position);
	Gtk::TreeViewColumn* column = get_column(COL_POSITION);
	column->set_cell_data_func(*column->get_first_cell_renderer(), sigc::mem_fun(this, &TorrentList::format_position));
	append_column("Name", columns.name);
	CellRendererProgressText* renderer = manage(new CellRendererProgressText());
	append_column("Progress", *renderer);
	column = get_column(COL_PROGRESS);
	column->add_attribute(*renderer, "value", COL_PROGRESS);
	column->add_attribute(*renderer, "text", COL_ETA);
	column->add_attribute(*renderer, "text-left", COL_DOWNRATE);
	column->add_attribute(*renderer, "text-right", COL_UPRATE);
	column->set_cell_data_func(*renderer, sigc::mem_fun(this, &TorrentList::format_rates));

	for(unsigned int i = 0; i < 3; i++)
	{
		Gtk::TreeView::Column* column = get_column(i);
		column->set_sort_column_id(i);
		if (i == COL_NAME)
		{
			Gtk::CellRenderer* name_render = column->get_first_cell_renderer();
			column->clear_attributes(*name_render);
			column->add_attribute(*name_render, "markup", COL_NAME);
			column->set_expand(true);
		}
		column->set_resizable(true);
	}
	set_headers_visible(false);

	Glib::RefPtr<SettingsManager> sm = Engine::instance()->get_settings_manager();

	/* FIXME: Add option to trunkate names */
	Gtk::SortType sort_order = Gtk::SortType(sm->get_int("UI", "SortOrder"));
	model->set_sort_column_id(sm->get_int("UI", "SortColumn"), sort_order);
	
	Glib::RefPtr<TorrentManager> tm = Engine::instance()->get_torrent_manager();
	tm->signal_added().connect(sigc::mem_fun(*this, &TorrentList::on_added));
	tm->signal_removed().connect(sigc::mem_fun(*this, &TorrentList::on_removed));
}

TorrentList::~TorrentList()
{
	Glib::RefPtr<SettingsManager> sm = Engine::instance()->get_settings_manager();

	int column;
	Gtk::SortType order;
	model->get_sort_column_id(column, order);

	sm->set("UI", "SortOrder", int(order));
	sm->set("UI", "SortColumn", column);

	PathList path_list = get_selection()->get_selected_rows();
	if (path_list.size() == 1)
	{
		Gtk::TreeIter iter = model->get_iter(*path_list.begin());
		if (iter)
		{
			Gtk::TreeRow row = *iter;
			sm->set("UI", "Selected", str(row[columns.hash]));
		}
	}
}

Glib::SignalProxy0<void> TorrentList::signal_changed()
{
	return get_selection()->signal_changed();
}

bool TorrentList::on_filter(const Gtk::TreeModel::const_iterator& iter, const GroupFilter& group)
{
	Gtk::TreeRow row = *iter;
	sha1_hash hash = row[columns.hash];
	WeakPtr<Torrent> torrent = Engine::instance()->get_torrent_manager()->get_torrent(hash);
	return (m_do_filter) ? group.eval(torrent) : true;
}

void TorrentList::on_filter_set(const GroupFilter& group)
{
	m_do_filter = true;
	filter->set_visible_func(sigc::bind(sigc::mem_fun(this, &TorrentList::on_filter), group));
}

void TorrentList::on_filter_unset()
{
	m_do_filter = false;
}

void TorrentList::set_filter_set_signal(sigc::signal<void, const GroupFilter&> signal)
{
	signal.connect(sigc::mem_fun(this, &TorrentList::on_filter_set));
}

void TorrentList::set_filter_unset_signal(sigc::signal<void> signal)
{
	signal.connect(sigc::mem_fun(this, &TorrentList::on_filter_unset));
}

void TorrentList::on_added(const sha1_hash& hash, const Glib::ustring& name, unsigned int position)
{
	Glib::ustring selected_hash = Engine::instance()->get_settings_manager()->get_string("UI", "Selected");
	WeakPtr<Torrent> torrent = Engine::instance()->get_torrent_manager()->get_torrent(hash);
	
	Gtk::TreeIter iter = model->append();
	Gtk::TreeRow new_row = *iter;
	new_row[columns.hash] = hash;
	if (selected_hash == str(hash))
		get_selection()->select(iter);

	update(torrent);
}

void TorrentList::on_removed(const sha1_hash& hash)
{
	Gtk::TreeIter iter = get_iter(hash);
	model->erase(iter);
}

Gtk::TreeIter TorrentList::get_iter(const sha1_hash& hash)
{
	Gtk::TreeNodeChildren children = model->children();
	for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
	{
		Gtk::TreeRow row = *iter;
		if (hash == row[columns.hash])
			return iter;
	}
}

bool TorrentList::is_selected(const sha1_hash& hash)
{
	PathList paths = get_selection()->get_selected_rows();
	for (PathList::iterator piter = paths.begin(); piter != paths.end(); ++piter)
	{
		Gtk::TreeIter iter = model->get_iter(*piter);
		if (iter)
		{
			Gtk::TreeRow row = *iter;
			if (hash == row[columns.hash])
				return true;
		}
	}
	return false;
}

HashList TorrentList::get_selected_list()
{
	PathList paths = get_selection()->get_selected_rows();

	//Sort the selected list by columns.position to ease moving
	std::list<Gtk::TreeRow> ordered_rows;
	for (PathList::iterator piter = paths.begin(); piter != paths.end(); ++piter)
	{
		Gtk::TreeIter iter = model->get_iter(*piter);
		if (iter)
		{
			Gtk::TreeRow row = *iter;
			std::list<Gtk::TreeRow>::iterator riter;
			for (riter = ordered_rows.begin(); riter != ordered_rows.end(); ++riter)
			{
				if (row[columns.position] < (*riter)[columns.position])
				{
					ordered_rows.insert(riter, row);
					break;
				}
			}
			if (riter == ordered_rows.end())
				ordered_rows.push_back(row);
		}
	}
	HashList list;
	for (std::list<Gtk::TreeRow>::iterator iter = ordered_rows.begin();
			 iter != ordered_rows.end(); ++iter)
	{
		list.push_back((*iter)[columns.hash]);
	}

	return list;
}

void TorrentList::set_sort_column(Column col_id)
{
	int current_col_id = 0;
	Gtk::SortType current_order;
	model->get_sort_column_id(current_col_id, current_order);
	model->set_sort_column_id(col_id, current_order);
}

void TorrentList::set_sort_order(Gtk::SortType order)
{
	int current_col_id = 0;
	Gtk::SortType current_order;
	model->get_sort_column_id(current_col_id, current_order);
	model->set_sort_column_id(current_col_id, order);
}

void TorrentList::format_rates(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter)
{
	Gtk::TreeRow row = *iter;
	CellRendererProgressText* cell_pt = dynamic_cast<CellRendererProgressText*>(cell);

	cell_pt->property_text_left() = "DL: " + suffix_value(row[columns.down_rate]) + "/s";
	cell_pt->property_text_right() = "UL: " + suffix_value(row[columns.up_rate]) + "/s";
}

void TorrentList::format_position(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter)
{
	Gtk::TreeRow row = *iter;
	Gtk::CellRendererText* cell_t = dynamic_cast<Gtk::CellRendererText*>(cell);

	cell_t->property_text() = str(row[columns.position]);
}

bool TorrentList::on_button_press_event(GdkEventButton *event)
{
	Gtk::TreeView::on_button_press_event(event);

	Gtk::TreePath path;
	Gtk::TreeViewColumn* column;
	int cell_x, cell_y;

	if (!get_path_at_pos((int)event->x, (int)event->y, path, column, cell_x, cell_y))
		return false;
	
	Gtk::TreeRow row = *model->get_iter(path);

	if (event->button == 1 && event->type == GDK_2BUTTON_PRESS)
		m_signal_double_click.emit(row[columns.hash]);
	else if (event->button == 3)
		m_signal_right_click.emit(row[columns.hash]);

	return true;
}

sigc::signal<void, const sha1_hash&> TorrentList::signal_double_click()
{
	return m_signal_double_click;
}

sigc::signal<void, const sha1_hash&> TorrentList::signal_right_click()
{
	return m_signal_right_click;
}

void TorrentList::update(const WeakPtr<Torrent>& torrent)
{
	Gtk::TreeRow row = *get_iter(torrent->get_hash());
	row[columns.position] = torrent->get_position();

	Glib::ustring color;
	switch (torrent->get_state())
	{
		case Torrent::STOPPED:
			color = "#999999";
			break;
		case Torrent::CHECK_QUEUE:
		case Torrent::QUEUED:
			color = "#5C5C5C";
			break;
		case Torrent::SEEDING:
			color = "#4F96FF";
			break;
		case Torrent::CHECKING:
			color = "#FF7F50";
			break;
		case Torrent::DOWNLOADING:
		default:
			color = "#000000";
			break;
	}

	std::stringstream ss;

	row[columns.state] = torrent->get_state_string();

	unsigned int up = torrent->get_total_uploaded();
	unsigned int down = torrent->get_total_downloaded();
	
	row[columns.down] = down;
	row[columns.up] = up;

	Glib::ustring name = Glib::Markup::escape_text(torrent->get_name());

	if (!torrent->is_running())
	{
		ss << "<span foreground='" << color << "'><b>" << name << "</b></span>\nStopped";
		row[columns.name] = ss.str();
		row[columns.down_rate] = 0;
		row[columns.up_rate] = 0;
		row[columns.seeds] = 0;
		row[columns.peers] = 0;
		row[columns.eta] = str(double(row[columns.progress]), 2) + " %";
		return;
	}

	torrent_status status = torrent->get_status();

	if (torrent->get_state() == Torrent::SEEDING)
	{
		int size = status.total_wanted_done - up;
		if (down != 0)
		{
			double ratio = (double)up/down;

			row[columns.eta] = str(ratio, 3) + " " + get_eta(size, status.upload_payload_rate);
			if (ratio < 1)
				row[columns.progress] = ratio*100;
		 	else
		 		row[columns.progress] = 100;
		}
		else
		{
			if (up != 0)
			{
				row[columns.eta] = "\u221E " + get_eta(size, status.upload_payload_rate);
				row[columns.progress] = 100;
			}
			else
			{
				row[columns.eta] = "0.000 " + get_eta(size, status.upload_payload_rate);
				row[columns.progress] = 0;
			}
		}
	}
	else
	{
		row[columns.progress] = double(status.progress*100);
		row[columns.eta] = str(double(status.progress*100), 2) + " % " + get_eta(status.total_wanted-status.total_wanted_done, status.download_payload_rate);
	}

	ss << "<span foreground='" << color << "'><b>" << name <<
		"</b> (" << suffix_value((int)torrent->get_info().total_size()) << ")" <<
		"</span>\n";
	Torrent::State state = torrent->get_state();
	if (state != Torrent::QUEUED && state != Torrent::SEEDING)
		ss << status.num_seeds << " connected seeds, " <<
					status.num_peers - status.num_seeds << " peers";
	else if (state == Torrent::QUEUED || state == Torrent::CHECK_QUEUE)
		ss << "Queued";
	if (state == Torrent::SEEDING)
		ss << status.num_peers - status.num_seeds << " connected peers";

	row[columns.name] = ss.str();
	row[columns.down_rate] = (unsigned int)status.download_payload_rate;
	row[columns.up_rate] = (unsigned int)status.upload_payload_rate;
	row[columns.seeds] = status.num_seeds;
	row[columns.peers] = status.num_peers - status.num_seeds;
}
