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

#include <glibmm/main.h>

#include "ExpanderButton.hh"

ExpanderButton::ExpanderButton() : 
	Glib::ObjectBase("ExpanderButton"),
  Gtk::DrawingArea()
{
	m_expanded = false;
	child = NULL;
	style = Gtk::EXPANDER_COLLAPSED;
	state = Gtk::STATE_NORMAL;
	add_events(Gdk::ENTER_NOTIFY_MASK | Gdk::LEAVE_NOTIFY_MASK | Gdk::BUTTON_PRESS_MASK);

	static bool property_installed;
	if (!property_installed)
	{
		property_installed = true;
		/* http://cvs.gnome.org/viewcvs/gtkmm/examples/book/custom/custom_widget/mywidget.cc */
		gtk_widget_class_install_style_property(GTK_WIDGET_CLASS(G_OBJECT_GET_CLASS(gobj())), 
  	   g_param_spec_int("expander-size",
			      						"Expander Size",
      	                "Size of the expander arrow",
        	              0,
		      							G_MAXINT,
		      							16,
 		      							G_PARAM_READABLE));
	}
}

ExpanderButton::~ExpanderButton()
{
}

sigc::signal<void> ExpanderButton::signal_clicked()
{
	return m_signal_clicked;
}

bool ExpanderButton::on_button_press_event(GdkEventButton* event)
{
	if (state == Gtk::STATE_INSENSITIVE)
		return true;

	m_expanded = !m_expanded;
	state = Gtk::STATE_ACTIVE; 
	animate();

	if (m_expanded && child)
		child->show();
	else if (child)
		child->hide();

	m_signal_clicked.emit();

	return true;
}

bool ExpanderButton::on_enter_notify_event(GdkEventCrossing* event)
{
	if (state != Gtk::STATE_INSENSITIVE)
	{
		state = Gtk::STATE_PRELIGHT;
		draw();
	}
	return true;
}

bool ExpanderButton::on_leave_notify_event(GdkEventCrossing* event)
{
	if (state != Gtk::STATE_INSENSITIVE)
	{
		state = Gtk::STATE_NORMAL;
		draw();
	}
	return true;
}

void ExpanderButton::on_realize()
{
	Gtk::DrawingArea::on_realize();
	
	get_window()->clear();
}

bool ExpanderButton::on_expose_event(GdkEventExpose*)
{
	draw();
	
	return true;
}

void ExpanderButton::draw()
{
	set_size_request(16, 16);
	if (!is_drawable())
		return;

	get_window()->clear();

	unsigned int w = get_allocation().get_width();
	unsigned int h = get_allocation().get_height();
	Glib::RefPtr<Gdk::Window> window = get_window();
	Gdk::Rectangle rect = Gdk::Rectangle(0, 0, w, h);
	Gtk::Widget* widget = dynamic_cast<Gtk::Widget*>(this);
	get_style()->paint_expander(window, state, rect, *widget, "expander", 8, 16, style);
}

bool ExpanderButton::animate()
{
	if (style == Gtk::EXPANDER_SEMI_EXPANDED || style == Gtk::EXPANDER_SEMI_COLLAPSED)
	{
		style = m_expanded ? Gtk::EXPANDER_EXPANDED : Gtk::EXPANDER_COLLAPSED;
	}
	else
	{
		style = m_expanded ? Gtk::EXPANDER_SEMI_EXPANDED : Gtk::EXPANDER_SEMI_COLLAPSED;
		Glib::signal_timeout().connect(sigc::mem_fun(this, &ExpanderButton
::animate), 50);
	}

	draw();
	return (style == Gtk::EXPANDER_SEMI_EXPANDED || style == Gtk::EXPANDER_SEMI_COLLAPSED);
}

void ExpanderButton::set_expanded(bool expanded)
{
	m_expanded = expanded;

	animate();

	if (m_expanded && child)
		child->show();
	else if (child)
		child->hide();
}

bool ExpanderButton::get_expanded()
{
	return m_expanded;
}

void ExpanderButton::add(Gtk::Widget* widget)
{
	if (!widget)
		return;

	if (child)
		delete child;

	child = widget;

	if (m_expanded && child)
		child->show();
	else if (child)
		child->hide();
}

void ExpanderButton::set_sensitive(bool sensitive)
{
	state = sensitive ? Gtk::STATE_NORMAL : Gtk::STATE_INSENSITIVE;
}
