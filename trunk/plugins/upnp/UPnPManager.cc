/*

Copyright (c) 2006 Marcelo Roberto Jimenez ( phoenix@amule.org )
Copyright (c) 2006 aMule Team ( admin@amule.org / http://www.amule.org )
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

#include <glibmm/ustring.h>
#include <upnp/upnptools.h>

#include "UPnPManager.hh"
#include <iostream>

UPnPManager* UPnPManager::self = NULL;

const char* sIGD = "urn:schemas-upnp-org:device:InternetGatewayDevice:1";

UPnPManager::Device::Device(IXML_Element* root,
														const Glib::ustring& url, 
														const Glib::ustring& location,
														const Glib::ustring& udn,
														const Glib::ustring& name,
														int expires) :
	m_url(url),
	m_location(location),
	m_udn(udn),
	m_name(name),
	m_expires(expires) 
{
	IXML_Element* elementList =	get_first_child(root, "serviceList");
	for (IXML_Element *element = get_first_child(elementList, "service"); 
				element; element = get_next_sibling(element, "service"))
	{
		Service* service = new Service(element, url);
		m_services[service->get_event_url()] = service;
	}
	
	elementList =	get_first_child(root, "deviceList");
	for (IXML_Element *element = get_first_child(elementList, "device"); 
				element; element = get_next_sibling(element, "device"))
	{
		Device* dev = new Device(element, url,	location, udn, name, expires);
		m_devices[dev->get_udn()] = dev;
	}
}

UPnPManager::Service::Service(IXML_Element *element, const Glib::ustring& URLBase)
{
	m_type = UPnPManager::get_value(element, "serviceType");
	m_id = UPnPManager::get_value(element, "serviceId");
	m_control_url = UPnPManager::get_value(element, "controlURL");
	m_event_url = UPnPManager::get_value(element, "eventSubURL");
	m_timeout = 1801;

	char* url = new char[URLBase.length() + m_control_url.length() + 1];
	if(UpnpResolveURL(URLBase.c_str(),	m_control_url.c_str(), url) == UPNP_E_SUCCESS)
		m_control_url = url;
	delete url;

	url = new char[URLBase.length() + m_event_url.length() + 1];
	if(UpnpResolveURL(URLBase.c_str(),	m_event_url.c_str(), url) == UPNP_E_SUCCESS)
		m_event_url = url;
	delete url;

//	m_wan = (m_type == UPNP_SERVICE_WAN_IP_CONNECTION || m_type == UPNP_SERVICE_WAN_PPP_CONNECTION);
	m_wan = true;
	std::cout << "New service, type: " << m_type << std::endl;

	if (m_wan)
		UPnPManager::self->subscribe(this);
}

bool UPnPManager::Service::send(const Glib::ustring& action, const UPnPManager::Service::ArgList& args)
{
	IXML_Document *doc = NULL;
	if (!args.empty())
	{
		for (unsigned int i = 0; i < args.size(); ++i)
		{
			int ret = UpnpAddToAction(&doc,	action.c_str(),	m_type.c_str(),	args[i].first.c_str(), args[i].second.c_str());
			if (ret != UPNP_E_SUCCESS)
			{
				std::cerr << "Failed to build UPnP action: " << UpnpGetErrorMessage(ret) << std::endl;
				return false;
			}
		}
	} 
	else
	{
		doc = UpnpMakeAction(action.c_str(), m_type.c_str(), 0, NULL);
		if (!doc)
		{
			std::cerr << "Failed to build UPnP action" << std::endl;
			return false;
		}
	}
	
	// Send the action synchronously
	IXML_Document* ret_doc = NULL;
	int ret = UpnpSendAction(UPnPManager::self->m_handle, m_control_url.c_str(), m_type.c_str(),	NULL, doc, &ret_doc);
	if (ret != UPNP_E_SUCCESS)
		std::cerr << "Failed to send UPnP action: " << UpnpGetErrorMessage(ret) << " URL: " << m_control_url << std::endl;

	ixmlDocument_free(doc);
	ixmlDocument_free(ret_doc);

	return (ret == UPNP_E_SUCCESS);
}

UPnPManager::UPnPManager()
{
	if (self == NULL)
		self = this;

	/* FIXME: Get Interface from settings manager and use that ip */
	if (UpnpInit(NULL, 0) != UPNP_E_SUCCESS)
		std::cerr << "Failed to intialize uPnP" << std::endl;
	
	int ret = UpnpRegisterClient(UPnPManager::upnp_cb, NULL, &m_handle);
	if (ret != UPNP_E_SUCCESS)
		std::cerr << "Failed to register UPnP client: " << UpnpGetErrorMessage(ret) << std::endl;

	m_searching = false;
}

sigc::signal<void> UPnPManager::signal_search_complete()
{
	return m_signal_search_complete;
}

void UPnPManager::search()
{
	if (m_searching)
		return;

	int ret = UpnpSearchAsync(m_handle, 3, "upnp:rootdevice", NULL);
	if (ret != UPNP_E_SUCCESS)
		std::cerr << "UPnP search failed: " << UpnpGetErrorMessage(ret) << std::endl;

	m_mutex.lock();
	m_searching = true;
	while (m_searching)
		m_cond.wait(m_mutex);
	m_mutex.unlock();

	m_signal_search_complete.emit();
}

bool UPnPManager::is_searching()
{
	return m_searching;
}

UPnPManager::~UPnPManager()
{
	UpnpFinish();
}

int UPnPManager::upnp_cb(Upnp_EventType type, void* event, void* cookie)
{
	switch (type)
	{
		case UPNP_DISCOVERY_SEARCH_TIMEOUT:
		{
			UPnPManager::self->m_mutex.lock();
			UPnPManager::self->m_searching = false;
			UPnPManager::self->m_cond.signal();
			UPnPManager::self->m_mutex.unlock();
			break;
		}
		case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
		case UPNP_DISCOVERY_SEARCH_RESULT:
		{
			Upnp_Discovery* d = static_cast<Upnp_Discovery*>(event);
			UPnPManager::self->on_discovery(type, d);
			break;
		}
		case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE:
		{
			Upnp_Discovery* d = static_cast<Upnp_Discovery*>(event);
			if (d->ErrCode == UPNP_E_SUCCESS)
			{
				Glib::ustring devType = d->DeviceType;
				if (devType == sIGD)
					UPnPManager::self->remove_igd(d->DeviceId);
			}
			break;
		}
		case UPNP_EVENT_AUTORENEWAL_FAILED:
		case UPNP_EVENT_SUBSCRIPTION_EXPIRED:
		{
			Upnp_Event_Subscribe* e =	static_cast<Upnp_Event_Subscribe*>(event);
			Upnp_SID sid;
			int timeout = 1801;
			int ret = UpnpSubscribe(UPnPManager::self->m_handle, e->PublisherUrl, &timeout,	sid);
			if (ret == UPNP_E_SUCCESS)
			{
				ServiceMap::iterator iter =	UPnPManager::self->m_services.find(e->PublisherUrl);
				if (iter != UPnPManager::self->m_services.end())
				{
					Service* service = iter->second;
					service->set_timeout(timeout);
					service->set_sid(sid);
					UPnPManager::self->refresh_port_mappings();
				}
			}
			break;
		}
		default:
			break;
	}

	return 0;
}

void UPnPManager::add_igd(IXML_Element* rootDevice, const Glib::ustring& urlBase, const Glib::ustring& location, int expires)
{
	Glib::ustring url = urlBase;
	Glib::ustring udn = get_value(rootDevice, "UDN");
	Glib::ustring name = get_value(rootDevice, "friendlyName");
	DeviceMap::iterator iter = m_devices.find(udn);
	
	if (iter == m_devices.end())
	{
		Device* dev = new Device(rootDevice, url,	location, udn, name, expires);
		m_devices[dev->get_udn()] = dev;
	}
	else
		iter->second->set_expires(expires);
}

void UPnPManager::remove_igd(const Glib::ustring& udn)
{
	DeviceMap::iterator iter = m_devices.find(udn);
	if (iter != m_devices.end())
	{
		delete m_devices[udn];
		m_devices.erase(iter);
	}
}

void UPnPManager::on_discovery(Upnp_EventType type, Upnp_Discovery* d)
{
	IXML_Document* doc = NULL;
	int ret = UpnpDownloadXmlDoc(d->Location, &doc);
	if (ret == UPNP_E_SUCCESS) 
	{
		IXML_Element* root = get_first_child(reinterpret_cast<IXML_Element*>(doc));
		Glib::ustring urlBase = get_value(root, "URLBase");
		urlBase = urlBase.empty() ? d->Location : urlBase;
		IXML_Element* rootDevice = get_first_child(root, "device");
		Glib::ustring devType = get_value(rootDevice, "deviceType");
		if (devType == sIGD)
		{
			if (type != UPNP_DISCOVERY_ADVERTISEMENT_ALIVE)
				std::cout << "Internet Gateway Device Detected." << std::endl;

			add_igd(rootDevice, urlBase, d->Location, d->Expires);
		}
		ixmlDocument_free(doc);
	}
	else
		std::cerr << "Error retrieving device description from " <<	d->Location << ": " << UpnpGetErrorMessage(ret) << std::endl;
}

void UPnPManager::subscribe(Service* service)
{
	int timeout = service->get_timeout();
	int ret = UpnpSubscribe(m_handle,	service->get_event_url().c_str(), &timeout, service->get_sid());
	if (ret == UPNP_E_SUCCESS)
	{
		service->set_timeout(timeout);

		m_services[service->get_event_url()] = service;
	}
	else
		std::cerr << "Error subscribing to " <<	service->get_event_url() << ": " << UpnpGetErrorMessage(ret) << std::endl;
}

void UPnPManager::unsubscribe(Service* service)
{
	ServiceMap::iterator iter = m_services.find(service->get_event_url());
	if (iter != m_services.end())
	{
		m_services.erase(iter);
		UpnpUnSubscribe(m_handle, service->get_sid());
		delete service;
	}
}

void UPnPManager::refresh_port_mappings()
{
}

bool UPnPManager::add_port_mapping(const Glib::ustring& port, const Glib::ustring& protocol, const Glib::ustring& address)
{
	m_mutex.lock();
	while (m_searching)
		m_cond.wait(m_mutex);
	m_mutex.unlock();

	Service::ArgList args(8);
	
	args[0].first = "NewRemoteHost";
	args[0].second = "";
	args[1].first = "NewExternalPort";
	args[1].second = port;
	args[2].first = "NewProtocol";
	args[2].second = protocol;
	args[3].first = "NewInternalPort";
	args[3].second = port;
	args[4].first = "NewInternalClient";
	args[4].second = address;
	args[5].first = "NewEnabled";
	args[5].second = "1";
	args[6].first = "NewPortMappingDescription";
	args[6].second = "";
	args[7].first = "NewLeaseDuration";
	args[7].second = "0";

	bool ret = false;
	for (ServiceMap::iterator iter = m_services.begin(); iter != m_services.end(); ++iter)
		ret = iter->second->send("AddPortMapping", args);

	return ret;
}

void UPnPManager::remove_port_mapping(const Glib::ustring& port, const Glib::ustring& protocol)
{
	m_mutex.lock();
	while (m_searching)
		m_cond.wait(m_mutex);
	m_mutex.unlock();

	Service::ArgList args(3);

	args[0].first = "NewRemoteHost";
	args[0].second = "";
	args[1].first = "NewExternalPort";
	args[1].second = port;
	args[2].first = "NewProtocol";
	args[2].second = protocol;
	
	for (ServiceMap::iterator iter = m_services.begin(); iter != m_services.end(); ++iter)
		iter->second->send("DeletePortMapping", args);
}

IXML_Element* UPnPManager::get_first_child(IXML_Element *element, const DOMString tag)
{
	if (!element || !tag)
		return NULL;

	IXML_Node *node = reinterpret_cast<IXML_Node*>(element);
	IXML_Node *child = ixmlNode_getFirstChild(node);
	const DOMString childTag = ixmlNode_getNodeName(child);
	while (child && childTag && strcmp(tag, childTag))
	{
		child = ixmlNode_getNextSibling(child);
		childTag = ixmlNode_getNodeName(child);
	}
	return reinterpret_cast<IXML_Element*>(child);
}

IXML_Element* UPnPManager::get_first_child(IXML_Element *element)
{
	if (!element)
		return NULL;

	IXML_Node *node = reinterpret_cast<IXML_Node*>(element);
	IXML_Node *child = ixmlNode_getFirstChild(node);

	return reinterpret_cast<IXML_Element*>(child);
}

Glib::ustring UPnPManager::get_value(IXML_Element *element, const DOMString tag)
{
	IXML_Element* child = get_first_child(element, tag);

	return get_value(child);
}

Glib::ustring UPnPManager::get_value(IXML_Element *element)
{
	if (element)
	{
		IXML_Node* node = ixmlNode_getFirstChild(reinterpret_cast<IXML_Node*>(element));
		const DOMString s = ixmlNode_getNodeValue(node);
		if (s)
			return s;
	}

	return "";
}

IXML_Element* UPnPManager::get_next_sibling(IXML_Element* element, const DOMString tag)
{
	if (!element || !tag)
		return NULL;

	IXML_Node *child = reinterpret_cast<IXML_Node*>(element);
	const DOMString childTag = "";
	while (child && childTag && strcmp(tag, childTag)) 
	{
		child = ixmlNode_getNextSibling(child);
		childTag = ixmlNode_getNodeName(child);
	}

	return reinterpret_cast<IXML_Element*>(child);
}

IXML_Element* UPnPManager::get_next_sibling(IXML_Element* element)
{
	if (!element)
		return NULL;

	IXML_Node *child = ixmlNode_getNextSibling(reinterpret_cast<IXML_Node*>(element));

	return reinterpret_cast<IXML_Element*>(child);
}




