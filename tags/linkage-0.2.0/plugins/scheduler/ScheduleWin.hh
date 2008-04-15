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

#ifndef SCHEDULE_WIN_HH
#define SCHEDULE_WIN_HH

#include <libglademm/xml.h>
#include <gtkmm/dialog.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/liststore.h>
#include <gtkmm/combobox.h>
#include <gtkmm/table.h>
#include <gtkmm/image.h>
#include <gdkmm/pixbuf.h>

#include "SchedulerConfig.hh"

class NameDialog : public Gtk::Dialog
{
	Glib::RefPtr<Gnome::Glade::Xml> glade_xml;
	Gtk::Entry* m_name;
public:
	Glib::ustring get_schedule_name() { return m_name->get_text(); }
	
	NameDialog(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& xml)
		: Gtk::Dialog(cobject), glade_xml(xml) 
		{ glade_xml->get_widget("text_name", m_name); };
	~NameDialog() { std::cout << "NDIAG destructah\n"; }
};

class ScheduleWin : public Gtk::Dialog
{
	enum ScheduleLimit
	{
		LIMIT_NORMAL,
		LIMIT_LIMIT1,
		LIMIT_LIMIT2,
		LIMIT_LIMIT3,
		LIMIT_BLOCKED,
		TOTAL_LIMITS
	};
	class ModelColumns : public Gtk::TreeModel::ColumnRecord
	{
	public:
		ModelColumns()
		{ add(id); add(name); }

		Gtk::TreeModelColumn<int> id;
		Gtk::TreeModelColumn<Glib::ustring> name;
	};
	ModelColumns m_columns;
	Glib::RefPtr<Gtk::ListStore> m_tree_model;
	
	Glib::RefPtr<Gnome::Glade::Xml> glade_xml;
	
	Gtk::SpinButton* m_limit1_up;
	Gtk::SpinButton* m_limit1_down;
	Gtk::SpinButton* m_limit2_up;
	Gtk::SpinButton* m_limit2_down;
	Gtk::SpinButton* m_limit3_up;
	Gtk::SpinButton* m_limit3_down;
	
	Gtk::Table* m_schedule_table;
	Glib::RefPtr<Gdk::Pixbuf> m_pixbufs[5];
	Gtk::Image* m_images[7][24];

	Gtk::SpinButton* m_spin_btns[2][3];
	Gtk::RadioButton* m_radio_btns[2][TOTAL_LIMITS];
	
	Gtk::Label* m_time_label;
	Gtk::ComboBox* m_schedule_combo;
	
	SchedulerConfig m_config;
	int m_button_pressed;
	
	bool update_time_label(GdkEventCrossing* event, TimeSlot slot);
	bool on_schedule_click(GdkEventButton* event, TimeSlot slot);
	
	bool on_button_press(GdkEventButton* event, TimeSlot slot);
	bool on_button_release(GdkEventButton* event, TimeSlot slot);
		
	bool on_timeslot_enter(GdkEventCrossing* event, TimeSlot slot);
	bool on_timeslot_leave(GdkEventCrossing* event, TimeSlot slot);
	void set_time_slot(TimeSlot slot, int limit);
	
	// int is limit# (0,1 or 2) and bool true mean up speed
	void on_set_limit(std::pair<int, bool> limup);
	
	// called when the combo box is changed
	void on_change_schedule();
	
	void on_new();
	void on_delete();
	void on_reset();
	
	void on_choose_rclick(ScheduleLimit lim) 
	{ m_config.get_current_schedule().rclick = lim; }
	void on_choose_lclick(ScheduleLimit lim) 
	{ m_config.get_current_schedule().lclick  = lim; }

	void load_schedule(const Schedule& sched);
	void populate_combobox();
public:
	//int run();
	SchedulerConfig get_config() { return m_config; }
	void configure(SchedulerConfig cfg);
	ScheduleWin(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& xml);
	~ScheduleWin();
};



#endif //SCHEDULE_WIN_HH
