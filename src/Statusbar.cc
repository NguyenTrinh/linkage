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

#include <gtkmm/frame.h>
#include <gtkmm/box.h>
#include <gtkmm/image.h>
#include <gtkmm/stock.h>

#include <glibmm/i18n.h>

#include "linkage/Engine.hh"
#include "linkage/SessionManager.hh"
#include "linkage/TorrentManager.hh"
#include "linkage/AlertManager.hh"
#include "linkage/Utils.hh"

#include "Statusbar.hh"

Statusbar::Statusbar(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::Statusbar(cobject)
{
	Gtk::Frame* frame = manage(new Gtk::Frame());
	Gtk::HBox* box = manage(new Gtk::HBox());

	Gtk::Image* image = NULL;

	m_upload = manage(new Gtk::Label());
	box->pack_end(*m_upload, false, false);
	image = manage(new Gtk::Image(Gtk::Stock::GO_UP, Gtk::ICON_SIZE_MENU));
	box->pack_end(*image, false, false, 5);
	
	m_download = manage(new Gtk::Label());
	box->pack_end(*m_download, false, false);
	image = manage(new Gtk::Image(Gtk::Stock::GO_DOWN, Gtk::ICON_SIZE_MENU));
	box->pack_end(*image, false, false, 5);
	
	m_connections = manage(new Gtk::Label());
	box->pack_end(*m_connections, false, false);
	image = manage(new Gtk::Image(Gtk::Stock::NETWORK, Gtk::ICON_SIZE_MENU));
	box->pack_end(*image, false, false, 5);

	frame->add(*box);
	frame->set_shadow_type(Gtk::SHADOW_IN);
	pack_end(*frame, false, false);

	Engine::get_session_manager()->signal_invalid_bencoding().connect(sigc::mem_fun(this, &Statusbar::on_invalid_bencoding));
	Engine::get_session_manager()->signal_missing_file().connect(sigc::mem_fun(this, &Statusbar::on_missing_file));
	Engine::get_session_manager()->signal_duplicate_torrent().connect(sigc::mem_fun(this, &Statusbar::on_duplicate_torrent));

	Engine::get_alert_manager()->signal_listen_failed().connect(sigc::mem_fun(this, &Statusbar::on_listen_failed));
	Engine::get_alert_manager()->signal_tracker_failed().connect(sigc::mem_fun(this, &Statusbar::on_tracker_failed));
	Engine::get_alert_manager()->signal_tracker_reply().connect(sigc::mem_fun(this, &Statusbar::on_tracker_reply));
	Engine::get_alert_manager()->signal_tracker_warning().connect(sigc::mem_fun(this, &Statusbar::on_tracker_warning));
	Engine::get_alert_manager()->signal_tracker_announce().connect(sigc::mem_fun(this, &Statusbar::on_tracker_announce));
	Engine::get_alert_manager()->signal_torrent_finished().connect(sigc::mem_fun(this, &Statusbar::on_torrent_finished));
	Engine::get_alert_manager()->signal_file_error().connect(sigc::mem_fun(this, &Statusbar::on_file_error));
	Engine::get_alert_manager()->signal_fastresume_rejected().connect(sigc::mem_fun(this, &Statusbar::on_fastresume_rejected));
	Engine::get_alert_manager()->signal_hash_failed().connect(sigc::mem_fun(this, &Statusbar::on_hash_failed));
	Engine::get_alert_manager()->signal_peer_ban().connect(sigc::mem_fun(this, &Statusbar::on_peer_ban));
}

Statusbar::~Statusbar()
{
}

void Statusbar::post(const Glib::ustring& msg)
{
	pop();
	push(msg);
}

void Statusbar::on_invalid_bencoding(const Glib::ustring& msg, const Glib::ustring& file)
{
	post(msg);
}

void Statusbar::on_missing_file(const Glib::ustring& msg, const Glib::ustring& file)
{
	post(msg);
}

void Statusbar::on_duplicate_torrent(const Glib::ustring& msg, const libtorrent::sha1_hash& hash)
{
	post(msg);
}

void Statusbar::on_listen_failed(const Glib::ustring& msg)
{
	post(msg);
}

void Statusbar::on_tracker_failed(const libtorrent::sha1_hash& hash, const Glib::ustring& msg, int code, int times)
{
	post(msg);
}

void Statusbar::on_tracker_reply(const libtorrent::sha1_hash& hash, const Glib::ustring& msg, int peers)
{
	post(msg);
}

void Statusbar::on_tracker_warning(const libtorrent::sha1_hash& hash, const Glib::ustring& msg)
{
	post(msg);
}

void Statusbar::on_tracker_announce(const libtorrent::sha1_hash& hash, const Glib::ustring& msg)
{
	post(msg);
}

void Statusbar::on_torrent_finished(const libtorrent::sha1_hash& hash, const Glib::ustring& msg)
{
	Glib::RefPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
	post(torrent->get_name() + " is_complete");
}

void Statusbar::on_file_error(const libtorrent::sha1_hash& hash, const Glib::ustring& msg)
{
	post(msg);
}

void Statusbar::on_fastresume_rejected(const libtorrent::sha1_hash& hash, const Glib::ustring& msg)
{
	post(msg);
}

void Statusbar::on_hash_failed(const libtorrent::sha1_hash& hash, const Glib::ustring& msg, int piece)
{
	post(msg);
}

void Statusbar::on_peer_ban(const libtorrent::sha1_hash& hash, const Glib::ustring& msg, const Glib::ustring& ip)
{
	post(msg);
}

void Statusbar::set_status(int connections, float down_rate, float up_rate)
{
	m_connections->set_text(String::ucompose("%1", connections));
	m_download->set_text(suffix_value(down_rate) + "/s");
	m_upload->set_text(suffix_value(up_rate) + "/s");
}

