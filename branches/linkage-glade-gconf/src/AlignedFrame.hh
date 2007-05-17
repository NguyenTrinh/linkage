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

#ifndef ALIGNED_FRAME_HH
#define ALIGNED_FRAME_HH

#include <gtkmm/label.h>
#include <gtkmm/alignment.h>
#include <gtkmm/frame.h>

class AlignedFrame : public Gtk::Frame
{
	Gtk::Alignment* m_alignment;
	Gtk::Label* m_label;
public:
	AlignedFrame();
	AlignedFrame(const Glib::ustring& text);

	void set_label(const Glib::ustring& text);
	void add(Gtk::Widget& widget);

	virtual ~AlignedFrame();
};

#endif /* ALIGNED_FRAME_HH */
