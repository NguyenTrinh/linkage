/*
Copyright (C) 2006-2008   Christian Lundgren
Copyright (C) 2007-2008   Dave Moore

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

#include "StateFilter.hh"
#include "GroupList.hh"
#include "CellRendererProgressText.hh"
#include "TorrentList.hh"

#include "linkage/Engine.hh"
#include "linkage/TorrentManager.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/Utils.hh"
#include "linkage/compose.hpp"

using namespace Linkage;

typedef Gtk::TreeSelection::ListHandle_Path PathList;

static GConfEnumStringPair sort_order_table[] = {
  { Gtk::SORT_ASCENDING, "SORT_ASCENDING" },
  { Gtk::SORT_DESCENDING, "SORT_DESCENDING" }
};


// 
// This must be in the same order as the TorrentList::Column enum
//
static GConfEnumStringPair sort_column_table[] = {
  { TorrentList::COL_POSITION, "position" },
  { TorrentList::COL_NAME, "name" },
  { TorrentList::COL_NAME_FORMATED, "name_formated" },
  { TorrentList::COL_PROGRESS, "progress" },
  { TorrentList::COL_STATUS, "status" },
  { TorrentList::COL_DOWNLOADED, "downloaded" },
  { TorrentList::COL_UPLOADED, "uploaded" },
  { TorrentList::COL_DOWNRATE, "downrate" },
  { TorrentList::COL_UPRATE, "uprate" },
  { TorrentList::COL_SEEDS, "seeds" },
  { TorrentList::COL_PEERS, "peers" },
  { TorrentList::COL_ETA, "eta" },
  { TorrentList::COL_RATIO, "ratio" },
  { TorrentList::COL_TRACKER, "tracker" },
  { TorrentList::COL_TRACKER_STATUS, "tracker_status" },
  { TorrentList::COL_HASH, "hash"}
};

TorrentList::TorrentList(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::TreeView(cobject),
	glade_xml(refGlade)
{
	SettingsManagerPtr sm = Engine::get_settings_manager();
	
	m_cols.push_back(EditColumnsDialog::ColumnData( "#", "position", "The priority of the torrent", true ));
	m_cols.push_back(EditColumnsDialog::ColumnData( "Name", "name", "The name of the torrent", true ));
	m_cols.push_back(EditColumnsDialog::ColumnData( "Name Formatted", "name_formatted", "The name of the torrent", false ));
	m_cols.push_back(EditColumnsDialog::ColumnData( "Progress", "progress", "The progress of the download", true ));
	m_cols.push_back(EditColumnsDialog::ColumnData( "Status", "status", "What the torrent is doing", true ));
	m_cols.push_back(EditColumnsDialog::ColumnData( "Downloaded", "downloaded", "Total amount downloaded", true ));
	m_cols.push_back(EditColumnsDialog::ColumnData( "Uploaded", "uploaded", "Total amount uploaded", true ));
	m_cols.push_back(EditColumnsDialog::ColumnData( "Down Speed", "downrate", "Download speed", true ));
	m_cols.push_back(EditColumnsDialog::ColumnData( "Up Speed", "uprate", "Upload speed", true ));
	m_cols.push_back(EditColumnsDialog::ColumnData( "Seeds", "seeds", "The number of seeds connected (Total number of seeds))", true ));
	m_cols.push_back(EditColumnsDialog::ColumnData( "Peers", "peers", "The number of peers connected (Total number of peers))", true ));
	m_cols.push_back(EditColumnsDialog::ColumnData( "ETA", "eta", "Estimated time until completion of download", true ));
	m_cols.push_back(EditColumnsDialog::ColumnData( "Ratio", "ratio", "Your ratio on this torrent", true ));
	m_cols.push_back(EditColumnsDialog::ColumnData( "Tracker", "tracker", "The address of the tracker", true ));
	m_cols.push_back(EditColumnsDialog::ColumnData( "Tracker Status", "tracker_status", "The response from the tracker", false ));
	m_cols.push_back(EditColumnsDialog::ColumnData( "Hash", "hash", "The info hash for the torrent", false ));
	
	
	m_cur_state = Torrent::NONE;

	model = Gtk::ListStore::create(columns);
	filter = Gtk::TreeModelFilter::create(model);
	filter->set_visible_func(sigc::mem_fun(this, &TorrentList::on_filter));

	set_model(filter);

	get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
	std::string mask = parse_column_string(sm->get_string("ui/torrent_view/columns/sort_order"));

	Gtk::CellRendererProgress* cell = manage(new Gtk::CellRendererProgress);
	
	for (int i = 0; i < TorrentList::TOTAL_COLS; i++)
	{
		Gtk::TreeViewColumn* pcol;
		int col_id;
		
		int c = int(mask[i]);
		
		switch (c)
		{
			case COL_POSITION:
				col_id = append_column(m_cols[COL_POSITION].title, columns.position);
				pcol = get_column(col_id - 1);
				break;
			case COL_NAME:
				col_id = append_column(m_cols[COL_NAME].title, columns.name);
				pcol = get_column(col_id - 1);
				break;
			case COL_NAME_FORMATED:
				col_id = append_column(m_cols[COL_NAME_FORMATED].title, columns.name_formated);
				pcol = get_column(col_id - 1);
				break;
			case COL_PROGRESS:				
				col_id = append_column(m_cols[COL_PROGRESS].title, *cell);
				pcol = get_column(col_id - 1);				
				pcol->add_attribute(cell->property_value(), columns.progress);
				break;
			case COL_STATUS:
				col_id = append_column(m_cols[COL_STATUS].title, columns.state);
				pcol = get_column(col_id - 1);
				break;
			case COL_DOWNLOADED:
				col_id = append_column(m_cols[COL_DOWNLOADED].title, columns.down);
				pcol = get_column(col_id - 1);
				break;
			case COL_UPLOADED:
				col_id = append_column(m_cols[COL_UPLOADED].title, columns.up);
				pcol = get_column(col_id - 1);
				break;
			case COL_DOWNRATE:
				col_id = append_column(m_cols[COL_DOWNRATE].title, columns.down_rate);
				pcol = get_column(col_id - 1);
				break;
			case COL_UPRATE:
				col_id = append_column(m_cols[COL_UPRATE].title, columns.up_rate);
				pcol = get_column(col_id - 1);
				break;
			case COL_SEEDS:
				col_id = append_column(m_cols[COL_SEEDS].title, columns.seeds);
				pcol = get_column(col_id - 1);
				break;
			case COL_PEERS:
				col_id = append_column(m_cols[COL_PEERS].title, columns.peers);
				pcol = get_column(col_id - 1);
				break;
			case COL_ETA:
				col_id = append_column(m_cols[COL_ETA].title, columns.eta);
				pcol = get_column(col_id - 1);
				break;
			case COL_RATIO:
				col_id = append_column(m_cols[COL_RATIO].title, columns.ratio);
				pcol = get_column(col_id - 1);
				break;
			case COL_TRACKER:
				col_id = append_column(m_cols[COL_TRACKER].title, columns.tracker);
				pcol = get_column(col_id - 1);
				break;
			case COL_TRACKER_STATUS:
				col_id = append_column(m_cols[COL_TRACKER_STATUS].title, columns.tracker_status);
				pcol = get_column(col_id - 1);
				break;
			case COL_HASH:
				col_id = append_column(m_cols[COL_HASH].title, columns.hash);
				pcol = get_column(col_id - 1);
				break;			
		}
		
		pcol->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);
		pcol->set_reorderable(true);
		pcol->set_resizable(true);
		pcol->set_min_width(1);
		int w = sm->get_int("ui/torrent_view/columns/col_" + m_cols[c].name + "_width");
		if (w > 0) {
			pcol->set_fixed_width(sm->get_int("ui/torrent_view/columns/col_" + m_cols[c].name + "_width"));
			pcol->queue_resize();
		}
		bool vis = sm->get_bool("ui/torrent_view/columns/col_" + m_cols[c].name + "_visible");
		pcol->set_visible(vis);
		m_cols[c].visible = vis;
	}
	
	//int col_id = append_column("#", columns.position);
	//col_id = append_column(_("Name"), columns.name_formated);
	//Gtk::TreeViewColumn* column = get_column(col_id - 1);
	//Gtk::CellRendererText* renderer_text = dynamic_cast<Gtk::CellRendererText*>
		//(column->get_first_cell_renderer());
	//column->clear_attributes(*renderer_text);
	//column->add_attribute(*renderer_text, "markup", columns.name_formated);
	//column->set_expand(true);
	//CellRendererProgressText* renderer = manage(new CellRendererProgressText());
	//col_id = append_column(_("Progress"), *renderer);
	//column = get_column(col_id - 1);
	//column->add_attribute(*renderer, "value", columns.progress);
	//column->add_attribute(*renderer, "text", columns.eta);
	////column->add_attribute(*renderer, "text-left", columns.down_rate_formated);
	////column->add_attribute(*renderer, "text-right", columns.up_rate_formated);
	//column->set_cell_data_func(*renderer, sigc::mem_fun(this, &TorrentList::format_rates));

	set_headers_visible(true);
	set_search_column(columns.name);

	Gtk::SortType order = (Gtk::SortType)sm->get_enum("ui/torrent_view/order", sort_order_table);
	gint col = sm->get_enum("ui/torrent_view/column", sort_column_table);
	model->set_sort_column_id(col, order);

	TorrentManagerPtr tm = Engine::get_torrent_manager();
	TorrentManager::TorrentList torrents = tm->get_torrents();
	std::for_each(torrents.begin(), torrents.end(), sigc::mem_fun(this, &TorrentList::on_added));

	PathList paths = get_selection()->get_selected_rows();
	if (!paths.empty())
		scroll_to_row(*paths.begin(), 0.5);

	tm->signal_added().connect(sigc::mem_fun(*this, &TorrentList::on_added));
	tm->signal_removed().connect(sigc::mem_fun(*this, &TorrentList::on_removed));

	/* Connect update signals */
	GroupList* group_list;
	glade_xml->get_widget_derived("groups_treeview1", group_list);
	group_list->signal_filter_set().connect(sigc::mem_fun(this, &TorrentList::on_filter_set));
	StateFilter* state_filter;
	glade_xml->get_widget_derived("state_combobox", state_filter);
	state_filter->signal_state_filter_changed().connect(sigc::mem_fun(this, &TorrentList::on_state_filter_changed));
}

TorrentList::~TorrentList()
{
	SettingsManagerPtr sm = Engine::get_settings_manager();

	Gtk::SortType order;
	int column;
	model->get_sort_column_id(column, order);
	sm->set("ui/torrent_view/order", sort_order_table, order);
	sm->set("ui/torrent_view/column", sort_column_table, column);
	
	save_column_setup();

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

sigc::signal<void, GdkEventButton*> TorrentList::signal_double_click()
{
	return m_signal_double_click;
}

sigc::signal<void, GdkEventButton*> TorrentList::signal_right_click()
{
	return m_signal_right_click;
}

Glib::SignalProxy0<void> TorrentList::signal_changed()
{
	return get_selection()->signal_changed();
}

bool TorrentList::on_filter(const Gtk::TreeModel::const_iterator& iter)
{
	Gtk::TreeRow row = *iter;
	libtorrent::sha1_hash hash = row[columns.hash];
	TorrentPtr torrent = Engine::get_torrent_manager()->get_torrent(hash);
	if (!torrent)
		return false;

	if (m_cur_state && !(m_cur_state & torrent->get_state()))
		return false;

	return (m_active_group) ? m_active_group->eval(torrent) : true;
}

void TorrentList::on_filter_set(const GroupPtr& group)
{
	m_active_group = group;
	/* FIXME: This hits an endless loop if we have a multi selection */
	filter->refilter();
}

void TorrentList::on_state_filter_changed(Torrent::State state)
{
	m_cur_state = state;
	filter->refilter();
}

void TorrentList::on_added(const TorrentPtr& torrent)
{
	Glib::ustring hash_str = Engine::get_settings_manager()->get_string("ui/torrent_view/selected");

	Gtk::TreeIter iter = model->append();
	Gtk::TreeRow row = *iter;
	row[columns.hash] = torrent->get_hash();
	row[columns.name] = torrent->get_name();

	row[columns.position] = torrent->get_position();
	row[columns.name_formated] = get_formated_name(torrent);
	if (Torrent::state_string(torrent->get_state()) != row[columns.state])
		on_state_changed(WeakTorrentPtr(torrent));

	if (hash_str == String::compose("%1", torrent->get_hash()))
		get_selection()->select(filter->convert_child_iter_to_iter(iter));
	row[columns.down] = suffix_value(torrent->get_previously_downloaded());
	row[columns.up] = suffix_value(torrent->get_previously_uploaded());
	torrent->property_position().signal_changed().connect(sigc::bind(
		sigc::mem_fun(this, &TorrentList::on_position_changed),
		WeakTorrentPtr(torrent)));
	torrent->property_state().signal_changed().connect(sigc::bind(
		sigc::mem_fun(this, &TorrentList::on_state_changed),
		WeakTorrentPtr(torrent)));

	update_row(row);
}

void TorrentList::on_removed(const TorrentPtr& torrent)
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

void TorrentList::on_position_changed(const WeakTorrentPtr& weak)
{
	TorrentPtr torrent = weak.lock();

	Gtk::TreeIter iter;
	Gtk::TreeNodeChildren children = model->children();
	for (iter = children.begin(); iter != children.end(); ++iter)
	{
		Gtk::TreeRow row = *iter;
		if (torrent->get_hash() == row[columns.hash])
		{
			row[columns.position] = torrent->get_position();
			break;
		}
	}

	if (!iter)
		return;

	int current_col_id = 0;
	Gtk::SortType current_order;
	model->get_sort_column_id(current_col_id, current_order);
	if (current_col_id == COL_POSITION)
		scroll_to_row(model->get_path(iter));
}

void TorrentList::on_state_changed(const WeakTorrentPtr& weak)
{
	TorrentPtr torrent = weak.lock();

	Gtk::TreeNodeChildren children = model->children();
	for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
	{
		Gtk::TreeRow row = *iter;
		if (torrent->get_hash() == row[columns.hash])
		{
			row[columns.state] = Torrent::state_string(torrent->get_state());
			if (torrent->is_stopped())
			{
				row[columns.down_rate] = "0";
				row[columns.up_rate] = "0";
				row[columns.seeds] = "0";
				row[columns.peers] = "0";
			}
			row[columns.name_formated] = get_formated_name(torrent);

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

Glib::ustring TorrentList::get_formated_name(const TorrentPtr& torrent)
{
	SettingsManagerPtr sm = Engine::get_settings_manager();
	int state = torrent->get_state();
	Glib::ustring color;

	if (state & Torrent::CHECK_QUEUE)
		color = sm->get_string("ui/colors/check_queue");
	else if (state & Torrent::CHECKING)
		color = sm->get_string("ui/colors/checking");
	else if (state & Torrent::ANNOUNCING)
		color = sm->get_string("ui/colors/announcing");
	else if (state & Torrent::STOPPED)
		color = sm->get_string("ui/colors/stopped");
	else if (state & Torrent::DOWNLOADING)
		color = sm->get_string("ui/colors/downloading");
	else if (state & Torrent::FINISHED)
		color = sm->get_string("ui/colors/finished");
	else if (state & Torrent::SEEDING)
		color = sm->get_string("ui/colors/seeding");
	else if (state & Torrent::ALLOCATING)
		color = sm->get_string("ui/colors/allocating");
	else if (state & Torrent::QUEUED)
		color = sm->get_string("ui/colors/queued");
	else if (state & Torrent::ERROR)
		color = sm->get_string("ui/colors/error");

	Glib::ustring name = torrent->get_name();
	int name_max = sm->get_int("ui/torrent_view/max_name_width");
	if (sm->get_bool("ui/torrent_view/trunkate_names") && (int)name.size() > name_max)
		name = name.substr(0, name_max) + "...";
	name = Glib::Markup::escape_text(name);

	//libtorrent::torrent_status status = torrent->get_status();
	//if (state & Torrent::DOWNLOADING || state & Torrent::SEEDING ||
		//(state & Torrent::FINISHED && !torrent->is_stopped()))
	//{
		//if (status.num_complete != -1 && status.num_incomplete != -1)
			//return String::ucompose(
				//"<span foreground='%1'><b>%2</b> (%3)</span>\n"
				//"%4 (%5) %8, %6 (%7) %9",
				//color, name, suffix_value(torrent->get_info()->total_size()),
				//status.num_seeds, status.num_complete,
				//status.num_peers - status.num_seeds, status.num_incomplete,
				//_("connected seeds"), _("peers"));
		//else
			 //return String::ucompose(
				//"<span foreground='%1'><b>%2</b> (%3)</span>\n"
				//"%4 %6, %5 %7",
				//color, name, suffix_value(torrent->get_info()->total_size()),
				//status.num_seeds, status.num_peers - status.num_seeds,
				//_("connected seeds"), _("peers"));
	//}
	//else
		//return String::ucompose(
			//"<span foreground='%1'><b>%2</b> (%3)</span>\n%4",
			//color, name, suffix_value(torrent->get_info()->total_size()),
			//Torrent::state_string(state));

	return String::ucompose("<span foreground='%1'><b>%2</b></span>", color, name);
}

void TorrentList::format_rates(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter)
{
	Gtk::TreeRow row = *iter;
	CellRendererProgressText* cell_pt = dynamic_cast<CellRendererProgressText*>(cell);

	//cell_pt->property_text_left() = suffix_value(row[columns.down_rate]) + "/s";
	//cell_pt->property_text_right() = suffix_value(row[columns.up_rate]) + "/s";
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

	TorrentPtr torrent = Engine::get_torrent_manager()->get_torrent(row[columns.hash]);
	libtorrent::torrent_status status = torrent->get_status();

	libtorrent::size_type down = status.total_payload_download +
			torrent->get_previously_downloaded();
	libtorrent::size_type up = status.total_payload_upload +
			torrent->get_previously_uploaded();

	row[columns.down] = suffix_value(down);
	row[columns.up] = suffix_value(up);
	row[columns.ratio] = (1.0f*up)/(1.0f*down);

	int state = torrent->get_state();
	if (state & Torrent::SEEDING || state & Torrent::FINISHED)
	{
		libtorrent::size_type size = down - up;
		if (down != 0)
		{
			float ratio = (1.0f*up)/(1.0f*down);

			if (ratio < 1.0f)
			{
				row[columns.progress] = ratio*100;
				row[columns.eta] = String::ucompose("%1", std::fixed,
					std::setprecision(3),
					get_eta(size, status.upload_payload_rate));
			}
			else
		 	{
		 		row[columns.progress] = 100;
		 		row[columns.eta] = "\u221E";
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
				row[columns.eta] = get_eta(size, status.upload_payload_rate);
				row[columns.progress] = 0;
			}
		}
	}
	else
	{
		float progress = status.progress*100;
		if (progress != row[columns.progress])
		{
			row[columns.progress] = progress;
			if (state & Torrent::STOPPED)
				row[columns.eta] = String::ucompose("%1 %%", std::fixed,
					std::setprecision(2),
					progress);
			else
				row[columns.eta] = String::ucompose("%1 %% %2", std::fixed,
					std::setprecision(2),
					progress,
					get_eta(status.total_wanted - status.total_wanted_done,
						status.download_payload_rate));
		}
	}
	
	if (state & Torrent::STOPPED)
		return;

	row[columns.down_rate] = suffix_value(status.download_payload_rate) + "/s";
	row[columns.up_rate] = suffix_value(status.upload_payload_rate) + "/s";
		
	Glib::ustring seeds =  String::ucompose("%1", std::fixed, std::setprecision(3), status.num_seeds);
	if (status.num_complete != -1)
		seeds = String::ucompose("%1 (%2)", std::fixed, std::setprecision(3), 
				status.num_seeds, status.num_complete);
	if (seeds != row[columns.seeds])
		row[columns.seeds] = seeds;

	
	Glib::ustring peers =  String::ucompose("%1", std::fixed, std::setprecision(3), status.num_seeds);
	if (status.num_complete != -1)
		peers = String::ucompose("%1 (%2)", std::fixed, std::setprecision(3), 
				status.num_peers - status.num_seeds, status.num_incomplete);
	if (peers != row[columns.peers])
		row[columns.peers] = peers;
}

void TorrentList::save_column_setup()
{
	Gtk::TreeViewColumn* col;
	Glib::ustring order;
	SettingsManagerPtr sm = Engine::get_settings_manager();

	for (int i = 0; i < TorrentList::TOTAL_COLS; i++)
	{
		col = get_column(i);
		// foreach column in the list
		for (int j = 0; j < TorrentList::TOTAL_COLS; j++)
		{
			if (m_cols[j].title == col->get_title())
			{
				if (col->get_width() > 0)
					sm->set("ui/torrent_view/columns/col_" + m_cols[j].name + "_width", col->get_width());
				sm->set("ui/torrent_view/columns/col_" + m_cols[j].name + "_visible", col->get_visible());
				order += "|";
				order += m_cols[j].name;
			}
		}
	}

	sm->set("ui/torrent_view/columns/sort_order", order);
}

std::string TorrentList::parse_column_string(std::string raw_order)
{
	std::string mask;
		
	std::string::size_type lastPos = raw_order.find_first_not_of('|', 0);
	std::string::size_type pos = raw_order.find_first_of('|', lastPos);
		
	while (std::string::npos != pos || std::string::npos != lastPos)
	{
		char c;
		for (int i = 0; i < TorrentList::TOTAL_COLS; i++)
		{
			if (m_cols[i].name == raw_order.substr(lastPos, pos - lastPos))
			{
				c = char(i);
			}
		}
		mask += c;
		lastPos = raw_order.find_first_not_of('|', pos);
		pos = raw_order.find_first_of('|', lastPos);
	}
	return mask;
}

void TorrentList::toggle_column(int col_id)
{
	SettingsManagerPtr sm = Engine::get_settings_manager();
	Gtk::TreeViewColumn* col;
	
	for (int i = 0; i < TOTAL_COLS; i++)
	{
		col = get_column(i);
		if (m_cols[col_id].title == col->get_title())
		{
			 m_cols[col_id].visible = !m_cols[col_id].visible;
			 col->set_visible(m_cols[col_id].visible);
			 sm->set("ui/torrent_view/columns/col_" + m_cols[col_id].name + "_visible", m_cols[col_id].visible);
		}
	}
}
