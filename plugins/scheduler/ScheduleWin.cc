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

#include "ScheduleWin.hh"

#include "linkage/Engine.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/Utils.hh"

#include <gdkmm/color.h>
#include <gtkmm/eventbox.h>

#include <sstream>
#include <fstream>
#include <iostream>

// #include <boost/serialization/utility.hpp>

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


using namespace Linkage;

static Glib::ustring week_days[] = 
{
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
	"Sunday"
};



ScheduleWin::ScheduleWin(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& xml)
	: Gtk::Dialog(cobject), glade_xml(xml)
{
	Gtk::Image* img;
	Gtk::EventBox* ebox;
	Gtk::Table *rtbl, *ltbl;
	
	m_button_pressed = -1;
	
	m_tree_model = Gtk::ListStore::create(m_columns);
	
	glade_xml->get_widget("time_label", m_time_label);
	glade_xml->get_widget("combo_schedules", m_schedule_combo);
	
	glade_xml->get_widget("schedule_table", m_schedule_table);
	glade_xml->get_widget("left_click_table", ltbl);
	glade_xml->get_widget("right_click_table", rtbl);
	
	glade_xml->connect_clicked
		("btn_new_sched", sigc::mem_fun(this, &ScheduleWin::on_new));
	glade_xml->connect_clicked
		("btn_delete_sched", sigc::mem_fun(this, &ScheduleWin::on_delete));
	glade_xml->connect_clicked
		("btn_reset", sigc::mem_fun(this, &ScheduleWin::on_reset));
	
	m_pixbufs[LIMIT_NORMAL] = Gdk::Pixbuf::create_from_file(DATA_DIR "/normal.png");
	m_pixbufs[LIMIT_LIMIT1] = Gdk::Pixbuf::create_from_file(DATA_DIR "/limit1.png");
	m_pixbufs[LIMIT_LIMIT2] = Gdk::Pixbuf::create_from_file(DATA_DIR "/limit2.png");
	m_pixbufs[LIMIT_LIMIT3] = Gdk::Pixbuf::create_from_file(DATA_DIR "/limit3.png");
	m_pixbufs[LIMIT_BLOCKED] = Gdk::Pixbuf::create_from_file(DATA_DIR "/blocked.png");
	
	// attach signals to the right/left click radio buttons
	// and populate the images
	for (int c = 0; c < TOTAL_LIMITS; c++)
	{
		std::ostringstream ssr, ssl;
		ssr << "sel_right_click_" << c+1;
		ssl << "sel_left_click_" << c+1;
		
		//connect left signal and image
		glade_xml->get_widget(ssl.str(), m_radio_btns[0][c]);
		m_radio_btns[0][c]->signal_toggled().connect(
			sigc::bind(sigc::mem_fun(this, &ScheduleWin::on_choose_lclick), (ScheduleLimit)c));
		img = manage(new Gtk::Image(m_pixbufs[c]));
		img->show();
		rtbl->attach(*img, 0, 1, c, c+1, 
			Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
			
		//connect right signal and image
		glade_xml->get_widget(ssr.str(), m_radio_btns[1][c]);
		m_radio_btns[1][c]->signal_toggled().connect(
			sigc::bind(sigc::mem_fun(this, &ScheduleWin::on_choose_rclick), (ScheduleLimit)c));
		img = manage(new Gtk::Image(m_pixbufs[c]));
		img->show();
		ltbl->attach(*img, 0, 1, c, c+1, 
			Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);
	}	
	
	// attach signals to the spin buttons
	for (int i = 0; i < 3; i++)
	{
		std::ostringstream ssu, ssd;
		std::pair<int, bool> args;
		ssu << "spb_limit" << i+1 << "_up";
		ssd << "spb_limit" << i+1 << "_down";
		// upload
		glade_xml->get_widget(ssu.str(), m_spin_btns[1][i]);
		m_spin_btns[1][i]->signal_value_changed().connect(
			sigc::bind(sigc::mem_fun(this, &ScheduleWin::on_set_limit), std::make_pair(i, true)));
		// download
		glade_xml->get_widget(ssd.str(), m_spin_btns[0][i]);
		m_spin_btns[0][i]->signal_value_changed().connect(
			sigc::bind(sigc::mem_fun(this, &ScheduleWin::on_set_limit), std::make_pair(i, false)));
	}
	
	// setup the combobox
	m_schedule_combo->signal_changed().connect(
		sigc::mem_fun(this, &ScheduleWin::on_change_schedule));
	m_schedule_combo->set_model(m_tree_model);
	m_schedule_combo->pack_start(m_columns.name);
	
	// build the schedule table
	for (int col=0; col < 7; col++)
	{
		for (int row=1; row < 25; row++)
		{
			ebox = manage(new Gtk::EventBox);
			img = manage(new Gtk::Image(m_pixbufs[LIMIT_NORMAL]));
			img->show();
			ebox->add(*img);
			
			m_schedule_table->attach(
				*ebox, col, col+1, row, row+1,
				Gtk::FILL|Gtk::EXPAND, (Gtk::AttachOptions)0,0,0);

			TimeSlot slot(col, row-1);
			ebox->signal_enter_notify_event().connect(
				sigc::bind(sigc::mem_fun(this, &ScheduleWin::on_timeslot_enter), slot));
			ebox->signal_leave_notify_event().connect(
				sigc::bind(sigc::mem_fun(this, &ScheduleWin::on_timeslot_leave), slot));
			ebox->signal_button_press_event().connect(
				sigc::bind(sigc::mem_fun(this, &ScheduleWin::on_button_press), slot));
			ebox->signal_button_release_event().connect(
				sigc::bind(sigc::mem_fun(this, &ScheduleWin::on_button_release), slot));
			
			m_images[col][row-1] = img;
			ebox->show();
		}
	}
	
}

ScheduleWin::~ScheduleWin()
{}

void ScheduleWin::configure(SchedulerConfig cfg)
{
	m_config = cfg;
	populate_combobox();
	m_schedule_combo->set_active(m_config.current_schedule);

	load_schedule(m_config.get_current_schedule());
}

void ScheduleWin::set_time_slot(TimeSlot slot, int limit)
{
	m_images[slot.first][slot.second]->set(m_pixbufs[limit]);
	m_config.get_current_schedule().schedule[slot.first][slot.second] = limit;
}


bool ScheduleWin::on_button_press(GdkEventButton* event, TimeSlot slot)
{
	switch (event->button)
	{
		case 1:
			m_button_pressed = 1;
			set_time_slot(slot, m_config.get_current_schedule().lclick);
			break;
		case 3:
			m_button_pressed = 3;
			set_time_slot(slot, m_config.get_current_schedule().rclick);
			break;
		default:
			m_button_pressed = -1;
			return true;
	}
}

bool ScheduleWin::on_button_release(GdkEventButton* event, TimeSlot slot)
{
	m_button_pressed = -1;
}
	
bool ScheduleWin::on_timeslot_enter(GdkEventCrossing* event, TimeSlot slot)
{
	switch (m_button_pressed)
	{
		case 1:
			set_time_slot(slot, m_config.get_current_schedule().lclick);
			break;
		case 3:
			set_time_slot(slot, m_config.get_current_schedule().rclick);
			break;
		default:
			break;
	}
	
	// update the label
	std::ostringstream sstr;
	sstr << week_days[slot.first] << " ";
	if (slot.second < 10)
		sstr << "0";
	sstr << slot.second << ":00 - ";
	if (slot.second+1 < 10)
		sstr << "0";
	sstr << slot.second+1 << ":00h";
	m_time_label->set_text(sstr.str());
}

bool ScheduleWin::on_timeslot_leave(GdkEventCrossing* event, TimeSlot slot)
{
	m_time_label->set_text("");
}

void ScheduleWin::load_schedule(const Schedule& sched)
{
	// set the images in the table
	for (int col=0; col < 7; col++)
	{
		for (int row=1; row < 25; row++)
		{
			int img = sched.schedule[col][row-1];
			if (img < 0 || img > TOTAL_LIMITS-1)
				img = 0;
			m_images[col][row-1]->set(m_pixbufs[img]);
		}
	}
	
	// set rclick/lclick
	m_radio_btns[0][sched.lclick]->set_active(true);
	m_radio_btns[1][sched.rclick]->set_active(true);
	
	// set limits
	for (int i = 0; i < 3; i++)
	{
		m_spin_btns[0][i]->set_value((double)sched.limits[i].down);
		m_spin_btns[1][i]->set_value((double)sched.limits[i].up);
	}
}

// called when a spin button is set
// int is limit# (0,1 or 2) and bool true means up_speed
void ScheduleWin::on_set_limit(std::pair<int, bool> limup)
{
	if (limup.second)
		m_config.get_current_schedule().limits[limup.first].up;
	else
		m_config.get_current_schedule().limits[limup.first].down;
}

// called when the combo box is changed
void ScheduleWin::on_change_schedule()
{
	Gtk::TreeModel::Row row = *(m_schedule_combo->get_active());
	m_config.current_schedule = row[m_columns.id];
	load_schedule(m_config.schedules[row[m_columns.id]]);
}

void ScheduleWin::on_new()
{
	// show dialog to get name
	NameDialog* ndiag;
	glade_xml->get_widget_derived("name_dialog", ndiag);
	
	int res = ndiag->run();
	if (res == Gtk::RESPONSE_OK)
		m_config.schedules.push_back(Schedule(ndiag->get_schedule_name()));
	ndiag->hide();
	populate_combobox();
	m_schedule_combo->set_active(m_config.schedules.size()-1);
}
void ScheduleWin::on_delete()
{
	// delete the currently selected schedule
	if (m_config.schedules.size() > 1)
	{
		int i = m_config.current_schedule;
		m_config.schedules.erase(m_config.schedules.begin()+i, m_config.schedules.begin()+i+1);
		populate_combobox();
		m_schedule_combo->set_active(0);
	}
}
void ScheduleWin::on_reset()
{
	// reset each image
	for (int col=0; col < 7; col++)
	{
		for (int row=1; row < 25; row++)
		{
			m_images[col][row-1]->set(m_pixbufs[LIMIT_NORMAL]);
		}
	}
}

void ScheduleWin::populate_combobox()
{
	m_tree_model->clear();
	for (int i = 0; i < m_config.schedules.size(); i++)
	{
		Gtk::TreeModel::Row row = *(m_tree_model->append());
		row[m_columns.id] = i;
		row[m_columns.name] = m_config.schedules[i].name;
	}
}
