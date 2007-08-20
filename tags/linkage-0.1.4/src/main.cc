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
#include <glibmm/i18n.h>
#include <libglademm.h>

#if HAVE_GNOME
#include <libgnomemm/main.h>
#include <libgnomemm/init.h>
#include <libgnomeuimm/init.h>
#include <libgnomevfsmm/init.h>

#include <libgnomeui/gnome-ui-init.h>
#include <libgnome/gnome-init.h>
#endif

#include "UI.hh"

#include "linkage/Engine.hh"
#include "linkage/DbusManager.hh"
#include "linkage/PluginManager.hh"
#include "linkage/SessionManager.hh"
#include "linkage/Utils.hh"

class Options : public Glib::OptionGroup
{
public:
	Options();
	~Options();

	bool version, quit;
	std::vector<Glib::ustring> files;
};

Options::Options()
: Glib::OptionGroup(PACKAGE_NAME, "Command line options"),
	version(false),
	quit(false)
{
	Glib::OptionEntry e_version;
	e_version.set_long_name("version") ;
	e_version.set_description(_("Show version and quit"));
	add_entry(e_version, version);

	Glib::OptionEntry e_quit;
	e_quit.set_long_name("quit");
	e_quit.set_description(_("Tell the running instance to quit"));
	add_entry(e_quit, quit);

	Glib::OptionEntry e_remaining;
	e_remaining.set_long_name(G_OPTION_REMAINING);
	add_entry(e_remaining, files);
}

Options::~Options()
{
}

static void send_files(const std::vector<Glib::ustring>& files, UI* recipient = NULL)
{
	// FIXME: handle file:// style URIs
	for (unsigned int i = 0; i < files.size(); i++)
	{
		Glib::ustring file = files[i];
		if (!Glib::path_is_absolute(file))
			file = Glib::build_filename(Glib::get_current_dir(), file);
		/* Pass file(s) to running instance */
		if (recipient)
			recipient->open(file);
		else
			Engine::get_dbus_manager()->send("Open", file);
	}
}

int main(int argc, char *argv[])
{
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	#if !HAVE_GNOME
	Gtk::Main kit(&argc, &argv);
	#endif

	boost::filesystem::path::default_name_check(boost::filesystem::native);

	if(!Glib::thread_supported()) 
		Glib::thread_init();

	Options options; 
	Glib::OptionContext* context = new Glib::OptionContext(
		_("[FILE...] \n\nA BitTorrent client\n"));
	context->set_main_group(options);
	
	#if HAVE_GNOME
	Gnome::Vfs::init();

	Gnome::Main app(PACKAGE_NAME, PACKAGE_VERSION,
		Gnome::UI::module_info_get(), argc, argv, *context);
	#else
	try
	{
		context->parse(argc, argv);
	}
	catch (const Glib::OptionError& e)
	{
		std::cerr << e.what() << std::endl;
		std::cout << _("Run \"linkage --help\" to see a full list of available command line options.\n");
		return 1;
	}
	delete context;
	#endif

	if (options.version)
	{
		std::cout << PACKAGE_VERSION << std::endl;
		return 0;
	}
	if (options.quit)
	{
		Engine::get_dbus_manager()->send("Quit");
		return 0;
	}

	if (!Engine::is_primary())
	{
		if (!options.files.empty())
		{
			send_files(options.files);
			std::cout << options.files.size() << _(" files passed to running instance.\n");
			return 0;
		}
		else
		{
			std::cerr << _("Another process is already running. Quitting...\n");
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

		// just to wake it up
		Engine::get_plugin_manager();

		ui->show();
		if (!options.files.empty())
			send_files(options.files, ui);

		#if HAVE_GNOME
		Gtk::Main::run();
		#else
		kit.run();
		#endif
		delete ui;
	}

	return 0;
}

