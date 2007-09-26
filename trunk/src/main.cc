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

	bool version, quit, daemon;
	std::vector<Glib::ustring> files;
};

Options::Options()
: Glib::OptionGroup(PACKAGE_NAME, "Command line options"),
	version(false),
	quit(false),
	daemon(false)
{
	Glib::OptionEntry e_daemon;
	e_daemon.set_long_name("daemon") ;
	e_daemon.set_description(_("Run without user interface"));
	add_entry(e_daemon, daemon);

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

static void send_files(const std::vector<Glib::ustring>& files)
{
	// FIXME: handle file:// style URIs
	for (unsigned int i = 0; i < files.size(); i++)
	{
		Glib::ustring file = files[i];
		if (!Glib::path_is_absolute(file))
			file = Glib::build_filename(Glib::get_current_dir(), file);
		/* Pass file(s) to running instance */
		if (Engine::is_primary())
			Engine::get_interface().open(file);
		else
			Engine::get_dbus_manager()->send("org.linkage.Interface", "Open", "/org/linkage/Interface", file);
	}
}

static void attach_interface()
{
	if (!Engine::is_daemon())
	{
		g_warning("Ignoring attempt to attach new interface to non-daemonized instance");
		return;
	}

	Glib::RefPtr<Gnome::Glade::Xml> xml;
	try
	{
		xml = Gnome::Glade::Xml::create(DATA_DIR "/linkage.glade");
	}
	catch (const Gnome::Glade::XmlError& ex)
	{
		g_error(ex.what().c_str());
	}

	UI* ui = NULL;
	xml->get_widget_derived("main_window", ui);
	ui->show();

	// just to wake it up
	Engine::get_plugin_manager();
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
		Engine::get_dbus_manager()->send("org.linkage.Interface", "Quit", "/org/linkage/Interface");
		return 0;
	}

	if (!Engine::is_primary())
	{
		bool attach = Engine::get_dbus_manager()->is_daemon_remote();
		if (attach)
		{
			std::cout << _("Attaching interface to daemonized instance") << std::endl;
			Engine::get_dbus_manager()->send("org.linkage.Engine", "LoadInterface", "/org/linkage/Engine");
		}

		if (!options.files.empty())
		{
			send_files(options.files);
			std::cout << options.files.size() << _(" files passed to running instance.")  << std::endl;
			return 0;
		}
		else
		{
			if (!attach)
			{
				std::cerr << _("Another process is already running. Quitting...")  << std::endl;
				return 1;
			}
		}
	}
	else
	{
		//FIXME: resume_session should be async
		Engine::get_session_manager()->resume_session();

		if (!options.daemon)
		{
			attach_interface();

			if (!options.files.empty())
				send_files(options.files);
		}
		else
		{
			Engine::get_dbus_manager()->signal_load_interface().connect(sigc::ptr_fun(&attach_interface));

			if (!options.files.empty())
				g_warning("Ignoring (%i) file(s) passed to daemon", options.files.size());
		}

		#if HAVE_GNOME
		Gtk::Main::run();
		#else
		kit.run();
		#endif

		Engine::uninit();
	}

	return 0;
}

