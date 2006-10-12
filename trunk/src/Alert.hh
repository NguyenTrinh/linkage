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

#include <glibmm/ustring.h>

#include "libtorrent/alert_types.hpp"

using namespace libtorrent;

class Alert
{
public:
	enum AlertType {
    ALERT_NONE,
    ALERT_ANY,
    ALERT_INVALID_BENCODING, 
    ALERT_FASTRESUME_FAILED,
    ALERT_MISSING_FILE,
    ALERT_DUPLICATE_TORRENT,
    ALERT_LISTEN_FAILED,
    ALERT_READWRITE_FAILED,
    ALERT_TRACKER_RESPONSE,
    ALERT_TRACKER_ANNOUNCE,
    ALERT_TRACKER_WARNING,
    ALERT_TRACKER_FAILED,
    ALERT_TORRENT_FINISHED,
    ALERT_FILEHASH_FAILED,
    ALERT_PEER_BAN
	};
	
  const Glib::ustring& what();
  bool get();
  AlertType code();
  
  const sha1_hash& hash();
  
  Alert();
  Alert(const Glib::ustring& msg, AlertType code);
  Alert(const Glib::ustring& msg, AlertType code, const sha1_hash& hash);
  ~Alert();

protected:
  Glib::ustring msg_;
  sha1_hash hash_;
  bool active;
  AlertType code_;
};
