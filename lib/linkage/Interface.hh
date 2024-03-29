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

#ifndef INTERFACE_HH
#define INTERFACE_HH

#include <list>

#include <glibmm/refptr.h>
#include <gtkmm/container.h>

#include "linkage/Torrent.hh"
#include "linkage/Plugin.hh"

#include "linkage/DBusAdaptorGlue.hh"

namespace Linkage
{

typedef std::list<TorrentPtr> SelectionList;

class Interface
:
  public org::linkage::Interface,
  public DBus::IntrospectableAdaptor,
  public DBus::ObjectAdaptor
{
private:
	Interface(const Interface&);
	Interface& operator=(const Interface&);

	/* DBUS */
	void Open(const DBus::String& file);
	void Add(const DBus::String& file, const DBus::String& path);
	DBus::Bool GetVisible();
	void SetVisible(const DBus::Bool& visible);
	void Quit();

public:
	virtual SelectionList get_selected() const;

	virtual bool get_visible() const;
	virtual void set_visible(bool visible);

	virtual Gtk::Container* get_container(Plugin::PluginParent parent);

	virtual void open(const Glib::ustring& uri);

	virtual void quit();

	Interface();
	virtual ~Interface();
};

} /* namespace */

#endif /* INTERFACE_HH */

