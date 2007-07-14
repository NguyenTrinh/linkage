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

/*g++ -shared notifications.cpp -o notify_plugin.so `pkg-config gtkmm-2.4 --libs --cflags` `pkg-config libtorrent --libs --cflags` `pkg-config libnotify --libs --cflags`*/

#ifndef PLUGIN_NOTIFICATION_HH
#define PLUGIN_NOTIFICATION_HH

#include "linkage/Plugin.hh"

#include <libnotify/notify.h>
#include <glib.h>

class NotifyPlugin : public Plugin
{
	enum NotifyType { NOTIFY_INFO, NOTIFY_WARNING, NOTIFY_ERROR };

	void notify(const Glib::ustring& title,
							const Glib::ustring& message,
							NotifyType type);

	void on_invalid_bencoding(const Glib::ustring& msg, const Glib::ustring& file);
	void on_missing_file(const Glib::ustring& msg, const Glib::ustring& file);
	void on_duplicate_torrent(const Glib::ustring& msg, const libtorrent::sha1_hash& hash);

	void on_listen_failed(const Glib::ustring& msg);
	void on_torrent_finished(const libtorrent::sha1_hash& hash, const Glib::ustring& msg);
	void on_file_error(const libtorrent::sha1_hash& hash, const Glib::ustring& msg);
	void on_fastresume_rejected(const libtorrent::sha1_hash& hash, const Glib::ustring& msg);

public:
	PluginParent get_parent() { return Plugin::PARENT_NONE; }
	Gtk::Widget* get_widget() { return NULL; }
	Gtk::Widget* get_config_widget() { return NULL; }

	void on_load();

	NotifyPlugin();
	~NotifyPlugin();
};

#endif /* PLUGIN_NOTIFICATION_HH */
