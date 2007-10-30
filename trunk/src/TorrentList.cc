/*
Copyright (C) 2006-2007   Christian Lundgren
Copyright (C) 2007        Dave Moore

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
#include <glibmm/i18n.h>

#include "CellRendererProgressText.hh"
#include "TorrentList.hh"

#include "linkage/Engine.hh"
#include "linkage/TorrentManager.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/Utils.hh"
#include "linkage/compose.hpp"

typedef Gtk::TreeSelection::ListHandle_Path PathList;

static GConfEnumStringPair sort_order_table[] = {
  { Gtk::SORT_ASCENDING, "SORT_ASCENDING" },
  { Gtk::SORT_DESCENDING, "SORT_DESCENDING" }
};

static GConfEnumStringPair sort_column_table[] = {
  { TorrentList::COL_POSITION, "COLUMN_POSITION" },
  { TorrentList::COL_NAME, "COLUMN_NAME" },
  { TorrentList::COL_NAME_FORMATED, "COLUMN_NAME_FORMATED" },
  { TorrentList::COL_PROGRESS, "COLUMN_PROGRESS" },
  { TorrentList::COL_STATUS, "COLUMN_STATUS" },
  { TorrentList::COL_DOWNLOADED, "COLUMN_DOWNLOADED" },
  { TorrentList::COL_UPLOADED, "COLUMN_UPLOADED" },
  { TorrentList::COL_DOWNRATE, "COLUMN_DOWNRATE" },
  { TorrentList::COL_UPRATE, "COLUMN_UPRATE" },
  { TorrentList::COL_SEEDS, "COLUMN_SEEDS" },
  { TorrentList::COL_PEERS, "COLUMN_PEERS" },
  { TorrentList::COL_ETA, "COLUMN_ETA" },
  { TorrentList::COL_HASH, "COLUMN_HASH"}
};

TorrentList::TorrentList(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::TreeView(cobject),
	glade_xml(refGlade)
{
	m_cur_state = Torrent::NONE;

	model = Gtk::ListStore::create(columns);
	filter = Gtk::TreeModelFilter::create(model);
	filter->set_visible_func(sigc::mem_fun(this, &TorrentList::on_filter));

	set_model(filter);

	get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);

	int col_id = append_column("#", columns.position);
	col_id = append_column(_("Name"), columns.name_formated);
	Gtk::TreeViewColumn* column = get_column(col_id - 1);
	Gtk::CellRendererText* renderer_text = dynamic_cast<Gtk::CellRendererText*>
		(column->get_first_cell_renderer());
	column->clear_attributes(*renderer_text);
	column->add_attribute(*renderer_text, "markup", columns.name_formated);
	column->set_expand(true);
	CellRendererProgressText* renderer = manage(new CellRendererProgressText());
	col_id = append_column(_("Progress"), *renderer);
	column = get_column(col_id - 1);
	column->add_attribute(*renderer, "value", columns.progress);
	column->add_attribute(*renderer, "text", columns.eta);
	column->add_attribute(*renderer, "text-left", columns.down_rate);
	column->add_attribute(*renderer, "text-right", columns.up_rate);
	column->set_cell_data_func(*renderer, sigc::mem_fun(this, &TorrentList::format_rates));

	set_headers_visible(false);
	set_search_column(columns.name);

	Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();

	Gtk::SortType order = (Gtk::SortType)sm->get_enum("ui/torrent_view/order", sort_order_table);
	gint col = sm->get_enum("ui/torrent_view/column", sort_column_table);
	model->set_sort_column_id(col, order);

	Glib::RefPtr<TorrentManager> tm = Engine::get_torrent_manager();
	TorrentManager::TorrentList torrents = tm->get_torrents();
	for (TorrentManager::TorrentList::iterator iter = torrents.begin();
				iter != torrents.end(); ++iter)
	{
		on_added(*iter);
	}
	PathList path_list = get_selection()->get_selected_rows();
	if (!path_list.empty())
	{
		Gtk::TreePath path = filter->convert_path_to_child_path(*path_list.begin());
		// this doesn't work so great for some reason
		scroll_to_row(path);
	}

	tm->signal_added().connect(sigc::mem_fun(*this, &TorrentList::on_added));
	tm->signal_removed().connect(sigc::mem_fun(*this, &TorrentList::on_removed));
}

TorrentList::~TorrentList()
{
	Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();

	Gtk::SortType order;
	int column;
	model->get_sort_column_id(column, order);
	sm->set("ui/torrent_view/order", sort_order_table, order);
	sm->set("ui/torrent_view/column", sort_order_table, column);

	PathList path_list = get_selection()->get_selected_rows();
	if (path_list.size() == 1)
	{
		Gtk::TreePath path = filter->convert_path_to_child_path(*path_list.begin());
		Gtk::TreeIter iter = model->get_iter(path);
		if (iter)
		{
			Gtk::TreeRow row = *iter;
			Glib::ustring hash_str = String::compose("%1", row[columns.hash]);
			sm->set("ui/torrent_view/selected", hash_str);
		}
	}
}

Glib::SignalProxy0<void> TorrentList::signal_changed()
{
	return get_selection()->signal_changed();
}

bool TorrentList::on_filter(const Gtk::TreeModel::const_iterator& iter)
{
	Gtk::TreeRow row = *iter;
	libtorrent::sha1_hash hash = row[columns.hash];
	Glib::RefPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
	if (!torrent)
		return false;

	if (m_cur_state && !(m_cur_state & torrent->get_state()))
		return false;

	return (m_active_group) ? m_active_group.eval(torrent) : true;
}

void TorrentList::on_filter_set(const Group& group)
{
	/* FIXME: This hits an endless loop if we have a multi selection */
	m_active_group = group;
	filter->refilter();
}

void TorrentList::on_state_filter_changed(Torrent::State state)
{
	m_cur_state = state;
	filter->refilter();
}

void TorrentList::on_added(const Glib::RefPtr<Torrent>& torrent)
{
	Glib::ustring selected_hash = Engine::get_settings_manager()->get_string("ui/torrent_view/selected");

	Gtk::TreeIter iter = model->append();
	Gtk::TreeRow row = *iter;
	row[columns.hash] = torrent->get_hash();
	row[columns.name] = torrent->get_name();

	if (selected_hash == String::compose("%1", torrent->get_hash()))
		get_selection()->select(filter->convert_child_iter_to_iter(iter));

	update_row(row);
}

void TorrentList::on_removed(const Glib::RefPtr<Torrent>& torrent)
{
	Gtk::TreeNodeChildren children = model->children();
	for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
	{
		Gtk::TreeRow row = *iter;
		if (torrent->get_hash() == row[columns.hash])
		{
			model->erase(iter);
			break;
		}
	}
}

bool TorrentList::is_selected(const libtorrent::sha1_hash& hash)
{
	PathList paths = get_selection()->get_selected_rows();
	for (PathList::iterator piter = paths.begin(); piter != paths.end(); ++piter)
	{
		Gtk::TreePath path = filter->convert_path_to_child_path(*piter);
		Gtk::TreeIter iter = model->get_iter(path);
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
		Gtk::TreePath path = filter->convert_path_to_child_path(*piter);
		Gtk::TreeIter iter = model->get_iter(path);
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

Glib::ustring TorrentList::get_formated_name(const Glib::RefPtr<Torrent>& torrent)
{
	Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();
	int state = torrent->get_state();
	Glib::ustring color;
	switch (state)
	{
		case Torrent::CHECK_QUEUE:
			color = sm->get_string("ui/colors/check_queue");
			break;
		case Torrent::CHECKING:
			color = sm->get_string("ui/colors/checking");
			break;
		case Torrent::ANNOUNCING:
			color = sm->get_string("ui/colors/announcing");
			break;
		case Torrent::DOWNLOADING:
			color = sm->get_string("ui/colors/downloading");
			break;
		case Torrent::FINISHED:
			color = sm->get_string("ui/colors/finished");
			break;
		case Torrent::SEEDING:
			color = sm->get_string("ui/colors/seeding");
			break;
		case Torrent::ALLOCATING:
			color = sm->get_string("ui/colors/allocating");
			break;
		case Torrent::STOPPED:
			color = sm->get_string("ui/colors/stopped");
			break;
		case Torrent::QUEUED:
			color = sm->get_string("ui/colors/queued");
			break;
		case Torrent::ERROR:
			color = sm->get_string("ui/colors/error");
			break;
		default:
			color = sm->get_string("ui/colors/stopped");
			break;
	}

	Glib::ustring name = torrent->get_name();
	int name_max = sm->get_int("ui/torrent_view/max_name_width");
	if (sm->get_bool("ui/torrent_view/trunkate_names") && name.size() > name_max)
		name = name.substr(0, name_max) + "...";
	name = Glib::Markup::escape_text(name);


	Glib::ustring format;
	libtorrent::torrent_status status = torrent->get_status();
	if (state & Torrent::DOWNLOADING || state & Torrent::SEEDING ||
		(state & Torrent::FINISHED && !torrent->is_stopped()))
	{
		if (status.num_complete != -1 && status.num_incomplete != -1)
			format = String::ucompose(_(
				"<span foreground='%1'><b>%2</b> (%3)</span>\n"
				"%4 (%5) connected seeds, %6 (%7) peers"),
				color, name, suffix_value(torrent->get_info()->total_size()),
				status.num_seeds, status.num_complete,
				status.num_peers - status.num_seeds, status.num_incomplete);
		else
			format = String::ucompose(_(
				"<span foreground='%1'><b>%2</b> (%3)</span>\n"
				"%4 connected seeds, %5 peers"),
				color, name, suffix_value(torrent->get_info()->total_size()),
				status.num_seeds, status.num_peers - status.num_seeds);
	}
	else
		format = String::ucompose(
			"<span foreground='%1'><b>%2</b> (%3)</span>\n%4",
			color, name, suffix_value(torrent->get_info()->total_size()),
			torrent->get_state_string(state));

	return format;
}

void TorrentList::format_rates(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter)
{
	Gtk::TreeRow row = *iter;
	CellRendererProgressText* cell_pt = dynamic_cast<CellRendererProgressText*>(cell);

	cell_pt->property_text_left() = suffix_value(row[columns.down_rate]) + "/s";
	cell_pt->property_text_right() = suffix_value(row[columns.up_rate]) + "/s";
}

bool TorrentList::on_button_press_event(GdkEventButton* event)
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

	if (event->button == 1 && event->type == GDK_2BUTTON_PRESS)
		m_signal_double_click.emit(event);
	else if (event->button == 3)
		m_signal_right_click.emit(event);

	return (event->button != 1);
}

sigc::signal<void, GdkEventButton*> TorrentList::signal_double_click()
{
	return m_signal_double_click;
}

sigc::signal<void, GdkEventButton*> TorrentList::signal_right_click()
{
	return m_signal_right_click;
}


void TorrentList::update()
{
	// sorting mess up iteration when we change the values in the sort column
	Gtk::SortType order;
	int col;
	model->get_sort_column_id(col, order);
	model->set_sort_column_id(Gtk::TreeSortable::DEFAULT_UNSORTED_COLUMN_ID, order);

	Gtk::TreeNodeChildren children = model->children();
	for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
	{
		Gtk::TreeRow row = *iter;

		// FIXME: don't update if row is invisible/filtered
		update_row(row);
	}

	model->set_sort_column_id(col, order);
}

void TorrentList::update_row(Gtk::TreeRow& row)
{
	Glib::RefPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(row[columns.hash]);

	Glib::ustring old_state = row[columns.state];
	unsigned int old_peers = row[columns.peers];
	unsigned int old_seeds = row[columns.seeds];
	unsigned int old_position = row[columns.position];

	//row[columns.name_formated] = get_formated_name(torrent);
	if (old_position != torrent->get_position())
		row[columns.position] = torrent->get_position();

	Glib::ustring state_string = torrent->get_state_string();
	// update formated name column only if needed
	bool state_changed = (old_state != state_string);
	if (state_changed)
	{
		row[columns.state] = state_string;
		row[columns.name_formated] = get_formated_name(torrent);
	}

	// don't continue if we don't need to, possibly include more states here..
	int state = torrent->get_state();
	if (!state_changed && state & Torrent::STOPPED)
		return;

	libtorrent::size_type up = torrent->get_total_uploaded();
	libtorrent::size_type down = torrent->get_total_downloaded();

	row[columns.down] = down;
	row[columns.up] = up;

	if (torrent->is_stopped())
	{
		row[columns.down_rate] = 0;
		row[columns.up_rate] = 0;
		row[columns.seeds] = 0;
		row[columns.peers] = 0;
		if (!torrent->is_completed())
		{
			float progress = 0;
			if (down)
			{
				libtorrent::size_type wanted_size = torrent->get_info()->total_size();
				std::vector<int> priorities = torrent->get_priorities();
				for (unsigned int i = 0; i < priorities.size(); i++)
				{
					// priority 0 means "don't download"
					if (!priorities[i])
						wanted_size -= torrent->get_info()->file_at(i).size;
				}
				
				progress = (1.0f*down)/(1.0f*wanted_size) * 100;
				if (progress > 100)
					progress = 100;
			}
			row[columns.progress] = progress;
			row[columns.eta] = String::ucompose("%1 %%", std::fixed, std::setprecision(2), progress);
			return;
		}
	}

	libtorrent::torrent_status status = torrent->get_status();
	if (state & Torrent::SEEDING || state & Torrent::FINISHED)
	{
		libtorrent::size_type size = down - up;
		if (down != 0)
		{
			float ratio = (1.0f*up)/(1.0f*down);

			if (ratio < 1.0f)
			{
				row[columns.progress] = ratio*100;
				row[columns.eta] = String::ucompose("%1 %2", std::fixed,
					std::setprecision(3),
					ratio, get_eta(size, status.upload_payload_rate));
			}
		 	else
		 	{
		 		row[columns.progress] = 100;
		 		row[columns.eta] = String::ucompose("%1", std::fixed, std::setprecision(3), ratio);
		 	}
		}
		else
		{
			if (up != 0)
			{
				row[columns.eta] = "\u221E";
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
		double progress = status.progress*100;
		row[columns.progress] = progress;
		row[columns.eta] = String::ucompose("%1 %% %2", std::fixed,
			std::setprecision(2),
			progress,
			get_eta(status.total_wanted - status.total_wanted_done,
				status.download_payload_rate));
	}
	row[columns.down_rate] = status.download_payload_rate;
	row[columns.up_rate] = status.upload_payload_rate;
	row[columns.seeds] = status.num_seeds;
	unsigned int peers = status.num_peers - status.num_seeds;
	row[columns.peers] = peers;

	// if we haven't already done it, update formated name if peers/seeds has changed
	if (!state_changed)
	{
		if (old_seeds != status.num_seeds || old_peers != peers)
			row[columns.name_formated] = get_formated_name(torrent);
	}
}

