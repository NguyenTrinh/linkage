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

#include "linkage/Engine.hh"
#include "linkage/Utils.hh"

#include "PluginUPnP.hh"

UPnPPlugin::UPnPPlugin()
{
}

UPnPPlugin::~UPnPPlugin()
{
	for (std::list<unsigned int>::iterator iter = ports.begin();
				iter != ports.end(); ++iter)
	{
		m_upnp->remove_port_mapping(str(*iter), "TCP");
		m_upnp->remove_port_mapping(str(*iter), "UDP");
	}

	if (m_upnp)
		delete m_upnp;
}

void UPnPPlugin::on_load()
{
	/* FIXME: Pass ip and min_port to manager */
	m_upnp = new UPnPManager();

	Engine::get_settings_manager()->signal_update_settings().connect(sigc::mem_fun(this, &UPnPPlugin::on_settings));

	on_settings();
}

void UPnPPlugin::on_settings()
{
	/* FIXME: Remove old mappings */
	Glib::ustring iface = Engine::get_settings_manager()->get_string("Network", "Interface");
	ip_address ip;

	unsigned int port = Engine::get_session_manager()->listen_port();
	if (get_ip(iface.c_str(), ip))
	{
		bool ret = m_upnp->add_port_mapping(str(port), "TCP", ip);
		ret = m_upnp->add_port_mapping(str(port), "UDP", ip) && ret;
		if (ret)
		{
			ports.push_back(port);
			std::cout << "Port forward succeded for " << ip << ":" << port << std::endl;
		}
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
				std::cout << "Mapping port for ip " << ip << std::endl;
				bool ret = m_upnp->add_port_mapping(str(port), "TCP", ip);
				ret = m_upnp->add_port_mapping(str(port), "UDP", ip) && ret;

				if (ret)
				{
					ports.push_back(port);
					std::cout << "Port forward succeded for " << ip << ":" << port << std::endl;
				}
			}
		}
	}
}

Plugin* CreatePlugin()
{
	 return new UPnPPlugin();
}
