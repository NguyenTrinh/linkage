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

#include <libnotify/notify.h>
#include <glib.h>

#include "linkage/Plugin.hh"
#include "linkage/Torrent.hh"

class NotifyPlugin : public Linkage::Plugin
{
	NotifyNotification* build_notification(const Glib::ustring& title,
		const Glib::ustring& message,
		NotifyUrgency urgency,
		const Glib::ustring& category);

	void notify(const Glib::ustring& title,
		const Glib::ustring& message,
		NotifyUrgency urgency,
		const Glib::ustring& category = Glib::ustring());

	void notify_with_action(const Glib::ustring& title,
		const Glib::ustring& message,
		NotifyUrgency urgency,
		const Glib::ustring& action,
		const Glib::ustring& action_title,
		const sigc::slot<void>& slot,
		const Glib::ustring& category = Glib::ustring());

	static void on_action_cb(NotifyNotification* notification, gchar* action, sigc::slot<void>* slot);
	static void free_slot(sigc::slot<void>* slot);

	void on_open_location(const Glib::ustring& path);
	void on_stop_torrent(const Linkage::WeakTorrentPtr& weak);

	void on_duplicate_torrent(const Glib::ustring& msg, const libtorrent::sha1_hash& hash);

	void on_listen_failed(const Glib::ustring& msg);
	void on_torrent_finished(const Linkage::TorrentPtr& torrent);
	void on_file_error(const Linkage::TorrentPtr& torrent, const Glib::ustring& msg);
	void on_fastresume_rejected(const Linkage::TorrentPtr& torrent, const Glib::ustring& msg);

public:
	Linkage::Plugin::Info get_info();

	NotifyPlugin();
	~NotifyPlugin();
};

#endif /* PLUGIN_NOTIFICATION_HH */
