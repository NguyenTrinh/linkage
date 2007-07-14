/*
Copyright (C) 2007	Christian Lundgren

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
#include <iostream>
#include <glibmm/thread.h>
#include <gtkmm/main.h>

#include "linkage/Engine.hh"
#include "linkage/Utils.hh"

#include "PluginUPnP.hh"

UPnPPlugin::UPnPPlugin()
{
	GError* error = NULL;
	Glib::ustring iface = Engine::get_settings_manager()->get_string("network/interface");
	Glib::ustring ip = get_ip(iface);

	m_context = gupnp_context_new(NULL, ip.empty() ? NULL : ip.c_str(), 0, &error);
	if (error)
	{
		g_warning(error->message);
		g_error_free(error);
	}

	m_cp = gupnp_control_point_new(m_context, GSSDP_ALL_RESOURCES);

	g_signal_connect(m_cp, "device-proxy-available", G_CALLBACK(UPnPPlugin::on_device_found), this);
	g_signal_connect(m_cp, "device-proxy-unavailable", G_CALLBACK(UPnPPlugin::on_device_lost), this);
	g_signal_connect(m_cp, "service-proxy-available", G_CALLBACK(UPnPPlugin::on_service_found), this);
	g_signal_connect(m_cp, "service-proxy-unavailable", G_CALLBACK(UPnPPlugin::on_service_lost), this);

	gssdp_resource_browser_set_active(reinterpret_cast<GSSDPResourceBrowser*>(m_cp), TRUE);
}

UPnPPlugin::~UPnPPlugin()
{
	g_object_unref(m_cp);
	g_object_unref(m_context);
}

void UPnPPlugin::on_device_found(GUPnPControlPoint* cp, GUPnPDeviceProxy* proxy, gpointer data)
{
	/* FIXME: store devices */
}

void UPnPPlugin::on_device_lost(GUPnPControlPoint* cp, GUPnPDeviceProxy* proxy, gpointer data)
{
	/* FIXME: remove device from store */
}

void UPnPPlugin::on_service_found(GUPnPControlPoint* cp, GUPnPServiceProxy* proxy, gpointer data)
{
	/* FIXME: store services */
	GUPnPServiceInfo* info = reinterpret_cast<GUPnPServiceInfo*>(proxy);
	char* type = gupnp_service_info_get_service_type(info);
	if (type)
	{
		if (strcmp(type, "urn:schemas-upnp-org:service:WANIPConnection:1") == 0)
		{
			gupnp_service_proxy_set_subscribed(proxy, TRUE);
			g_signal_connect(proxy, "subscription-lost", G_CALLBACK(UPnPPlugin::on_subscription_lost), data);
			UPnPPlugin* plugin = static_cast<UPnPPlugin*>(data);
			plugin->on_wan_service_found(proxy);
		}
		g_free(type);
	}
}

void UPnPPlugin::on_service_lost(GUPnPControlPoint* cp, GUPnPServiceProxy* proxy, gpointer data)
{
	/* FIXME: remove service from store */
}

void UPnPPlugin::on_subscription_lost(GUPnPServiceProxy *proxy, gpointer error, gpointer data)
{
	/* FIXME: renew subscription */
}

void UPnPPlugin::on_wan_service_found(GUPnPServiceProxy* proxy)
{
	unsigned int port = Engine::get_session_manager()->listen_port();
	Glib::ustring iface = Engine::get_settings_manager()->get_string("network/interface");
	Glib::ustring ip = get_ip(iface);
	if (ip.empty())
		ip = gupnp_context_get_host_ip(m_context);

	GError* error = NULL;

	gupnp_service_proxy_begin_action(proxy,
		"AddPortMapping",
		UPnPPlugin::on_action_done,
		this,
		&error,
		"NewRemoteHost", G_TYPE_STRING, "",
		"NewExternalPort", G_TYPE_UINT, port,
		"NewProtocol", G_TYPE_STRING, "TCP",
		"NewInternalPort", G_TYPE_UINT, port,
		"NewInternalClient", G_TYPE_STRING, ip.c_str(),
		"NewEnabled", G_TYPE_UINT, 1,
		"NewPortMappingDescription", G_TYPE_STRING, PACKAGE_NAME "/" PACKAGE_VERSION,
		"NewLeaseDuration", G_TYPE_INT, 0,
		NULL);
	if (error)
	{
		g_warning(error->message);
		g_error_free(error);
		error = NULL;
	}

	gupnp_service_proxy_begin_action(proxy,
		"AddPortMapping",
		UPnPPlugin::on_action_done,
		this,
		&error,
		"NewRemoteHost", G_TYPE_STRING, "",
		"NewExternalPort", G_TYPE_UINT, port,
		"NewProtocol", G_TYPE_STRING, "UDP",
		"NewInternalPort", G_TYPE_UINT, port,
		"NewInternalClient", G_TYPE_STRING, ip.c_str(),
		"NewEnabled", G_TYPE_UINT, 1,
		"NewPortMappingDescription", G_TYPE_STRING, PACKAGE_NAME "/" PACKAGE_VERSION,
		"NewLeaseDuration", G_TYPE_INT, 0,
		NULL);
	if (error)
	{
		g_warning(error->message);
		g_error_free(error);
	}
}

void UPnPPlugin::on_action_done(GUPnPServiceProxy *proxy, GUPnPServiceProxyAction *action, gpointer data)
{
	GError* error = NULL;
	bool ret = gupnp_service_proxy_end_action(proxy, action, &error, NULL);
	
	g_warning("Action done: %i", ret);
	if (error)
	{
		g_warning(error->message);
		g_error_free(error);
	}
}

Plugin::Info UPnPPlugin::get_info()
{
	return Plugin::Info("UPnPPlugin",
		"Enables port forwarding through gssdp/gupnp",
		"1",
		"Christian Lundgren",
		"http://code.google.com/p/linkage",
		false,
		Plugin::PARENT_NONE);
}

Plugin* create_plugin()
{
	 return new UPnPPlugin();
}

Plugin::Info plugin_info()
{
	return Plugin::Info("UPnPPlugin",
		"Enables port forwarding through gssdp/gupnp",
		"1",
		"Christian Lundgren",
		"http://code.google.com/p/linkage",
		false,
		Plugin::PARENT_NONE);
}

