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

#include <dbus-c++/dbus.h>
#include <dbus-c++/glib-integration.h>

#include "UI.hh"

#include "linkage/Engine.hh"
#include "linkage/PluginManager.hh"
#include "linkage/Utils.hh"

#include "linkage/DBusProxyGlue.hh"

using namespace Linkage;

class Proxy
: public org::linkage::InterfaceProxy,
  public DBus::IntrospectableProxy,
  public DBus::ObjectProxy
{
public:
	Proxy() : DBus::ObjectProxy(Engine::get_bus(), "/org/linkage/interface", "org.linkage") {}
};

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

struct send_file : public std::unary_function<void, Glib::ustring>
{
	void operator()(Glib::ustring file)
	{
		// FIXME: handle file:// style URIs
		if (!Glib::path_is_absolute(file))
			file = Glib::build_filename(Glib::get_current_dir(), file);

		if (!Engine::is_primary())
		{
			Proxy remote;
			remote.Open(file);
		}
		else
			Engine::get_interface().open(file);
	}
};

DBus::Glib::BusDispatcher dispatcher;

int main(int argc, char *argv[])
{
	/* gettext */
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	/* enable support for native paths */
	boost::filesystem::path::default_name_check(boost::filesystem::native);

	/* init and parse argvs */
	#if !HAVE_GNOME
	Gtk::Main kit(&argc, &argv);
	#endif

	/* connect to dbus */
	DBus::default_dispatcher = &dispatcher;
	dispatcher.attach(NULL);
	DBus::Connection connection = DBus::Connection::SessionBus();

	/* init backend, hooked up to our dbus connection */
	Engine::init(connection);

	Options options;
	//work around for gnomemm bug
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
		Proxy remote;
		remote.Quit();
		return 0;
	}

	/* check if another instance is running */
	if (!Engine::is_primary())
	{
		if (!options.files.empty())
			std::for_each(options.files.begin(), options.files.end(), send_file());

		/* for startup notification, since we don't show any windows */
		gdk_notify_startup_complete();

		return 0;
	}

	/* no other instance, fire up ui */

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

	if (!options.files.empty())
		std::for_each(options.files.begin(), options.files.end(), send_file());

	#if HAVE_GNOME
	Gtk::Main::run();
	#else
	kit.run();
	#endif

	ui->hide();
	delete ui;

	Engine::uninit();

	return 0;
}

