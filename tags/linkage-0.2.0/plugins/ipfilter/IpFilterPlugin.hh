/*
Copyright (C) 2008	Christian Lundgren

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

#ifndef IP_FILTER_PLUGIN_HH
#define IP_FILTER_PLUGIN_HH

#include <gtkmm/filechooserbutton.h>

#include "linkage/Plugin.hh"

#include "p2p/list.hpp"

class IpFilterPlugin : public Linkage::Plugin
{
	Gtk::Dialog *dialog;
	Gtk::Label *label_info, *label_result;

	p2p::list filter;

	void on_file_changed(Gtk::FileChooserButton* button);

	void on_dialog_response(int response_id);
	
public:
	Linkage::Plugin::Info get_info();

	Gtk::Dialog* get_config_dialog() { return dialog; }

	IpFilterPlugin();
	~IpFilterPlugin();
};

#endif /* IP_FILTER_PLUGIN_HH */

