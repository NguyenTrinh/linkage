/*
Copyright (C) 2006-2007   Christian Lundgren
Copyright (C) 2007        Dave Moore

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

#if HAVE_GNOME
#include <libgnomevfsmm/utils.h>
#include <libgnomevfsmm/uri.h>
#endif

#include <glibmm/spawn.h>

#include <glibmm/miscutils.h>
#include <glibmm/i18n.h>

#include "SessionClient.hh"

SessionClient::SessionClient()
{
	#if HAVE_GNOME
	Gnome::UI::Client* gnome_client = Gnome::UI::Client::master_client();
	if (gnome_client)
	{
		gnome_client->signal_die().connect(sigc::mem_fun(this, &SessionClient::on_die_gnome));
		gnome_client->signal_save_yourself().connect(sigc::mem_fun(this, &SessionClient::on_save_yourself_gnome));
	}
	else
		g_warning(_("Failed to connect to GNOME session"));
	#endif

	#if HAVE_EXO
	GdkDisplay* display = get_screen()->get_display()->gobj();
	GdkWindow* leader = gdk_display_get_default_group(display);
	exo_client = exo_xsession_client_new_with_group(leader);
	g_signal_connect(G_OBJECT(exo_client), "save-yourself", 
		G_CALLBACK(SessionClient::on_save_yourself_exo), this);
	#endif
}

SessionClient::~SessionClient()
{
	#if HAVE_EXO
	g_object_unref(G_OBJECT(exo_client));
	#endif
}

sigc::signal<void> SessionClient::signal_quit()
{
	return m_signal_quit;
}

#if HAVE_GNOME
void SessionClient::on_die_gnome()
{
	m_signal_quit.emit();
}

bool SessionClient::on_save_yourself_gnome(int phase, Gnome::UI::SaveStyle save_style,
	bool shutdown, Gnome::UI::InteractStyle interact_style, bool fast)
{
	Gnome::UI::Client* gnome_client = Gnome::UI::Client::master_client();
	if (gnome_client)
	{
		std::vector<std::string> argv;
		argv.push_back("linkage");
		gnome_client->set_clone_command(argv);
		gnome_client->set_restart_command(argv);
	}
	else
		g_warning(_("Failed to connect to GNOME session"));

	m_signal_quit.emit();

	return true;
}
#endif

#if HAVE_EXO
void SessionClient::on_save_yourself_exo(ExoXsessionClient* client, SessionClient* self)
{
	SessionClient* ui = static_cast<SessionClient*>(data);

	gchar* argv = "linkage";
	gint argc = 1;
	exo_xsession_client_set_restart_command(ui->exo_client, &argv, argc);

	self->m_signal_quit.emit();
}
#endif

void SessionClient::open_location(const Glib::ustring& path)
{
	// FIXME: show message dialog instead of g_warning
	#if HAVE_GNOME
	Glib::ustring uri = Gnome::Vfs::Uri::make_from_input(path);
	try
	{
		Gnome::Vfs::url_show(uri);
	}
	catch (Gnome::Vfs::exception& ex)
	{
		g_warning(ex.what().c_str());
	}
	#elif HAVE_EXO
	GError* e = NULL;
	if (!exo_url_show(path.c_str(), NULL, &e))
	{
		g_warning(e->message);
	}
	#else
	Glib::ustring app = Glib::find_program_in_path("nautilus");
	if (app.empty())
		app = Glib::find_program_in_path("thunar");
	if (!app.empty())
	{
		Glib::ustring cmd = app + " \"" + path + "\"";
		Glib::spawn_command_line_async(cmd);
	}
	else
		g_warning(_("No suitable file manager found"));
	#endif
}

