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

#ifndef TORRENTLISTRENDERERS_HH
#define TORRENTLISTRENDERERS_HH

#include <gtkmm/cellrenderertext.h>

#include "linkage/Torrent.hh"

class HashRenderer : public Gtk::CellRendererText
{
	Glib::Property<libtorrent::sha1_hash> m_prop_hash;
protected:
	void render_vfunc(const Glib::RefPtr<Gdk::Drawable>& window,
						Gtk::Widget& widget,
						const Gdk::Rectangle&,
						const Gdk::Rectangle& cell_area,
						const Gdk::Rectangle&,
						Gtk::CellRendererState flags);

public:
	Glib::PropertyProxy<libtorrent::sha1_hash> property_hash() { return m_prop_hash.get_proxy(); };
	
	HashRenderer();
	~HashRenderer()  {}
};


class SuffixRenderer : public Gtk::CellRendererText
{
	Glib::Property<libtorrent::size_type> m_prop_fsize;
	Glib::Property<float> m_prop_speed;
	bool m_speed;
protected:
	void render_vfunc(const Glib::RefPtr<Gdk::Drawable>& window,
						Gtk::Widget& widget,
						const Gdk::Rectangle&,
						const Gdk::Rectangle& cell_area,
						const Gdk::Rectangle&,
						Gtk::CellRendererState flags);

public:
	Glib::PropertyProxy<libtorrent::size_type> property_fsize() { return m_prop_fsize.get_proxy(); };
	Glib::PropertyProxy<float> property_speed() { return m_prop_speed.get_proxy(); };
	
	SuffixRenderer(bool speed = false);
	~SuffixRenderer()  {}
};

class EtaRenderer : public Gtk::CellRendererText
{
	Glib::Property<libtorrent::size_type> m_prop_time;
protected:
	void render_vfunc(const Glib::RefPtr<Gdk::Drawable>& window,
						Gtk::Widget& widget,
						const Gdk::Rectangle&,
						const Gdk::Rectangle& cell_area,
						const Gdk::Rectangle&,
						Gtk::CellRendererState flags);

public:
	Glib::PropertyProxy<libtorrent::size_type> property_time() { return m_prop_time.get_proxy(); };
	
	EtaRenderer();
	~EtaRenderer()  {}
};

class PeerRenderer : public Gtk::CellRendererText
{
	Glib::Property<int> m_prop_connected;
	Glib::Property<int> m_prop_total;
protected:
	void render_vfunc(const Glib::RefPtr<Gdk::Drawable>& window,
						Gtk::Widget& widget,
						const Gdk::Rectangle&,
						const Gdk::Rectangle& cell_area,
						const Gdk::Rectangle&,
						Gtk::CellRendererState flags);

public:
	Glib::PropertyProxy<int> property_connected() { return m_prop_connected.get_proxy(); };
	Glib::PropertyProxy<int> property_total() { return m_prop_total.get_proxy(); };
	
	PeerRenderer();
	~PeerRenderer()  {}
};

#endif //TORRENTLISTRENDERERS_HH
