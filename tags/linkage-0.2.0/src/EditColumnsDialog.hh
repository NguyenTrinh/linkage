/*
Copyright (C) 2008	Dave Moore

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

#ifndef EDITCOLUMNSDIALOG_HH
#define EDITCOLUMNSDIALOG_HH

#include <libglademm/xml.h>
#include <gtkmm/dialog.h>
#include <gtkmm/cellrenderertoggle.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>


class EditColumnsDialog : public Gtk::Dialog
{
	class ColumnsModelColumns : public Gtk::TreeModelColumnRecord
	{
	public:
		ColumnsModelColumns()
		{
			add(id);
			add(visible);
			add(title);
			add(desc);
		};
		Gtk::TreeModelColumn<int> id;
		Gtk::TreeModelColumn<bool> visible;
		Gtk::TreeModelColumn<Glib::ustring> title;
		Gtk::TreeModelColumn<Glib::ustring> desc;
	};
	
	ColumnsModelColumns m_columns;
	Glib::RefPtr<Gtk::ListStore> m_model;

	Glib::RefPtr<Gnome::Glade::Xml> glade_xml;
	Gtk::TreeView* m_column_list;
	
	sigc::signal<void, int> m_signal_visible_toggled;

	void on_visible_toggled(const Glib::ustring& path);
	
public:
	struct ColumnData
	{
		Glib::ustring title;
		std::string name;
		Glib::ustring desc;
		bool visible;
		
		ColumnData(Glib::ustring t, std::string n, Glib::ustring d, bool v)
			: name(n), title(t), desc(d), visible(v) {}
		
	};
	
	EditColumnsDialog(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& xml);
	~EditColumnsDialog() {};
	
	//void populate_column_list(std::list<TorrentList::ColumnData>& cols);
	sigc::signal<void, int> signal_visible_toggled();
	
	int run(const std::vector<EditColumnsDialog::ColumnData>& cols);
};

typedef std::vector<EditColumnsDialog::ColumnData>  ColumnDataList;

#endif	/* EDITCOLUMNSDIALOG_HH */
