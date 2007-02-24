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

#include "linkage/Engine.hh"
#include "PluginNotification.hh"


int notify_send(const char *summary, const char *body)
{
	NotifyNotification *notify;
	static const gchar *type = NULL;
	static gchar *icon_str = NULL;
	static glong expire_timeout = NOTIFY_EXPIRES_DEFAULT;
	static NotifyUrgency urgency = NOTIFY_URGENCY_NORMAL;

	if (!notify_init("linkage"))
		return 0;

	notify = notify_notification_new(summary, body, icon_str, NULL);
	notify_notification_set_category(notify, type);
	notify_notification_set_urgency(notify, urgency);
	notify_notification_set_timeout(notify, expire_timeout);

	notify_notification_show(notify, NULL);

	g_object_unref(G_OBJECT(notify));

	notify_uninit();

	return 1;
}

NotifyPlugin::NotifyPlugin()
{
}

NotifyPlugin::~NotifyPlugin()
{
}

Glib::ustring NotifyPlugin::get_name()
{
	return "NotifyPlugin";
}

Glib::ustring NotifyPlugin::get_description()
{
	return "Displays notifications trough libnotify";
}

void NotifyPlugin::on_load()
{
	Engine::instance()->get_session_manager()->signal_invalid_bencoding().connect(sigc::mem_fun(this, &NotifyPlugin::on_invalid_bencoding));
	Engine::instance()->get_session_manager()->signal_missing_file().connect(sigc::mem_fun(this, &NotifyPlugin::on_missing_file));
	Engine::instance()->get_session_manager()->signal_duplicate_torrent().connect(sigc::mem_fun(this, &NotifyPlugin::on_duplicate_torrent));

	Engine::instance()->get_alert_manager()->signal_listen_failed().connect(sigc::mem_fun(this, &NotifyPlugin::on_listen_failed));
	Engine::instance()->get_alert_manager()->signal_torrent_finished().connect(sigc::mem_fun(this, &NotifyPlugin::on_torrent_finished));
	Engine::instance()->get_alert_manager()->signal_file_error().connect(sigc::mem_fun(this, &NotifyPlugin::on_file_error));
	Engine::instance()->get_alert_manager()->signal_fastresume_rejected().connect(sigc::mem_fun(this, &NotifyPlugin::on_fastresume_rejected));
}

bool NotifyPlugin::notify(const Glib::ustring& title,
													const Glib::ustring& message,
													NotifyType type)
{
	//FIXME: add NotifyType support
	return notify_send(title.c_str(), message.c_str());
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
	WeakPtr<Torrent> torrent = Engine::instance()->get_torrent_manager()->get_torrent(hash);
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
