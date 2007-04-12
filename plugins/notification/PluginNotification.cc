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

#include <gtkmm/statusicon.h>

#include "linkage/Engine.hh"
#include "PluginNotification.hh"

NotifyPlugin::NotifyPlugin() : 
	Plugin("NotifyPlugin",
					"Displays notifications trough libnotify",
					"1",
					"Christian Lundgren",
					"http://code.google.com/p/linkage")
{
}

NotifyPlugin::~NotifyPlugin()
{
	notify_uninit();
}

void NotifyPlugin::on_load()
{
	notify_init("linkage");

	Engine::get_session_manager()->signal_invalid_bencoding().connect(sigc::mem_fun(this, &NotifyPlugin::on_invalid_bencoding));
	Engine::get_session_manager()->signal_missing_file().connect(sigc::mem_fun(this, &NotifyPlugin::on_missing_file));
	Engine::get_session_manager()->signal_duplicate_torrent().connect(sigc::mem_fun(this, &NotifyPlugin::on_duplicate_torrent));

	Engine::get_alert_manager()->signal_listen_failed().connect(sigc::mem_fun(this, &NotifyPlugin::on_listen_failed));
	Engine::get_alert_manager()->signal_torrent_finished().connect(sigc::mem_fun(this, &NotifyPlugin::on_torrent_finished));
	Engine::get_alert_manager()->signal_file_error().connect(sigc::mem_fun(this, &NotifyPlugin::on_file_error));
	Engine::get_alert_manager()->signal_fastresume_rejected().connect(sigc::mem_fun(this, &NotifyPlugin::on_fastresume_rejected));
}

void NotifyPlugin::notify(const Glib::ustring& title,
													const Glib::ustring& message,
													NotifyType type)
{
	NotifyUrgency urgency;
	gchar* icon = NULL;
	switch (type)
	{
		case NOTIFY_ERROR:
			icon = "dialog-error";
			urgency = NOTIFY_URGENCY_CRITICAL;
			break;
		case NOTIFY_WARNING:
			icon = "dialog-warning";
			urgency = NOTIFY_URGENCY_NORMAL;
			break;
		case NOTIFY_INFO:
		default:
			icon = "dialog-information";
			urgency = NOTIFY_URGENCY_LOW;
			break;
	}
	
	glong timeout = NOTIFY_EXPIRES_DEFAULT;
	NotifyNotification* notify;
	WeakPtr<Plugin> plugin = Engine::get_plugin_manager()->get_plugin("TrayPlugin");
	if (plugin)
	{
		GtkStatusIcon* status_icon = static_cast<GtkStatusIcon*>(plugin->get_user_data());
		notify = notify_notification_new_with_status_icon(title.c_str(), message.c_str(), icon, status_icon);
	}
	else
		notify = notify_notification_new(title.c_str(), message.c_str(), icon, NULL);
	notify_notification_set_urgency(notify, urgency);
	notify_notification_set_timeout(notify, timeout);

	notify_notification_show(notify, NULL);

	g_object_unref(G_OBJECT(notify));
}

Plugin * CreatePlugin()
{
	 return new NotifyPlugin();
}

void NotifyPlugin::on_invalid_bencoding(const Glib::ustring& msg, const Glib::ustring& file)
{
	notify("Invalid bencoding", msg, NOTIFY_ERROR);
}

void NotifyPlugin::on_missing_file(const Glib::ustring& msg, const Glib::ustring& file)
{
	notify("Missing file", msg, NOTIFY_ERROR);
}

void NotifyPlugin::on_duplicate_torrent(const Glib::ustring& msg, const sha1_hash& hash)
{
	notify("Duplicate torrent", msg, NOTIFY_ERROR);
}

void NotifyPlugin::on_listen_failed(const Glib::ustring& msg)
{
	notify("Listen failed", msg, NOTIFY_ERROR);
}

void NotifyPlugin::on_torrent_finished(const sha1_hash& hash, const Glib::ustring& msg)
{
	WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
	notify("Torrent finished", torrent->get_name() + " is complete", NOTIFY_INFO);
}

void NotifyPlugin::on_file_error(const sha1_hash& hash, const Glib::ustring& msg)
{
	notify("File error", msg, NOTIFY_ERROR);
}

void NotifyPlugin::on_fastresume_rejected(const sha1_hash& hash, const Glib::ustring& msg)
{
	notify("Fastresume failed", msg, NOTIFY_WARNING);
}
