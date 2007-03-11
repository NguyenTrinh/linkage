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
	filter->set_visible_func(sigc::mem_fun(this, &TorrentList::on_filter));

	set_model(filter);

	get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);

	append_column("#", columns.position);
	append_column("Name", columns.name);
	CellRendererProgressText* renderer = manage(new CellRendererProgressText());
	append_column("Progress", *renderer);
	Gtk::TreeViewColumn* column = get_column(COL_PROGRESS);
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

	Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();

	/* FIXME: Add option to trunkate names */
	Gtk::SortType sort_order = Gtk::SortType(sm->get_int("UI", "SortOrder"));
	model->set_sort_column_id(sm->get_int("UI", "SortColumn"), sort_order);

	Glib::RefPtr<TorrentManager> tm = Engine::get_torrent_manager();
	tm->signal_added().connect(sigc::mem_fun(*this, &TorrentList::on_added));
	tm->signal_removed().connect(sigc::mem_fun(*this, &TorrentList::on_removed));
}

TorrentList::~TorrentList()
{
	Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();

	int column;
	Gtk::SortType order;
	model->get_sort_column_id(column, order);

	sm->set("UI", "SortOrder", int(order));
	sm->set("UI", "SortColumn", column);

	PathList path_list = get_selection()->get_selected_rows();
	if (path_list.size() == 1)
	{
		Gtk::TreePath path = filter->convert_path_to_child_path(*path_list.begin());
		Gtk::TreeIter iter = model->get_iter(path);
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

bool TorrentList::on_filter(const Gtk::TreeModel::const_iterator& iter)
{
	Gtk::TreeRow row = *iter;
	sha1_hash hash = row[columns.hash];
	WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
	if (!torrent)
		return false;

	return (m_active_group) ? m_active_group.eval(torrent) : true;
}

void TorrentList::on_filter_set(const Group& group)
{
	m_active_group = group;
	filter->refilter();
}

void TorrentList::on_filter_unset()
{
	m_active_group = Group();
	filter->refilter();
}

void TorrentList::set_filter_set_signal(sigc::signal<void, const Group&> signal)
{
	signal.connect(sigc::mem_fun(this, &TorrentList::on_filter_set));
}

void TorrentList::set_filter_unset_signal(sigc::signal<void> signal)
{
	signal.connect(sigc::mem_fun(this, &TorrentList::on_filter_unset));
}

void TorrentList::on_added(const sha1_hash& hash, const Glib::ustring& name, unsigned int position)
{
	Glib::ustring selected_hash = Engine::get_settings_manager()->get_string("UI", "Selected");
	WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
	
	Gtk::TreeIter iter = model->append();
	Gtk::TreeRow new_row = *iter;
	new_row[columns.hash] = hash;

	if (selected_hash == str(hash))
		get_selection()->select(filter->convert_child_iter_to_iter(iter));

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

void TorrentList::format_rates(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter)
{
	Gtk::TreeRow row = *iter;
	CellRendererProgressText* cell_pt = dynamic_cast<CellRendererProgressText*>(cell);

	cell_pt->property_text_left() = "DL: " + suffix_value(row[columns.down_rate]) + "/s";
	cell_pt->property_text_right() = "UL: " + suffix_value(row[columns.up_rate]) + "/s";
}

bool TorrentList::on_button_press_event(GdkEventButton* event)
{
	Gtk::TreePath path;
	Gtk::TreeViewColumn* column;
	int cell_x, cell_y;

	/* Don't reset a multi selection */
	if (event->button == 1 || get_selection()->get_selected_rows().size() <= 1)
		TreeView::on_button_press_event(event);

	if (!get_path_at_pos((int)event->x, (int)event->y, path, column, cell_x, cell_y) && event->button == 1)
		return false;

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

void TorrentList::update(const WeakPtr<Torrent>& torrent)
{
	Gtk::TreeRow row = *get_iter(torrent->get_hash());
	row[columns.position] = torrent->get_position();

	Glib::ustring color;
	switch (torrent->get_state())
	{
		case Torrent::ERROR:
			color = "#C22C22";
			break;
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

	size_type up = torrent->get_total_uploaded();
	size_type down = torrent->get_total_downloaded();
	
	row[columns.down] = down;
	row[columns.up] = up;

	Glib::ustring name = Glib::Markup::escape_text(torrent->get_name());

	if (!torrent->is_running())
	{
		ss << "<span foreground='" << color << "'><b>" << name.c_str() << "</b> (" << suffix_value(torrent->get_info().total_size()) << ")</span>\nStopped";
		row[columns.name] = ss.str();
		row[columns.down_rate] = 0;
		row[columns.up_rate] = 0;
		row[columns.seeds] = 0;
		row[columns.peers] = 0;
		row[columns.progress] = 0;
		row[columns.eta] = "";
		return;
	}

	torrent_status status = torrent->get_status();

	if (torrent->get_state() == Torrent::SEEDING)
	{
		size_type size = status.total_wanted_done - up;
		if (down != 0)
		{
			double ratio = (double)up/down;

			if (ratio < 1)
			{
				row[columns.progress] = ratio*100;
				row[columns.eta] = str(ratio, 3) + " " + get_eta(size, status.upload_payload_rate);
			}
		 	else
		 	{
		 		row[columns.progress] = 100;
		 		row[columns.eta] = str(ratio, 3);
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
		row[columns.progress] = double(status.progress*100);
		row[columns.eta] = str(double(status.progress*100), 2) + " % " + get_eta(status.total_wanted-status.total_wanted_done, status.download_payload_rate);
	}

	Torrent::State state = torrent->get_state();
	Glib::ustring s = suffix_value((state == Torrent::DOWNLOADING) ? down : up);

	ss << "<span foreground='" << color << "'><b>" << name.c_str() <<
		"</b> (" << s << " of " << suffix_value(torrent->get_info().total_size()) << ")" <<
		"</span>\n";
	if (state == Torrent::DOWNLOADING)
		ss << status.num_seeds << " connected seeds, " <<
					status.num_peers - status.num_seeds << " peers";
	else if (state == Torrent::SEEDING || state == Torrent::FINISHED)
		ss << status.num_peers - status.num_seeds << " connected peers";
	else
		ss << torrent->get_state_string();

	row[columns.name] = ss.str();
	row[columns.down_rate] = status.download_payload_rate;
	row[columns.up_rate] = status.upload_payload_rate;
	row[columns.seeds] = status.num_seeds;
	row[columns.peers] = status.num_peers - status.num_seeds;
}
