/*
Copyright (C) 2007   Christian Lundgren

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

#ifndef ADD_DIALOG_HH
#define ADD_DIALOG_HH

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/filechooserbutton.h>
#include <gtkmm/label.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/expander.h>
#include <gtkmm/button.h>

#include <libglademm.h>

#include "SimpleFileList.hh"

class Group;

class AddDialog : public Gtk::Dialog
{
	//Hack to let us use comboboxtext with glade
	class ComboBoxTextGlade : public Gtk::ComboBoxText
	{
	public:
		ComboBoxTextGlade(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& xml);
		~ComboBoxTextGlade();
	};

	SimpleFileList* file_list;
	Gtk::FileChooserButton *button_file, *button_path;
	Gtk::Label *label_size, *label_free;
	Gtk::CheckButton *check_seed;
	Gtk::Entry* entry_name;
	ComboBoxTextGlade* combo_group;
	Gtk::Expander* expander;
	Gtk::Button* button_ok;

	libtorrent::torrent_info m_info;

	void on_show();

	void on_file_changed();
	void on_path_changed();

	void reset_info();

	friend class UI;
	void on_groups_changed(const std::list<Group>& groups);
	
public:
	typedef struct
	{
		Glib::ustring name, group;
		std::string file, path;
		bool seed;
		std::vector<bool> filter;
	} AddData;

	AddData get_data();
	const libtorrent::torrent_info& get_info();

	int run_with_file(const std::string& file);
	int run();

	AddDialog(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& xml);
	~AddDialog();
};

#endif /* ADD_DIALOG_HH */
