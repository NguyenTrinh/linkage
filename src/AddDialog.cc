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

#include <sys/vfs.h>

#include <gtkmm/main.h>
#include <glibmm/i18n.h>

#include "linkage/Engine.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/Utils.hh"

#include "GroupsWin.hh"
#include "Group.hh"
#include "AddDialog.hh"

using namespace Linkage;

AddDialog::AddDialog(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& xml)
	: Gtk::Dialog(cobject)
{
	xml->get_widget("add_filechooserbutton_file", button_file);
	xml->get_widget("add_filechooserbutton_path", button_path);
	xml->get_widget("add_label_size", label_size);
	xml->get_widget("add_label_free", label_free);
	xml->get_widget("add_entry_name", entry_name);
	xml->get_widget("add_checkbutton_seed", check_seed);
	xml->get_widget("add_expander", expander);
	xml->get_widget("add_okbutton", button_ok);
	xml->get_widget_derived("add_treeview", file_list);
	xml->get_widget_derived("add_combobox_group", combo_group);

	/* Connect update signal */
	GroupsWin* groups_win;
	xml->get_widget_derived("groups_win", groups_win);
	groups_win->signal_groups_changed().connect(sigc::mem_fun(this, &AddDialog::on_groups_changed));

	Gtk::FileFilter torrent_filter;
	torrent_filter.add_mime_type("application/x-bittorrent");
	torrent_filter.set_name(_("BitTorrent files"));

	Gtk::FileFilter no_filter;
	no_filter.add_pattern("*");
	no_filter.set_name(_("All files"));

	button_file->add_filter(torrent_filter);
	button_file->add_filter(no_filter);

	button_file->signal_selection_changed().connect(sigc::mem_fun(this, &AddDialog::on_file_changed));
	button_path->signal_selection_changed().connect(sigc::mem_fun(this, &AddDialog::on_path_changed));

	expander->set_expanded(Engine::get_settings_manager()->get_bool("ui/advanced_expanded"));
}

AddDialog::~AddDialog()
{
	Engine::get_settings_manager()->set("ui/advanced_expanded", expander->get_expanded());


	delete combo_group;
	delete file_list;
}

void AddDialog::on_show()
{
	if (Engine::get_settings_manager()->get_bool("files/use_default_path"))
	{
		Glib::ustring path = Engine::get_settings_manager()->get_string("files/default_path");
		button_path->set_filename(path);
	}

	Gtk::Dialog::on_show();
}

void AddDialog::on_file_changed()
{
	std::string file = button_file->get_filename();
	libtorrent::entry entry;
	if (load_entry(file, entry))
	{
		m_info = boost::intrusive_ptr<libtorrent::torrent_info>(new libtorrent::torrent_info(entry));
		entry_name->set_text(m_info->name());
		file_list->populate(m_info);
		label_size->set_markup(String::ucompose(
			_("<i>%1 free disk space required</i>"),
			suffix_value(m_info->total_size())));
		button_ok->set_sensitive(true);
		//update label_free if save path was set before torrent file
		on_path_changed();
	}
	else
	{
		Glib::ustring color = Engine::get_settings_manager()->get_string("ui/colors/error");
		label_size->set_markup(String::ucompose(
			"<span color='%1'><i>%2</i></span>",
			color, 
			_("Invalid torrent")));
		reset_info();

		button_ok->set_sensitive(false);
	}
}

void AddDialog::on_path_changed()
{
	std::string path = button_path->get_filename();
	struct statfs sfs;
	
	statfs(path.c_str(), &sfs);
	libtorrent::size_type free = sfs.f_bavail * sfs.f_bsize;

	Glib::ustring markup = String::ucompose(
		_("<i>%1 free disk space available</i>"),
		suffix_value(free));

	if (m_info && m_info->is_valid() && free < m_info->total_size())
	{
		Glib::ustring color = Engine::get_settings_manager()->get_string("ui/colors/error");
		markup = "<span color='" + color + "'>" + markup + "</span>";
	}
	label_free->set_markup(markup);
}

void AddDialog::reset_info()
{
	entry_name->set_text("");
	file_list->clear();
	label_size->set_markup("");
	label_free->set_markup("");
	combo_group->set_active(-1);
}

void AddDialog::on_groups_changed(const std::list<GroupPtr>& groups)
{
	combo_group->clear();
	for (std::list<GroupPtr>::const_iterator i = groups.begin(); i != groups.end(); ++i)
	{
		GroupPtr group = *i;
		combo_group->append_text(group->get_name());
	}
}

AddDialog::AddData AddDialog::get_data()
{
	AddData data;
	data.file = Glib::filename_to_utf8(button_file->get_filename());
	data.path = button_path->get_filename();
	data.name = entry_name->get_text();
	data.group = combo_group->get_active_text();
	data.seed = check_seed->get_active();
	data.filter = file_list->get_filter();

	reset_info();

	button_file->unselect_all();
	button_path->unselect_all();

	return data;
}

Torrent::InfoPtr AddDialog::get_info()
{
	Torrent::InfoPtr info = m_info;
	m_info = Torrent::InfoPtr(NULL);

	return info;
}

int AddDialog::run_with_file(const Glib::ustring& file)
{
	//hack so users can't press ok before the filename is set properly
	button_ok->set_sensitive(false);

	//FIXME: uncomment when gtkfilechooser is fixed
	//button_file->set_filename(Glib::filename_from_utf8(file));

	// horrible hack to make sure file is selected
	button_file->set_filename(Glib::filename_from_utf8(file));
	while (Glib::file_test(file, Glib::FILE_TEST_EXISTS))
	{
		while (Gtk::Main::events_pending())
			Gtk::Main::iteration(false);

		if (!button_file->get_filename().empty())
			break;
		else
			button_file->set_filename(Glib::filename_from_utf8(file));
	}

	return run();
}

int AddDialog::run()
{
	if (button_file->get_filename().empty())
		button_ok->set_sensitive(false);

	int response = Gtk::Dialog::run();

	//only reset info on cancel, otherwise do it in get_data
	if (response == Gtk::RESPONSE_CANCEL)
	{
		reset_info();

		button_file->unselect_all();
		button_path->unselect_all();
	}

	return response;
}

