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

#include <iostream>
#include <list>

#include <gtkmm/main.h>

#include "linkage/Engine.hh"
#include "UI.hh"

/* FIXME: use add_gtk_option_group */
bool parse_args(int argc, char* argv[], std::list<Glib::ustring>& data);
void send_args(const std::list<Glib::ustring>& args);

int main(int argc, char *argv[])
{
	Gtk::Main kit(&argc, &argv);

	std::list<Glib::ustring> files;
	bool args = parse_args(argc, argv, files);

	if (!Engine::is_primary())
	{
		if (args)
		{
			send_args(files);
			Engine::get_dbus_manager()->send("ToggleVisible");
			std::cout << argc - 1 << " file(s) passed to running instance.\n";
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
		UI* ui = new UI();
		ui->show();
		if (args)
			send_args(files);

		kit.run();
		delete ui;
	}

	return 0;
}

bool parse_args(int argc, char* argv[], std::list<Glib::ustring>& data)
{
	/* FIXME: Add support for command line options */
  for (int i = 1; i < argc; i++)
	{
		Glib::ustring file = argv[i];
		if (file.substr(0, 1) == "-" || file.substr(0, 2) == "--")
		{
			throw Glib::OptionError(Glib::OptionError::UNKNOWN_OPTION,
															"Unknown flag: \n"
															"Usage:\n	linkage [TORRENTS...]");
		}
		/* Check for relative paths */
		if (file.substr(0, 1) != "/")
			file.insert(0,g_getenv("PWD") +	Glib::ustring("/"));

		data.push_back(file);
	}
	
	return (argc > 1);
}

void send_args(const std::list<Glib::ustring>& args)
{
	for (std::list<Glib::ustring>::const_iterator iter = args.begin();
					iter != args.end(); ++iter)
	{
		/* Pass file(s) to running instance */
		Engine::get_dbus_manager()->send("Open", *iter);
	}
}
