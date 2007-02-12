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
	model = Gtk::TreeStore::create(columns);

	set_model(model);

	get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);

	append_column("#", columns.number);
	append_column("Name", columns.name);
	CellRendererProgressText* render = new CellRendererProgressText();
	append_column("Progress", *Gtk::manage(render));
	Gtk::TreeViewColumn* column = get_column(2);
	column->add_attribute(*render, "value", COL_PROGRESS);
	column->add_attribute(*render, "text", COL_ETA);
	column->add_attribute(*render, "text1", COL_DOWNRATE);
	column->add_attribute(*render, "text2", COL_UPRATE);
	column->set_cell_data_func(*render, sigc::mem_fun(this, &TorrentList::format_data));
	
	for(unsigned int i = 0; i < 3; i++)
	{
		Gtk::TreeView::Column* column = get_column(i);
		column->set_sort_column_id(i + 1);
		if (i == 1)
		{
			std::vector<Gtk::CellRenderer*> renders = column->get_cell_renderers();
			column->clear_attributes(*renders[0]);
			column->add_attribute(*renders[0], "markup", 2);
		}
		column->set_resizable(true);
	}
	set_headers_visible(false);
	set_expander_column(*get_column(1));
	
	Glib::RefPtr<SettingsManager> sm = Engine::instance()->get_settings_manager();
	std::list<Glib::ustring> groups = sm->get_keys("Groups");
	for (std::list<Glib::ustring>::iterator iter = groups.begin(); iter != groups.end(); ++iter)
	{
		add_group(*iter);
	}
	
	Gtk::SortType sort_order = Gtk::SortType(sm->get_int("UI", "SortOrder"));
	model->set_sort_column_id(sm->get_int("UI", "SortColumn"), sort_order);
	
	Glib::RefPtr<TorrentManager> tm = Engine::instance()->get_torrent_manager();
	tm->signal_position_changed().connect(sigc::mem_fun(*this, &TorrentList::on_position_changed));
	tm->signal_group_changed().connect(sigc::mem_fun(*this, &TorrentList::on_group_changed));
	tm->signal_added().connect(sigc::mem_fun(*this, &TorrentList::on_added));
	tm->signal_removed().connect(sigc::mem_fun(*this, &TorrentList::on_removed));
	
	Engine::instance()->get_session_manager()->signal_session_resumed().connect(sigc::mem_fun(*this, &TorrentList::on_session_resumed)); // FIXME: This is a ugly hack =(
	
	//FIXME: Get stored GroupFilters from SettingsManager
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
			Glib::ustring hash_str = str(row[columns.hash]);
			sm->set("UI", "Selected", hash_str);
		}
	}
	
	Gtk::TreeNodeChildren children = model->children();
	for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
	{
		Gtk::TreePath path = model->get_path(iter);
		Gtk::TreeRow row = *iter;
		Glib::ustring name = row[columns.name];
		name = name.substr(3, name.size()-7);
		sm->set("Groups", name, row_expanded(path));
	}
	
	for (std::list<GroupFilter*>::iterator iter = filters.begin();
				iter != filters.end(); ++iter)
	{
		delete *iter;
	}
				
}

Glib::SignalProxy0<void> TorrentList::signal_changed()
{
	return get_selection()->signal_changed();
}

void TorrentList::on_session_resumed()
{
	Glib::RefPtr<SettingsManager> sm = Engine::instance()->get_settings_manager();
	Gtk::TreeNodeChildren children = model->children();
	for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
	{
		Gtk::TreePath path = model->get_path(iter);
		Gtk::TreeRow row = *iter;
		Glib::ustring name = row[columns.name];
		/* Strip <i> tag from name */
		name = name.substr(3, name.size()-7);
		if (sm->get_bool("Groups", name))
			expand_row(path, false);
	}
	
	Glib::ustring selected_hash = sm->get_string("UI", "Selected");
	Gtk::TreeIter iter;
	
	Gtk::TreeNodeChildren parents = model->children();
	for (Gtk::TreeIter parent_iter = parents.begin();
			 parent_iter != parents.end(); ++parent_iter)
	{
		Gtk::TreeRow row = *parent_iter;
		Gtk::TreeNodeChildren children = row.children();
		for (Gtk::TreeIter child_iter = children.begin();
					child_iter != children.end(); ++child_iter)
		{
			Gtk::TreeRow child_row = *child_iter;
			if (selected_hash == str(child_row[columns.hash]))
				iter = child_iter;
		}
	}
	
	if (iter)
		get_selection()->select(iter);
}

void TorrentList::on_size_allocate(Gtk::Allocation& allocation)
{
	get_column(1)->set_min_width(int(allocation.get_width()/1.5));
	Gtk::Widget::on_size_allocate(allocation);
}

void TorrentList::on_position_changed(const sha1_hash& hash, unsigned int position)
{
	Gtk::TreeIter iter = get_iter(hash);
	
	if (iter)
	{
		Gtk::TreeRow row = *iter;
		row[columns.number] = position;
	}
}

void TorrentList::on_group_changed(const sha1_hash& hash, const Glib::ustring& group)
{
	Gtk::TreeIter iter = get_iter(hash);
	if (iter)
	{
		bool selected = is_selected(hash); //FIXME: Doesn't honour a multiple selection
		
		Gtk::TreeRow row = *iter;
		Gtk::TreeRow group_row = *(row.parent()); //Get the current group
		
		if ("<i>" + group + "</i>" != group_row[columns.name]) //Make sure current and target group differs
		{
			group_row = *get_iter(group); //Get the target group
			
			model->erase(iter);
			
			Gtk::TreeRow new_row = *(model->append(group_row.children()));
			new_row[columns.hash] = hash;
			if (selected)
				get_selection()->select(new_row);
		}
	}
}

void TorrentList::on_added(const sha1_hash& hash, const Glib::ustring& name, const Glib::ustring& group, unsigned int position)
{
	Glib::ustring filter_group = group;
	WeakPtr<Torrent> torrent = Engine::instance()->get_torrent_manager()->get_torrent(hash);
	for (std::list<GroupFilter*>::iterator iter = filters.begin();
				iter != filters.end(); ++iter)
	{
		GroupFilter* filter = *iter;
		if (filter->eval(torrent))
		{
			filter_group = filter->get_name();
			break;
		}
	}
	
	Gtk::TreeRow group_row = *get_iter(group);
	Gtk::TreeRow new_row = *(model->append(group_row.children()));
	new_row[columns.hash] = hash;
	new_row[columns.name] = name;
	new_row[columns.number] = position;
	
	torrent->set_group(filter_group);
}

void TorrentList::on_removed(const sha1_hash& hash)
{
	Gtk::TreeIter iter = get_iter(hash);
	model->erase(iter);
}

Gtk::TreeIter TorrentList::get_iter(const Glib::ustring& group)
{
	Gtk::TreeIter match;
	
	Gtk::TreeNodeChildren children = model->children();
	for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
	{
		Gtk::TreeRow row = *iter;
		if ("<i>" + group + "</i>" == row[columns.name])
			match = iter;
	}
	return match;
}

Gtk::TreeIter TorrentList::get_iter(const sha1_hash& hash)
{
	Gtk::TreeIter match;
	
	Gtk::TreeNodeChildren parents = model->children();
	for (Gtk::TreeIter parent_iter = parents.begin();
			 parent_iter != parents.end(); ++parent_iter)
	{
		Gtk::TreeRow row = *parent_iter;
		Gtk::TreeNodeChildren children = row.children();
		for (Gtk::TreeIter child_iter = children.begin();
					child_iter != children.end(); ++child_iter)
		{
			Gtk::TreeRow child_row = *child_iter;
			if (hash == child_row[columns.hash])
				match = child_iter;
		}
	}
	return match;
}

bool TorrentList::is_selected(const sha1_hash& hash)
{
	PathList path_list = get_selection()->get_selected_rows();
	if (path_list.size() == 1)
	{
		Gtk::TreeIter iter = model->get_iter(*path_list.begin());
		if (iter)
		{
			Gtk::TreeRow row = *iter;
			return (hash == row[columns.hash]);
		}
	}
	return false;
}

HashList TorrentList::get_selected_list()
{
	PathList path_list = get_selection()->get_selected_rows();
	
	//Sort the selected list by columns.number to ease moving
	std::list<Gtk::TreeRow> ordered_list;
	for (PathList::iterator iter = path_list.begin();
				iter != path_list.end(); ++iter)
	{
		Gtk::TreeRow row = *(model->get_iter(*iter));
		if (row.parent()) //Only add torrents, not groups
		{
			for (std::list<Gtk::TreeRow>::iterator liter = ordered_list.begin();
						liter != ordered_list.end(); ++liter)
			{
				if (row[columns.number] < (*liter)[columns.number])
					ordered_list.insert(liter, row);
			}
			ordered_list.push_back(row);
		}
	}
	ordered_list.unique();
	HashList list;
	for (std::list<Gtk::TreeRow>::iterator iter = ordered_list.begin();
			 iter != ordered_list.end(); ++iter)
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
	
void TorrentList::select(const Glib::ustring& path)
{
	Gtk::TreeIter iter = model->get_iter(path);
	if (iter)
		get_selection()->select(iter);
}

void TorrentList::format_data(Gtk::CellRenderer* cell, const Gtk::TreeIter& iter)
{
	Gtk::TreeRow row = *iter;
	CellRendererProgressText* cell_pt = dynamic_cast<CellRendererProgressText*>(cell);
	
	if (!row.parent())
	{
		/* Don't set text for unexpanded parents */
		if (row_expanded(model->get_path(iter)) || !row.children().size())
		{
			cell_pt->property_text1() = "";
			cell_pt->property_text2() = "";
		}
		else
		{
			cell_pt->property_text1() = suffix_value(row[columns.down_rate]) + "/s";
			cell_pt->property_text2() = suffix_value(row[columns.up_rate]) + "/s";
		}
	}
	else
	{
		cell_pt->property_text1() = suffix_value(row[columns.down_rate]) + "/s";
		cell_pt->property_text2() = suffix_value(row[columns.up_rate]) + "/s";
	}
}

void TorrentList::update_groups()
{
	Gtk::TreeNodeChildren parents = model->children();
	for (Gtk::TreeIter group_iter = parents.begin();
			 group_iter != parents.end(); ++group_iter)
	{
		Gtk::TreeRow group_row = *group_iter;
		
		unsigned int peers = 0, seeds = 0, up = 0, down = 0, up_rate = 0, down_rate = 0;
		double progress = 0;
		Gtk::TreeNodeChildren children = group_row.children();
		for (Gtk::TreeIter iter = children.begin();
				iter != children.end(); ++iter)
		{
			Gtk::TreeRow row = *iter;
			peers += row[columns.peers];
			seeds += row[columns.seeds];
			up += row[columns.up];
			down += row[columns.down];
			up_rate += row[columns.up_rate];
			down_rate += row[columns.down_rate];
			progress += row[columns.progress];
		}
		group_row[columns.number] = children.size();
		group_row[columns.peers] = peers;
		group_row[columns.seeds] = seeds;
		group_row[columns.up] = up;
		group_row[columns.down] = down;
		group_row[columns.up_rate] = up_rate;
		group_row[columns.down_rate] = down_rate;
		
		if (children.size() != 0)
		{
			group_row[columns.progress] = progress/children.size();
			group_row[columns.eta] = "Average " + str(progress/children.size(), 2) + " %";
		}
		else
		{
			group_row[columns.progress] = 0;
			group_row[columns.eta] = "";
		}
	}
}

Gtk::TreeIter TorrentList::add_group(const Glib::ustring& name)
{
	Gtk::TreeIter iter = model->append();
	Gtk::TreeRow row = *iter;
	row[columns.name] = "<i>" + name + "</i>";
	return iter;
}

void TorrentList::update_row(const WeakPtr<Torrent>& torrent)
{			
	Gtk::TreeRow row = *get_iter(torrent->get_hash());
	row[columns.number] = torrent->get_position();
	
	Glib::ustring color;
	switch (torrent->get_state())
	{
		case Torrent::STOPPED:
			color = "#999999";
			break;
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
	
	if (!torrent->is_running())
	{
		ss << "<span foreground='" << color << "'><b>" << torrent->get_name() << "</b></span>\nStopped";
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
		unsigned int size = status.total_wanted_done - up;
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
	
	ss << "<span foreground='" << color << "'><b>" << torrent->get_name() <<
		"</b> (" << suffix_value((int)torrent->get_info().total_size()) << ")" <<
		"</span>\n";
	if (torrent->get_state() != Torrent::QUEUED)
		ss << status.num_seeds << " connected seeds, " <<
					status.num_peers - status.num_seeds << " peers";
	else
		ss << "Queued";

	row[columns.name] = ss.str();
		
	row[columns.down_rate] = (unsigned int)status.download_payload_rate;
	row[columns.up_rate] = (unsigned int)status.upload_payload_rate;
	row[columns.seeds] = status.num_seeds;
	row[columns.peers] = status.num_peers - status.num_seeds;
}
