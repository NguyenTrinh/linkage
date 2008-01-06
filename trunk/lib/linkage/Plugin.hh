/*
Copyright (C) 2006	Christian Lundgren

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

#ifndef PLUGIN_HH
#define PLUGIN_HH

#include <list>

#include <sigc++/signal.h>

#include <glibmm/ustring.h>

#include <boost/smart_ptr.hpp>

#include "libtorrent/intrusive_ptr_base.hpp"

namespace Gtk
{
class Widget;
};

namespace Linkage
{
class Plugin;
typedef boost::intrusive_ptr<Plugin> PluginPtr;

class Plugin : public libtorrent::intrusive_ptr_base<Plugin>
{
public:
	enum PluginParent { PARENT_NONE, PARENT_MAIN, PARENT_DETAILS, PARENT_MENU, PARENT_TOOLBAR };

	struct Info
	{
		Glib::ustring name;
		Glib::ustring description;
		Glib::ustring version;
		Glib::ustring author;
		Glib::ustring website;
		bool has_options;
		PluginParent parent;

		Info(const Glib::ustring& name_,
			const Glib::ustring& description_,
			const Glib::ustring& version_,
			const Glib::ustring& author_,
			const Glib::ustring& website_,
			bool has_options_,
			PluginParent parent_) :
				name(name_),
				description(description_),
				version(version_),
				author(author_),
				website(website_),
				has_options(has_options_),
				parent(parent_) {}
		Info() {}
	};

	sigc::signal<void, Plugin*> signal_unloading();

	/*
		Do _not_ override with "return plugin_info()", since that function might
		be bound to another plugin. Should return an equal struct of plugin_info()
	*/
	virtual Plugin::Info get_info();

	/* This can be used to pass data to another plugin or whatever */
	/* FIXME: Use dbus manager as well, allow plugin to register signals/interfaces */
	virtual gpointer get_user_data(gpointer arg = NULL) { return NULL; }

	/*
		Override and return a widget containing your plugins options,
		Use Gtk::Widget::signal_parent_changed() to be notified when the configure
		window is closed, the widget should be deleted by the plugin.
	*/
	virtual Gtk::Widget* get_config_widget() { return NULL; }

	Plugin();
	virtual ~Plugin();
};

}; /* namespace */

/* Glib::module needs standard C names */
#ifdef __cplusplus
extern "C" {
#endif

/* override and return a newly created plugin object */
extern Linkage::Plugin* create_plugin();

/* should return be equal the overridden Plugin::get_info(), implemented separately */
extern Linkage::Plugin::Info plugin_info();

#ifdef __cplusplus
}
#endif

#endif /* PLUGIN_HH */

