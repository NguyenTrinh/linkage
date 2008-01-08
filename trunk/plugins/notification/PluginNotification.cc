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
#include <glibmm/i18n.h>

#include "PluginNotification.hh"

#include "linkage/Engine.hh"
#include "linkage/SessionManager.hh"
#include "linkage/AlertManager.hh"
#include "linkage/PluginManager.hh"
#include "linkage/TorrentManager.hh"
#include "linkage/ucompose.hpp"

/* dbus-c++ defines these ... */
#ifdef HAVE_CONFIG_H 
#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef VERSION

#include "config.h"
#endif

#if HAVE_GNOME
#include <libgnomevfsmm/utils.h>
#include <libgnomevfsmm/uri.h>
#endif

#if HAVE_EXO
#include <exo/exo.h>
#endif

#define PLUGIN_NAME	"NotifyPlugin"
#define PLUGIN_DESC	_("Displays notifications trough libnotify")
#define PLUGIN_VER	PACKAGE_VERSION
#define PLUGIN_AUTHOR	"Christian Lundgren"
#define PLUGIN_WEB	"http://code.google.com/p/linkage"

using namespace Linkage;

NotifyPlugin::NotifyPlugin()	
{
	notify_init(PLUGIN_NAME);

	Engine::get_session_manager()->signal_duplicate_torrent().connect(sigc::mem_fun(this, &NotifyPlugin::on_duplicate_torrent));

	Engine::get_alert_manager()->signal_listen_failed().connect(sigc::mem_fun(this, &NotifyPlugin::on_listen_failed));
	Engine::get_alert_manager()->signal_torrent_finished().connect(sigc::mem_fun(this, &NotifyPlugin::on_torrent_finished));
	Engine::get_alert_manager()->signal_file_error().connect(sigc::mem_fun(this, &NotifyPlugin::on_file_error));
	Engine::get_alert_manager()->signal_fastresume_rejected().connect(sigc::mem_fun(this, &NotifyPlugin::on_fastresume_rejected));
}

NotifyPlugin::~NotifyPlugin()
{
	notify_uninit();
}

Plugin::Info NotifyPlugin::get_info()
{
	return Plugin::Info(PLUGIN_NAME,
		PLUGIN_DESC,
		PLUGIN_VER,
		PLUGIN_AUTHOR,
		PLUGIN_WEB,
		false,
		Plugin::PARENT_NONE);
}

NotifyNotification* NotifyPlugin::build_notification(const Glib::ustring& title,
	const Glib::ustring& message,
	NotifyUrgency urgency,
	const Glib::ustring& category)
{
	Glib::ustring icon;
	switch (urgency)
	{
		case NOTIFY_URGENCY_CRITICAL:
			icon = "dialog-error";
			break;
		case NOTIFY_URGENCY_NORMAL:
			icon = "dialog-warning";
			break;
		case NOTIFY_URGENCY_LOW:
		default:
			icon = "dialog-information";
			break;
	}

	NotifyNotification* notification = notify_notification_new(
		title.c_str(), message.c_str(), icon.c_str(), NULL);

	PluginPtr plugin = Engine::get_plugin_manager()->get_plugin("TrayPlugin");
	if (plugin)
	{
		GtkStatusIcon* status_icon = static_cast<GtkStatusIcon*>(plugin->get_user_data());
		if (status_icon)
			notify_notification_attach_to_status_icon(notification, status_icon);
	}
		
	notify_notification_set_urgency(notification, urgency);
	notify_notification_set_timeout(notification, NOTIFY_EXPIRES_DEFAULT);
	if (!category.empty())
		notify_notification_set_category(notification, category.c_str());

	return notification;
}
		
void NotifyPlugin::notify_with_action(const Glib::ustring& title,
	const Glib::ustring& message,
	NotifyUrgency urgency,
	const Glib::ustring& action,
	const Glib::ustring& action_title,
	const sigc::slot<void>& slot,
	const Glib::ustring& category)
{
	NotifyNotification* notification = build_notification(title, message, urgency, category);

	sigc::slot<void>* data = new sigc::slot<void>(slot);
	notify_notification_add_action(notification, action.c_str(),
		action_title.c_str(),
		(NotifyActionCallback)NotifyPlugin::on_action_cb,
		data,
		(GFreeFunc)NotifyPlugin::free_slot);

	notify_notification_show(notification, NULL);
}

void NotifyPlugin::notify(const Glib::ustring& title,
	const Glib::ustring& message,
	NotifyUrgency urgency,
	const Glib::ustring& category)
{
	NotifyNotification* notification = build_notification(title, message, urgency, category);

	notify_notification_show(notification, NULL);
}

void NotifyPlugin::free_slot(sigc::slot<void>* slot)
{
	delete slot;
}

void NotifyPlugin::on_action_cb(NotifyNotification* notification, gchar* action, sigc::slot<void>* slot)
{
	(*slot)();
}

void NotifyPlugin::on_open_location(const Glib::ustring& path)
{
	// duplicated in UI :(
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
	#endif

	if (!HAVE_GNOME && !HAVE_EXO)
	{
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
	}
}

void NotifyPlugin::on_stop_torrent(const WeakTorrentPtr& weak)
{
	TorrentPtr torrent = weak.lock();
	Engine::get_session_manager()->stop_torrent(torrent);
}

void NotifyPlugin::on_duplicate_torrent(const Glib::ustring& msg, const libtorrent::sha1_hash& hash)
{
	notify(_("Duplicate torrent"), msg, NOTIFY_URGENCY_NORMAL);
}

// FIXME: make sure all of these actually works ok
void NotifyPlugin::on_listen_failed(const Glib::ustring& msg)
{
	notify(_("Listen failed"), msg, NOTIFY_URGENCY_CRITICAL, "network.error");
}

void NotifyPlugin::on_torrent_finished(const Linkage::TorrentPtr& torrent)
{
	Glib::ustring msg = String::ucompose(_("%1 is complete"), torrent->get_name());

	Glib::ustring path = torrent->get_path();
	if (torrent->get_info()->num_files() > 1)
		path = Glib::build_filename(path, torrent->get_name());

	sigc::slot<void> slot = sigc::bind(sigc::mem_fun(this, &NotifyPlugin::on_open_location), path);

	notify_with_action(_("Torrent finished"), msg,	NOTIFY_URGENCY_LOW,
		"open", _("Open location"), slot, "transfer.complete");
}

void NotifyPlugin::on_file_error(const Linkage::TorrentPtr& torrent, const Glib::ustring& msg)
{
	sigc::slot<void> slot = sigc::bind(sigc::mem_fun(this, &NotifyPlugin::on_stop_torrent), WeakTorrentPtr(torrent));

	notify_with_action(_("File error"), msg, NOTIFY_URGENCY_CRITICAL,
		"stop", _("Stop torrent"), slot);
}

void NotifyPlugin::on_fastresume_rejected(const Linkage::TorrentPtr& torrent, const Glib::ustring& msg)
{
	notify(_("Fast resume failed"), msg, NOTIFY_URGENCY_NORMAL);
}

Plugin* create_plugin()
{
	 return new NotifyPlugin();
}

Plugin::Info plugin_info()
{
	return Plugin::Info(PLUGIN_NAME,
		PLUGIN_DESC,
		PLUGIN_VER,
		PLUGIN_AUTHOR,
		PLUGIN_WEB,
		false,
		Plugin::PARENT_NONE);
}

