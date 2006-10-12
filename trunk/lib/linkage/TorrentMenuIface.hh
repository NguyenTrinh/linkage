/*
Copyright (C) 2006  Christian Lundgren

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef TORRENTMENU_IFACE_HH
#define TORRENTMENU_IFACE_HH

#include "linkage/Plugin.hh"
#include "gtkmm/menuitem.h"

class TorrentMenuPlugin : public Plugin
{
public:
  virtual Plugin::PluginParent get_parent();
  
  virtual Gtk::MenuItem* get_menu();
  
  TorrentMenuPlugin();
  TorrentMenuPlugin(const Glib::ustring& name, const Glib::ustring& description, int version);
  ~TorrentMenuPlugin();
};

#endif /* TORRENTMENU_IFACE_HH */
