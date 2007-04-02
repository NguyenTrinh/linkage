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

#ifndef UPNP_MANAGER_HH
#define UPNP_MANAGER_HH

#include <map>
#include <vector>

#include <glibmm/ustring.h>
#include <glibmm/thread.h>
#include "upnp/upnp.h"

class UPnPManager : public sigc::trackable
{
	class Device;
	class Service;
	typedef std::map<Glib::ustring, Device*> DeviceMap;
	typedef std::map<Glib::ustring, Service*> ServiceMap;

	class Device
	{
		Glib::ustring m_url, m_location, m_udn, m_name;
		int m_expires;
		DeviceMap m_devices;
		ServiceMap m_services;
	public:
		Device(IXML_Element* root,
						const Glib::ustring& url, 
						const Glib::ustring& location,
						const Glib::ustring& udn,
						const Glib::ustring& name,
						int expires);
		~Device() {}

		const Glib::ustring& get_url() { return m_url; }
		const Glib::ustring& get_location() { return m_location; }
		int get_expires() { return m_expires; }
		void set_expires(int expires)	{ m_expires = expires; }
		const Glib::ustring& get_udn() { return m_udn; }
		const Glib::ustring& get_name() { return m_name; }
	};

	class Service
	{
		Glib::ustring m_type;
		Glib::ustring m_id;
		Glib::ustring m_control_url;
		Glib::ustring m_event_url;
		int m_timeout;
		Upnp_SID m_sid;
		bool m_wan;

	public:
		Service(IXML_Element *element, const Glib::ustring& URLBase);
		~Service() {};
		
		const Glib::ustring& get_type() { return m_type; }
		const Glib::ustring& get_id() { return m_id; }
		const Glib::ustring& get_control_url() { return m_control_url; }
		const Glib::ustring& get_event_url() { return m_event_url; }
		int get_timeout() { return m_timeout; }
		void set_timeout(int timeout) { m_timeout = timeout; }
		char* get_sid() { return m_sid; }
		void set_sid(const char* sid) { memcpy(m_sid, sid, sizeof(Upnp_SID)); }
		bool is_wan() { return m_wan; }

		typedef std::vector<std::pair<Glib::ustring, Glib::ustring> > ArgList;
		bool send(const Glib::ustring& action, const ArgList& args);
	};

	/* According to aMule using the cookie in the callback is unreliable */
	static UPnPManager* self;

	Glib::Cond m_cond;
	Glib::Mutex m_mutex;
	bool m_searching;
	sigc::signal<void> m_signal_search_complete;

	DeviceMap m_devices;
	ServiceMap m_services;

	UpnpClient_Handle m_handle;

	static int upnp_cb(Upnp_EventType type, void* event, void* cookie);

	void add_igd(IXML_Element *rootDevice, const Glib::ustring& urlBase, const Glib::ustring& location, int expires);
	void remove_igd(const Glib::ustring& udn);

	void on_discovery(Upnp_EventType type, Upnp_Discovery* d);

	void refresh_port_mappings();

	void subscribe(Service* service);
	void unsubscribe(Service* service);

	static IXML_Element* get_first_child(IXML_Element* element, const DOMString tag);
	static IXML_Element* get_first_child(IXML_Element* element);
	static IXML_Element* get_next_sibling(IXML_Element* element, const DOMString tag);
	static IXML_Element* get_next_sibling(IXML_Element* element);
	static Glib::ustring get_value(IXML_Element *element, const DOMString tag);
	static Glib::ustring get_value(IXML_Element *element);
public:
	void search();
	bool is_searching();
	sigc::signal<void> signal_search_complete();

	bool add_port_mapping(const Glib::ustring& port, const Glib::ustring& protocol, const Glib::ustring& address);
	bool remove_port_mapping(const Glib::ustring& port, const Glib::ustring& protocol);

	UPnPManager();
	~UPnPManager();
};

#endif /* UPNP_MANAGER_HH */
