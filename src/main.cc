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

#include "config.h"

#include <iostream>
#include <list>

#include <gtkmm/main.h>
#include <libglademm.h>

#include "linkage/Engine.hh"
#include "linkage/Utils.hh"

#include "UI.hh"

bool parse_args(int argc, char* argv[], std::list<Glib::ustring>& files);
void send_files(const std::list<Glib::ustring>& files);

int main(int argc, char *argv[])
{
	boost::filesystem::path::default_name_check(boost::filesystem::native);

	if(!Glib::thread_supported()) 
		Glib::thread_init();

	std::list<Glib::ustring> files;
	if (parse_args(argc, argv, files))
		return 0;

	/* Don't set up translations since we don't have any */
	Gtk::Main kit(&argc, &argv, false);

	bool file_args = (argc > 1);

	if (!Engine::is_primary())
	{
		if (file_args)
		{
			send_files(files);
				std::cout << argc - 1 << " files passed to running instance.\n";
			return 0;
		}
		else
		{
			std::cerr << "Another process is already running. Quitting...\n";
			return 1;
		}
	}
	else
	{
		Engine::get_session_manager()->resume_session();

		UI* ui = 0;
		Glib::RefPtr<Gnome::Glade::Xml> xml;
		try
		{
			xml = Gnome::Glade::Xml::create(DATA_DIR "/linkage.glade");
		}
		catch (const Gnome::Glade::XmlError& ex)
		{
			g_error(ex.what().c_str());
			return 1;
		}

		xml->get_widget_derived("main_window", ui);

		ui->show();
		if (file_args)
			send_files(files);

		// just to wake it up
		Engine::get_plugin_manager();

		kit.run();
		delete ui;
	}

	return 0;
}

bool parse_args(int argc, char* argv[], std::list<Glib::ustring>& files)
{
	Glib::OptionGroup options(PACKAGE_NAME, "Command line options");
	bool version = false, quit = false;
	Glib::OptionEntry e_version;
	e_version.set_long_name("version") ;
	e_version.set_description("Show version and quit");
	options.add_entry(e_version, version);
  Glib::OptionEntry e_quit;
  e_quit.set_long_name("quit");
  e_quit.set_description("Tell the running instance to quit");
  options.add_entry(e_quit, quit);
 
	Glib::OptionContext context("[FILE...] \n\nA BitTorrent client for GTK+\n");
	context.set_main_group(options);
	try
	{
		context.parse(argc, argv);
	}
	catch (const Glib::OptionError& e)
	{
		std::cerr << e.what() << std::endl;
		std::cout << "Run \"linkage --help\" to see a full list of available command line options.\n";
		return true;
	}

	if (version)
	{
		std::cout << PACKAGE_VERSION << std::endl;
		return true;
	}

  if (quit)
  {
    Engine::get_dbus_manager()->send("Quit");
    return true;
  }
 
	for (int i = 1; i < argc; i++)
	{
		Glib::ustring file = argv[i];
		if (!Glib::path_is_absolute(file))
			file.insert(0, Glib::get_current_dir() + "/");

		files.push_back(file);
	}
	
	return false;
}

void send_files(const std::list<Glib::ustring>& files)
{
	for (std::list<Glib::ustring>::const_iterator iter = files.begin();
		iter != files.end(); ++iter)
	{
		/* Pass file(s) to running instance */
		Engine::get_dbus_manager()->send("Open", *iter);
	}
}
