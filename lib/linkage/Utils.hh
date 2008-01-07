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

#ifndef UTILS_HH
#define UTILS_HH

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <arpa/inet.h>
#include <net/if.h>

#include <list>
#include <math.h>
#include <sstream>
#include <string>
#include <glibmm/ustring.h>
#include <glibmm/miscutils.h>
#include <gdkmm/color.h>

#include "libtorrent/peer_id.hpp"
#include "libtorrent/entry.hpp"

#include "linkage/ucompose.hpp"

namespace Linkage
{

Glib::ustring suffix_value(libtorrent::size_type value);
Glib::ustring suffix_value(float value);

Glib::ustring get_eta(libtorrent::size_type size, float rate);
Glib::ustring format_time(libtorrent::size_type seconds);

/* Can _not_ use ustring for this! */
libtorrent::sha1_hash info_hash(const std::string& chars);

std::list<Glib::ustring> get_interfaces();

Glib::ustring get_ip(const Glib::ustring& iface);

std::string http_get(const std::string& url, int timeout = 30);

Glib::ustring get_config_dir();
Glib::ustring get_data_dir();

bool load_entry(const Glib::ustring& file, libtorrent::entry& e);
bool load_entry(const Glib::ustring& file, libtorrent::entry& e, std::vector<char>& buff);
void save_entry(const Glib::ustring& file, const libtorrent::entry& e);

Gdk::Color lighter(const Gdk::Color& color, double fac);

Glib::ustring hex_color(const Gdk::Color& color);

} /* namespace */

#endif /* UTILS_HH */
