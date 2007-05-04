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

#ifndef EXPANDER_BUTTON_HH
#define EXPANDER_BUTTON_HH

#include <gtkmm/drawingarea.h>

/* Gtk::Expander doesn't send press/enter events to it's child so we do this */
		
class ExpanderButton : public Gtk::DrawingArea
{
	Gtk::Widget* child;
	Gtk::ExpanderStyle style;
	Gtk::StateType state;
	bool m_expanded;

	sigc::signal<void> m_signal_clicked;

	void draw();
	bool animate();

	bool on_button_press_event(GdkEventButton* event);
	bool on_enter_notify_event(GdkEventCrossing* event);
	bool on_leave_notify_event(GdkEventCrossing* event);
	void on_realize();
	bool on_expose_event(GdkEventExpose* event);

public:
	void set_expanded(bool expanded = true);
	bool get_expanded();
	void set_sensitive(bool sensitive = true);
	void add(Gtk::Widget* widget);

	sigc::signal<void> signal_clicked();
	
	ExpanderButton();
	~ExpanderButton();
};

#endif /* EXPANDER_BUTTON_HH */
