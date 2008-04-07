/*
Copyright (C) 2008	Dave Moore

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

#include "PluginScheduler.hh"

#include <time.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <glibmm/i18n.h>

#include <linkage/Utils.hh>
#include <linkage/Engine.hh>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

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

#define PLUGIN_NAME	"SchedulerPlugin"
#define PLUGIN_DESC	_("Changes bandwidth settings according to schedule")
#define PLUGIN_VER	PACKAGE_VERSION
#define PLUGIN_AUTHOR	"Dave Moore"
#define PLUGIN_WEB	"http://code.google.com/p/linkage"

using namespace Linkage;

SchedulerPlugin::SchedulerPlugin()
{
	// load the config file
	m_config_file = Glib::build_filename(get_config_dir(), "scheduler.cfg");
	if (!g_file_test(m_config_file.c_str(), G_FILE_TEST_EXISTS))
	{
		m_config.current_schedule = 0;
		m_config.schedules.push_back(Schedule());
	}
	else
	{
		std::ifstream ifs(m_config_file.c_str(), std::ios::binary);
		boost::archive::text_iarchive ia(ifs);
		ia >> m_config;
	}

	// create the configure dialog
	Glib::RefPtr<Gnome::Glade::Xml> xml;
	try
	{
		xml = Gnome::Glade::Xml::create(DATA_DIR "/scheduler.glade");
	}
	catch (const Gnome::Glade::XmlError& ex)
	{
		g_error(ex.what().c_str());
	}
	xml->get_widget_derived("schedule_dialog", m_dialog);
	m_dialog->signal_response().connect(sigc::mem_fun(this, &SchedulerPlugin::on_dialog_response));
	m_dialog->configure(m_config);
	// set up the timers and current schedule
	using namespace boost::posix_time;
	using namespace boost::gregorian;
	
	m_next_hour_slot = sigc::mem_fun(*this, &SchedulerPlugin::on_next_hour);
	m_hourly_slot = sigc::mem_fun(*this, &SchedulerPlugin::on_every_hour);

	// get current time
	ptime now = second_clock::local_time();

	// figure out when the next hour strikes
	time_duration t = hours(1) - minutes(now.time_of_day().minutes()) - seconds(now.time_of_day().seconds());

	// create an event for the next hour which creates an hourly event when it dies
	Glib::signal_timeout().connect(m_next_hour_slot, t.total_milliseconds());
	
	// apply the current schedule
	apply_limits();
}

SchedulerPlugin::~SchedulerPlugin()
{
	m_next_hour_conn.disconnect();
	m_hourly_conn.disconnect();
}

Plugin::Info SchedulerPlugin::get_info()
{
	return Plugin::Info(PLUGIN_NAME,
		PLUGIN_DESC,
		PLUGIN_VER,
		PLUGIN_AUTHOR,
		PLUGIN_WEB,
		true,
		Plugin::PARENT_NONE);
}

//void SchedulerPlugin::configure() 
//{
	//Glib::RefPtr<Gnome::Glade::Xml> xml;
	//try
	//{
		//xml = Gnome::Glade::Xml::create(DATA_DIR "/scheduler.glade");
	//}
	//catch (const Gnome::Glade::XmlError& ex)
	//{
		//g_error(ex.what().c_str());
	//}
	//ScheduleWin* dialog = NULL;
	//xml->get_widget_derived("schedule_dialog", dialog);
	
	//int res = dialog->run(m_config);
	//if (res == Gtk::RESPONSE_OK)
	//{
		//// copy and save config
		//m_config = dialog->get_config();
		//save_config(m_config);
	//}
	//delete dialog;
//}

Gtk::Dialog* SchedulerPlugin::get_config_dialog()
{
	return (Gtk::Dialog*)m_dialog;
}

void SchedulerPlugin::on_dialog_response(int response)
{
	if (response == Gtk::RESPONSE_OK)
	{
		// copy and save config
		m_config = m_dialog->get_config();
		save_config(m_config);
	}
	m_dialog->hide();
}

void SchedulerPlugin::save_config(const SchedulerConfig & cfg)
{
	std::ofstream ofs(m_config_file.c_str());
	boost::archive::text_oarchive oa(ofs);
	oa << cfg;
}

bool SchedulerPlugin::on_next_hour()
{
	using namespace boost::posix_time;

	apply_limits();
	// set the timer to go off every hour
	Glib::signal_timeout().connect(m_hourly_slot, hours(1).total_milliseconds());
	return false;
}

bool SchedulerPlugin::on_every_hour()
{
	apply_limits();
	return true;
}

void SchedulerPlugin::apply_limits()
{
	using namespace boost::posix_time;
	ptime now = second_clock::local_time();
	int hour = now.time_of_day().hours();
	int day = now.date().day_of_week();
	day = day == 0 ? day=6 : day-1;
	
	int l = m_config.get_current_schedule().schedule[day][hour];
	Limit lim = m_config.get_current_schedule().limits[l];
	
	SessionManagerPtr sesm = Engine::get_session_manager();
	if (l == 0) // normal
	{
		SettingsManagerPtr setm = Engine::get_settings_manager();
		int up_rate = setm->get_int("network/max_up_rate")*1024;
		int down_rate = setm->get_int("network/max_up_rate")*1024;
		if (up_rate < 1) up_rate = -1;
		if (down_rate < 1) down_rate = -1;
		sesm->set_upload_rate_limit(up_rate);
		sesm->set_download_rate_limit(down_rate);
	}
	else if (l == 4) // blocked
	{
		// FIXME: this should stop the session or something
		// other than set the limits to 1 B/s
		sesm->set_upload_rate_limit(1);
		sesm->set_download_rate_limit(1);
	}
	else
	{
		sesm->set_upload_rate_limit(lim.up * 1024);
		sesm->set_download_rate_limit(lim.down * 1024);
	}


}

Plugin* create_plugin()
{
	 return new SchedulerPlugin();
}

Plugin::Info plugin_info()
{
	return Plugin::Info(PLUGIN_NAME,
		PLUGIN_DESC,
		PLUGIN_VER,
		PLUGIN_AUTHOR,
		PLUGIN_WEB,
		true,
		Plugin::PARENT_NONE);
}


