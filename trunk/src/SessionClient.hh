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

#ifndef SESSION_CLIENT_HH
#define SESSION_CLIENT_HH

#include <sigc++/signal.h>

#include <glibmm/ustring.h>

#if HAVE_GNOME
#include <libgnomeuimm/client.h>
#endif

#if HAVE_EXO
#include <exo/exo.h>
#endif

class SessionClient : public sigc::trackable
{
	#if HAVE_GNOME
	void on_die_gnome();
	bool on_save_yourself_gnome(int phase, Gnome::UI::SaveStyle save_style,
		bool shutdown, Gnome::UI::InteractStyle interact_style, bool fast);
	#endif

	#if HAVE_EXO
	ExoXsessionClient* exo_client;
	static void on_save_yourself_exo(ExoXsessionClient *client, gpointer data);
	#endif

	sigc::signal<void> m_signal_quit;

public:
	//this isn't really session related but whatever
	void open_location(const Glib::ustring& path);

	sigc::signal<void> signal_quit();

	SessionClient();
	~SessionClient();
};

#endif /* SESSION_CLIENT_HH */
