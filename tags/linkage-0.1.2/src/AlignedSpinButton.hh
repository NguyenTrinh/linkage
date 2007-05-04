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

#ifndef ALIGNED_SPINBUTTON_HH
#define ALIGNED_SPINBUTTON_HH

#include <gtkmm/spinbutton.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/alignment.h>

class AlignedSpinButton : public Gtk::Alignment
{
	Gtk::Adjustment* adjustment;
	Gtk::SpinButton* spinbutton;
	
public:
	double get_value();
	void set_value(double value);
	void set_range(double min, double max);
	Glib::SignalProxy0<void> signal_value_changed();
	
	AlignedSpinButton(double min, double max);
	~AlignedSpinButton();
};

#endif /* ALIGNED_SPINBUTTON_HH */
