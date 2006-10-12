/*
Copyright (C) 2006  Christian Lundgren

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <sigc++/signal.h>
#include "libtorrent/peer_id.hpp"
#include <glibmm/ustring.h>

using namespace libtorrent;

class AlertManager
{
  sigc::signal<void, const Glib::ustring&> signal_listen_failed_;
  sigc::signal<void, const sha1_hash&, const Glib::ustring&, int, int> signal_tracker_failed_;
  sigc::signal<void, const sha1_hash&, const Glib::ustring&> signal_tracker_reply_;
  sigc::signal<void, const sha1_hash&, const Glib::ustring&> signal_tracker_warning_;
  sigc::signal<void, const sha1_hash&, const Glib::ustring&> signal_tracker_announce_;
  sigc::signal<void, const sha1_hash&, const Glib::ustring&> signal_torrent_finished_;
  sigc::signal<void, const sha1_hash&, const Glib::ustring&> signal_file_error_;
  sigc::signal<void, const sha1_hash&, const Glib::ustring&> signal_fastresume_rejected_;
  sigc::signal<void, const sha1_hash&, const Glib::ustring&, int> signal_hash_failed_;
  sigc::signal<void, const sha1_hash&, const Glib::ustring&, const Glib::ustring&> signal_peer_ban_;
  
  bool check_alerts();
  
  static AlertManager* smInstance;

public:
  sigc::signal<void, const Glib::ustring&> signal_listen_failed();
  sigc::signal<void, const sha1_hash&, const Glib::ustring&, int, int> signal_tracker_failed();
  sigc::signal<void, const sha1_hash&, const Glib::ustring&> signal_tracker_reply();
  sigc::signal<void, const sha1_hash&, const Glib::ustring&> signal_tracker_warning();
  sigc::signal<void, const sha1_hash&, const Glib::ustring&> signal_tracker_announce();
  sigc::signal<void, const sha1_hash&, const Glib::ustring&> signal_torrent_finished();
  sigc::signal<void, const sha1_hash&, const Glib::ustring&> signal_file_error();
  sigc::signal<void, const sha1_hash&, const Glib::ustring&> signal_fastresume_rejected();
  sigc::signal<void, const sha1_hash&, const Glib::ustring&, int> signal_hash_failed();
  sigc::signal<void, const sha1_hash&, const Glib::ustring&, const Glib::ustring&> signal_peer_ban();
  
  static AlertManager* instance();
  static void goodnight();
  
  AlertManager();
  ~AlertManager();
};
