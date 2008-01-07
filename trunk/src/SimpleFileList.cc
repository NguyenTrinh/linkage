/*
Copyright (C) 2007	Christian Lundgren

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

#include <gtkmm/cellrenderertext.h>
#include <gtkmm/cellrenderertoggle.h>
#include <glibmm/i18n.h>

#include "SimpleFileList.hh"

#include "linkage/Utils.hh"

SimpleFileList::SimpleFileList(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::TreeView(cobject)
{
	model = Gtk::ListStore::create(columns);

	set_model(model);

	Gtk::CellRendererToggle* toggle_render = manage(new Gtk::CellRendererToggle());
	toggle_render->signal_toggled().connect(sigc::mem_fun(*this, &SimpleFileList::on_filter_toggled));
	int cols_count = append_column(_("Filter"), *toggle_render);
	Gtk::TreeViewColumn* column = get_column(cols_count - 1);
	column->add_attribute(*toggle_render, "active", columns.filter);
	column->set_sort_column(columns.filter);
	column->set_resizable(true);

	cols_count = append_column(_("Name"), columns.name);
	column = get_column(cols_count - 1);
	column->set_sort_column(columns.name);
	column->set_resizable(true);

	cols_count = append_column(_("Size"), columns.size);
	column = get_column(cols_count - 1);
	Gtk::CellRendererText* cell = dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell_renderer());
	column->set_cell_data_func(*cell, sigc::bind(sigc::mem_fun(this, &SimpleFileList::format_data), columns.size));
	column->set_sort_column(columns.size);
	column->set_resizable(true);
}

SimpleFileList::~SimpleFileList()
{
}

void SimpleFileList::on_filter_toggled(const Glib::ustring& path)
{
	Gtk::TreeRow row = *(model->get_iter(path));

	row[columns.filter] = !row[columns.filter];
}

void SimpleFileList::format_data(Gtk::CellRenderer* cell,
	const Gtk::TreeIter& iter,
	const Gtk::TreeModelColumn<libtorrent::size_type>& column)
{
	Gtk::TreeRow row = *iter;
	Gtk::CellRendererText* cell_text = dynamic_cast<Gtk::CellRendererText*>(cell);
	cell_text->property_text() = Linkage::suffix_value(row[column]);
}

std::vector<bool> SimpleFileList::get_filter()
{
	Gtk::TreeNodeChildren children = model->children();
	std::vector<bool> filter(children.size(), false);
	for (Gtk::TreeIter iter = children.begin(); iter != children.end(); ++iter)
	{
		Gtk::TreeRow row = *iter;
		filter[row[columns.index]] = row[columns.filter];
	}
	return filter;
}

void SimpleFileList::populate(const boost::intrusive_ptr<libtorrent::torrent_info>& info)
{
	model->clear();

	for (int i = 0; i < info->num_files(); i++)
	{
		libtorrent::file_entry file = info->file_at(i);
		Gtk::TreeRow row = *(model->append());
		row[columns.name] = file.path.string();
		row[columns.size] = file.size;
		row[columns.index] = i;
	}
}

void SimpleFileList::clear()
{
	model->clear();
}


