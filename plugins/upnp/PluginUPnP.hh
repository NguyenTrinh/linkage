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

#ifndef PLUGIN_UPNP_HH
#define PLUGIN_UPNP_HH

#include "linkage/Plugin.hh"

#include "libgssdp/gssdp.h"
#include "libgupnp/gupnp.h"

class UPnPPlugin : public Plugin
{
	GUPnPControlPoint* m_cp;
	GUPnPContext* m_context;

	static void on_device_found(GUPnPControlPoint* cp, GUPnPDeviceProxy* proxy, gpointer data);
	static void on_device_lost(GUPnPControlPoint* cp, GUPnPDeviceProxy* proxy, gpointer data);
	static void on_service_found(GUPnPControlPoint* cp, GUPnPServiceProxy* proxy, gpointer data);
	static void on_service_lost(GUPnPControlPoint* cp, GUPnPServiceProxy* proxy, gpointer data);

	static void on_subscription_lost(GUPnPServiceProxy *proxy, gpointer error, gpointer data);

	void on_wan_service_found(GUPnPServiceProxy* proxy);
	static void on_action_done(GUPnPServiceProxy *proxy, GUPnPServiceProxyAction *action, gpointer data);

public:
	Gtk::Widget* get_config_widget() { return NULL; }

	Plugin::Info get_info();
	
	UPnPPlugin();
	virtual ~UPnPPlugin();
};

#endif /* PLUGIN_UPNP_HH */

