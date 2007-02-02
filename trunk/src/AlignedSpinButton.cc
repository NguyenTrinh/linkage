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

#include "AlignedSpinButton.hh"

AlignedSpinButton::AlignedSpinButton(double min, double max) :
	Gtk::Alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER, 0.0, 1.0)
{
	adjustment = new Gtk::Adjustment(0, min, max);
	spinbutton = new Gtk::SpinButton(*adjustment);
	spinbutton->set_numeric(true);
	spinbutton->set_update_policy(Gtk::UPDATE_IF_VALID);
	add(*spinbutton);
}

AlignedSpinButton::~AlignedSpinButton()
{
	delete adjustment;
	delete spinbutton;
}

double AlignedSpinButton::get_value()
{
	return spinbutton->get_value();
}

void AlignedSpinButton::set_value(double value)
{
	spinbutton->set_value(value);
}

void AlignedSpinButton::set_range(double min, double max)
{
	spinbutton->set_range(min, max);
}

Glib::SignalProxy0<void> AlignedSpinButton::signal_value_changed()
{
	return spinbutton->signal_value_changed();
}
