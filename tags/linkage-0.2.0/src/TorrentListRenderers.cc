/*
Copyright (C) 2008        Dave Moore
Copyright (C) 2008        Christian Lundgren

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

#include <sstream>

#include "TorrentListRenderers.hh"
#include "linkage/Utils.hh"

using namespace Linkage;

HashRenderer::HashRenderer() : 
		Glib::ObjectBase(typeid(HashRenderer)),
		Gtk::CellRendererText(),
		m_prop_hash(*this, "hash")
{}

void HashRenderer::render_vfunc(
	const Glib::RefPtr<Gdk::Drawable>& window,
	Gtk::Widget& widget,
	const Gdk::Rectangle& bg_area,
	const Gdk::Rectangle& cell_area,
	const Gdk::Rectangle& exp_area,
	Gtk::CellRendererState flags)
{
	std::ostringstream sstr;
	sstr << m_prop_hash;
	property_text() = sstr.str();
	Gtk::CellRendererText::render_vfunc(window, widget, bg_area, cell_area, exp_area, flags);
}


SuffixRenderer::SuffixRenderer(bool speed) : 
		Glib::ObjectBase(typeid(SuffixRenderer)),
		Gtk::CellRendererText(),
		m_prop_speed(*this, "speed"),
		m_prop_fsize(*this, "fsize"),
		m_speed(speed)
{}

void SuffixRenderer::render_vfunc(
	const Glib::RefPtr<Gdk::Drawable>& window,
	Gtk::Widget& widget,
	const Gdk::Rectangle& bg_area,
	const Gdk::Rectangle& cell_area,
	const Gdk::Rectangle& exp_area,
	Gtk::CellRendererState flags)
{	
	if (m_speed)
		property_text() = (suffix_value(m_prop_speed) + "/s");
	else
		property_text() = suffix_value(m_prop_fsize);
	Gtk::CellRendererText::render_vfunc(window, widget, bg_area, cell_area, exp_area, flags);
}

EtaRenderer::EtaRenderer() : 
		Glib::ObjectBase(typeid(EtaRenderer)),
		Gtk::CellRendererText(),
		m_prop_time(*this, "time")
{}

void EtaRenderer::render_vfunc(
	const Glib::RefPtr<Gdk::Drawable>& window,
	Gtk::Widget& widget,
	const Gdk::Rectangle& bg_area,
	const Gdk::Rectangle& cell_area,
	const Gdk::Rectangle& exp_area,
	Gtk::CellRendererState flags)
{	
	if (m_prop_time > 0) 
		property_text() = format_time(m_prop_time);
	else
		property_text() = "\u221E";
	Gtk::CellRendererText::render_vfunc(window, widget, bg_area, cell_area, exp_area, flags);
}

PeerRenderer::PeerRenderer() : 
		Glib::ObjectBase(typeid(PeerRenderer)),
		Gtk::CellRendererText(),
		m_prop_connected(*this, "connected"),
		m_prop_total(*this, "total")
{}

void PeerRenderer::render_vfunc(
	const Glib::RefPtr<Gdk::Drawable>& window,
	Gtk::Widget& widget,
	const Gdk::Rectangle& bg_area,
	const Gdk::Rectangle& cell_area,
	const Gdk::Rectangle& exp_area,
	Gtk::CellRendererState flags)
{	
	std::ostringstream sstr;
	sstr << m_prop_connected;
	if (m_prop_total != -1)
		sstr << "(" << m_prop_total << ")";
	property_text() = sstr.str();
	Gtk::CellRendererText::render_vfunc(window, widget, bg_area, cell_area, exp_area, flags);
}


