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
#include <gtkmm/widget.h>

class Plugin : public sigc::trackable
{
private:	
	sigc::signal<void, Plugin*> m_signal_unloading;

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

		Info(const Glib::ustring& n,
			const Glib::ustring& d,
			const Glib::ustring& v,
			const Glib::ustring& a,
			const Glib::ustring& w,
			bool h,
			PluginParent p) :
				name(n),
				description(d),
				version(v),
				author(a),
				website(w),
				has_options(h),
				parent(p) {}
		Info() {}
	};

	sigc::signal<void, Plugin*> signal_unloading();

	/*
		Do _not_ override with return plugin_info(), since that function might
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

/* Glib::module needs standard C names */
#ifdef __cplusplus
extern "C" {
#endif

/* This is the function each plugin implements for the main app to
	 create an instance of the plugin child class. */
extern Plugin* create_plugin();

extern Plugin::Info plugin_info();

#ifdef __cplusplus
}
#endif

#endif /* PLUGIN_HH */

