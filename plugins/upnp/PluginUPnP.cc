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

#include <glibmm/thread.h>

#include "linkage/Engine.hh"
#include "linkage/Utils.hh"

#include "PluginUPnP.hh"

UPnPPlugin::UPnPPlugin()
{
}

UPnPPlugin::~UPnPPlugin()
{
	/* Hack to make sure we don't leave the search thread behind */
	m_mutex.lock();
	while (m_upnp->is_searching())
		m_cond.wait(m_mutex);
	m_mutex.unlock();

	for (PortMap::iterator iter = ports.begin(); iter != ports.end(); ++iter)
	{
		if (iter->second & P_TCP)
		{
			if (m_upnp->remove_port_mapping(str(iter->first), "TCP"))
				std::cout << "Successfully unmapped port: " << iter->first << " (TCP)" << std::endl;
			else
				std::cout << "Failed to unmap port: " << iter->first << " (TCP)" << std::endl;
		}
		if (iter->second & P_UDP)
		{
			if (m_upnp->remove_port_mapping(str(iter->first), "UDP"))
				std::cout << "Successfully unmapped port: " << iter->first << " (UDP)" << std::endl;
			else
				std::cout << "Failed to unmap port: " << iter->first << " (UDP)" << std::endl;
		}
	}

	if (m_upnp)
		delete m_upnp;
}

void UPnPPlugin::on_load()
{
	/* FIXME: Pass ip and min_port to manager */
	m_upnp = new UPnPManager();
	m_upnp->signal_search_complete().connect(sigc::mem_fun(this, &UPnPPlugin::update_mappings));

	Glib::Thread* search = Glib::Thread::create(sigc::mem_fun(m_upnp, &UPnPManager::search), false);

	Engine::get_settings_manager()->signal_update_settings().connect(sigc::mem_fun(this, &UPnPPlugin::on_settings));
}

void UPnPPlugin::on_settings()
{
	unsigned int port = Engine::get_session_manager()->listen_port();
	ports[port] = P_NONE;

	if (!m_upnp->is_searching())
		update_mappings();
}

void UPnPPlugin::update_mappings()
{
	/* FIXME: Remove old mappings */
	Glib::ustring iface = Engine::get_settings_manager()->get_string("Network", "Interface");
	ip_address ip;

	unsigned int port = Engine::get_session_manager()->listen_port();
	if (ports.find(port) == ports.end())
		ports[port] = P_NONE;

	for (PortMap::iterator iter = ports.begin(); iter != ports.end(); ++iter)
	{
		port = iter->first;
		if (iter->second & P_TCP && iter->second & P_UDP)
			continue;

		if (get_ip(iface.c_str(), ip))
		{
			if (m_upnp->add_port_mapping(str(port), "TCP", ip))
				ports[port] |= P_TCP;
			if (m_upnp->add_port_mapping(str(port), "UDP", ip))
				ports[port] |= P_UDP;

			if (ports[port] & P_TCP)
				std::cout << "Successfully mapped " << ip << ":" << port << " (TCP)" << std::endl;
			else
				std::cout << "Failed to map " << ip << ":" << port << " (TCP)" << std::endl;
			if (ports[port] & P_UDP)
				std::cout << "Successfully mapped " << ip << ":" << port << " (UDP)" << std::endl;
			else
				std::cout << "Failed to map " << ip << ":" << port << " (UDP)" << std::endl;
		}
		else
		{
			/* No valid interface is specified so we'll just try any ip */
			std::list<Glib::ustring> ifaces = get_interfaces();
			for (std::list<Glib::ustring>::iterator iter = ifaces.begin();
						iter != ifaces.end(); ++iter)
			{
				/* Skip loop back interface */
				if (*iter == "lo")
					continue;

				if (get_ip((*iter).c_str(), ip))
				{
					if (m_upnp->add_port_mapping(str(port), "TCP", ip))
						ports[port] |= P_TCP;
					if (m_upnp->add_port_mapping(str(port), "UDP", ip))
						ports[port] |= P_UDP;

					if (ports[port] & P_TCP)
						std::cout << "Successfully mapped " << ip << ":" << port << " (TCP)" << std::endl;
					else
						std::cout << "Failed to map " << ip << ":" << port << " (TCP)" << std::endl;
					if (ports[port] & P_UDP)
						std::cout << "Successfully mapped " << ip << ":" << port << " (UDP)" << std::endl;
					else
						std::cout << "Failed to map " << ip << ":" << port << " (UDP)" << std::endl;
				}
			}
		}
	}
	m_mutex.lock();
	m_cond.signal();
	m_mutex.unlock();
}

Plugin* CreatePlugin()
{
	 return new UPnPPlugin();
}
