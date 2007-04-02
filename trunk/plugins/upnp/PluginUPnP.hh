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
#include "UPnPManager.hh"

class UPnPPlugin : public Plugin
{
	Glib::Cond m_cond;
	Glib::Mutex m_mutex;

	enum MappedProtocol { P_NONE, P_TCP, P_UDP };
	UPnPManager* m_upnp;
	typedef std::map<unsigned int, int> PortMap;
	PortMap ports;
	void on_settings();
	void update_mappings();

public:
	Glib::ustring get_name() { return "UPnPPlugin"; };
	Glib::ustring get_description() {return "Enables port forwarding through uPnP"; };
	
	PluginParent get_parent() { return Plugin::PARENT_NONE; };
	Gtk::Widget* get_widget() { return NULL; };
	
	void on_load();
	
	UPnPPlugin();
	virtual ~UPnPPlugin();
};

#endif /* PLUGIN_UPNP_HH */
