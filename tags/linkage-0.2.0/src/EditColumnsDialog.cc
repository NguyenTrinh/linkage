/*
Copyright (C) 2008  Dave Moore

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

#include "EditColumnsDialog.hh"

EditColumnsDialog::EditColumnsDialog(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& xml)
	: Gtk::Dialog(cobject), glade_xml(xml)
{
	glade_xml->get_widget("column_list", m_column_list);
	
	m_model = Gtk::ListStore::create(m_columns);
	m_column_list->set_model(m_model);
	
	//Gtk::CellRendererToggle* renderer = new Gtk::CellRendererToggle();
	//renderer->signal_toggled().connect(sigc::mem_fun(*this,
        //&EditColumnsDialog::on_visible_toggled));
	
	
	int col_id = m_column_list->append_column_editable("", m_columns.visible);
	Gtk::CellRendererToggle* renderer = dynamic_cast<Gtk::CellRendererToggle*>
		(m_column_list->get_column(col_id - 1)->get_first_cell_renderer());
	renderer->signal_toggled().connect(sigc::mem_fun(this, &EditColumnsDialog::on_visible_toggled));
	
	m_column_list->append_column("Column", m_columns.title);
	m_column_list->append_column("Description", m_columns.desc);
}

int EditColumnsDialog::run(const ColumnDataList& cols)
{
	for (int i = 0; i < cols.size(); i++)
	{
		Gtk::TreeModel::Row row = *(m_model->append());
		row[m_columns.visible] = cols[i].visible;
		row[m_columns.title] = cols[i].title;
		row[m_columns.desc] = cols[i].desc;
		row[m_columns.id] = i;
	}
	
	int res = Gtk::Dialog::run();
	hide();
	m_model->clear();
	return res;
}

void EditColumnsDialog::on_visible_toggled(const Glib::ustring& path)
{
	Gtk::TreeRow row = *(m_model->get_iter(path));
	m_signal_visible_toggled.emit(row[m_columns.id]);
}

sigc::signal<void, int> EditColumnsDialog::signal_visible_toggled()
{
	return m_signal_visible_toggled;
}

//


