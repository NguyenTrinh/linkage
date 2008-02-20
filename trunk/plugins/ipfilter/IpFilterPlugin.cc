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

#include <glib/gstdio.h>
#include <sys/mman.h>

#include <glibmm/i18n.h>
#include <gtkmm/stock.h>
#include <gtkmm/image.h>
#include <gtkmm/dialog.h>
#include <gtkmm/label.h>

#include "linkage/Engine.hh"
#include "linkage/SessionManager.hh"
#include "linkage/Utils.hh"

#include "libtorrent/ip_filter.hpp"

#include "IpFilterPlugin.hh"

#include <istream>

/* dbus-c++ defines these ... */
#ifdef HAVE_CONFIG_H 
#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef VERSION

#include "config.h"
#endif

#define PLUGIN_NAME	"IP filter"
#define PLUGIN_DESC	_("Import IP diffrent filters")
#define PLUGIN_VER	PACKAGE_VERSION
#define PLUGIN_AUTHOR	"Christian Lundgren.\nLibp2p was written by Cory Nelson under the zlib license"
#define PLUGIN_WEB	"http://code.google.com/p/linkage"

#define GZ_LEN 8192

/* LT pulls in zlib */

static bool decode(const std::string& file, std::streambuf* in)
{
	char buf[GZ_LEN];
	gzFile fd;
	int len = 0;

	fd = gzopen(file.c_str(), "rb");
	if (!fd)
		return false;

	while (true)
	{
		len = gzread(fd, buf, GZ_LEN);

		if (len <= 0)
			break;

		in->sputn(buf, len);
	}

	gzclose(fd);

	return len != -1;
}

static void encode(const std::string& file, std::stringstream& out)
{
	char buf[GZ_LEN];
	gzFile fd;

	fd = gzopen(file.c_str(), "wb");
	if (!fd)
		return;

	while (true)
	{
		out.read(buf, GZ_LEN);

		gzwrite(fd, buf, out.gcount());

		if (out.tellg() < 0)
			break;
	}

	gzclose(fd);
}


static bool load_filter(const std::string& file, p2p::list& filter, p2p::list::file_type type = p2p::list::file_auto)
{
	p2p::list list;
	bool ret = false;

	std::stringstream stream;
	std::streambuf* in = stream.rdbuf();
	
	if (!decode(file, in))
		return false;	

	try
	{
		list.load(stream, type);
		ret = true;
	}
	catch (std::exception& ex) {
		g_debug(ex.what());
		ret = false;
	}

	if (ret)
	{
		filter.insert(list);
		filter.optimize(true);
	}

	return ret && list.size();
}

static libtorrent::ip_filter p2p_to_lt(const p2p::list& list)
{
	libtorrent::ip_filter lt_filter;
	libtorrent::address begin, end;
	char b_str[16], e_str[16];

	for (p2p::list::const_iterator iter = list.begin();
		iter != list.end(); ++iter)
	{
		sprintf(b_str, "%i.%i.%i.%i", iter->start.ipb[3], iter->start.ipb[2], iter->start.ipb[1], iter->start.ipb[0]);
		sprintf(e_str, "%i.%i.%i.%i", iter->end.ipb[3], iter->end.ipb[2], iter->end.ipb[1], iter->end.ipb[0]);
		begin = libtorrent::address::from_string(b_str);
		end = libtorrent::address::from_string(e_str);
		lt_filter.add_rule(begin, end, libtorrent::ip_filter::blocked);
	}

	return lt_filter;
}

IpFilterPlugin::IpFilterPlugin()
{
	dialog = new Gtk::Dialog(_("Global IP Filter"));

	/* set up child widgets for dialog */
	label_info = Gtk::manage(new Gtk::Label());
	label_info->set_line_wrap(true);
	label_result = Gtk::manage(new Gtk::Label());

	Gtk::Image* image = Gtk::manage(new Gtk::Image(Gtk::Stock::DIALOG_INFO, Gtk::ICON_SIZE_DIALOG));
	Gtk::VBox* vbox = dialog->get_vbox();
	Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox());

	Gtk::FileChooserButton* button = Gtk::manage(new Gtk::FileChooserButton());
	button->signal_selection_changed().connect(sigc::bind(sigc::mem_fun(this, &IpFilterPlugin::on_file_changed), button));

	hbox->pack_start(*image);
	hbox->pack_start(*label_info);

	vbox->add(*hbox);
	vbox->add(*button);
	vbox->add(*label_result);

	Gtk::Button* b = dialog->add_button(_("Clear filter"), Gtk::RESPONSE_REJECT);
	image = Gtk::manage(new Gtk::Image(Gtk::Stock::CLEAR, Gtk::ICON_SIZE_BUTTON));
	b->set_image(*image);
	dialog->add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_CLOSE);

	dialog->signal_response().connect(sigc::mem_fun(this, &IpFilterPlugin::on_dialog_response));

	dialog->show_all_children();

	std::string file = Glib::build_filename(Linkage::get_config_dir(), "ip_filter.p2p.gz");
	/* FIXME: this blocks badly, thread list loading */
	load_filter(file, filter);
	label_info->set_text(String::ucompose(
		_("Current filter contains %1 filtered IP ranges.\n\nTo add more ranges select a file with the button below. Supported formats are P2P and P2B."), filter.size()));
	Linkage::Engine::get_session_manager()->set_ip_filter(p2p_to_lt(filter));
}

IpFilterPlugin::~IpFilterPlugin()
{
	delete dialog;
}

void IpFilterPlugin::on_file_changed(Gtk::FileChooserButton* button)
{
	if (load_filter(button->get_filename(), filter))
		label_result->set_markup(_("<i>Successfully added new range to filter.</i>"));
	else
		label_result->set_markup(_("<i>Failed to indentify file type!</i>"));

	label_info->set_text(String::ucompose(
		_("Current filter contains %1 filtered IP ranges.\n\nTo add more ranges select a file with the button below. Supported formats are P2P and P2B."), filter.size()));
	Linkage::Engine::get_session_manager()->set_ip_filter(p2p_to_lt(filter));
}

void IpFilterPlugin::on_dialog_response(int response_id)
{
	std::stringstream out;

	switch (response_id)
	{
	case Gtk::RESPONSE_REJECT:
		filter = p2p::list();
		label_info->set_text(String::ucompose(
		_("Current filter contains %1 filtered IP ranges.\n\nTo add more ranges select a file with the button below. Supported formats are P2P and P2B."), filter.size()));
	case Gtk::RESPONSE_CLOSE:
		Linkage::Engine::get_session_manager()->set_ip_filter(p2p_to_lt(filter));
		/* FIXME: peerguardian lists contains illegal non ISO-8859-1 text,
			so saving/loading as p2b fails badly :( */
		/* FIXME: this blocks badly, thread list saving */
		filter.save(out, p2p::list::file_p2p);
		encode(Glib::build_filename(Linkage::get_config_dir(), "ip_filter.p2p.gz"), out);
		break;
	default:
		break;
	}

	dialog->hide();
}

Linkage::Plugin::Info IpFilterPlugin::get_info()
{
	return Linkage::Plugin::Info(PLUGIN_NAME,
		PLUGIN_DESC,
		PLUGIN_VER,
		PLUGIN_AUTHOR,
		PLUGIN_WEB,
		false,
		Plugin::PARENT_NONE);
}

Linkage::Plugin* create_plugin()
{
	 return new IpFilterPlugin();
}

Linkage::Plugin::Info plugin_info()
{
	return Linkage::Plugin::Info(PLUGIN_NAME,
		PLUGIN_DESC,
		PLUGIN_VER,
		PLUGIN_AUTHOR,
		PLUGIN_WEB,
		true,
		Linkage::Plugin::PARENT_NONE);
}

