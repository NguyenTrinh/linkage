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

#define inaddrr(x) (*(struct in_addr *) &ifr->x[sizeof sa.sin_port])
#define IFRSIZE	 ((int)(size * sizeof (struct ifreq)))

#include <list>
#include <math.h>
#include <sstream>
#include <glibmm/ustring.h>
#include <glibmm/miscutils.h>

#include "libtorrent/peer_id.hpp"
#include "libtorrent/entry.hpp"

typedef char ip_address[15+1];

using namespace libtorrent;

Glib::ustring suffix_value(size_type value);
Glib::ustring suffix_value(float value);

Glib::ustring str(int value);
Glib::ustring str(unsigned int value);
Glib::ustring str(size_type value);
Glib::ustring str(double value, int precision);
Glib::ustring str(const sha1_hash& hash);
Glib::ustring get_eta(size_type size, float rate);
Glib::ustring format_time(size_type seconds);

/* Can _not_ use ustring for this! */
sha1_hash info_hash(const std::string& chars);

std::list<Glib::ustring> get_interfaces();

bool get_ip(const char *iface, ip_address ip);

Glib::ustring get_config_dir();
Glib::ustring get_data_dir();

void save_entry(const sha1_hash& hash, const entry& e, const Glib::ustring& suffix = Glib::ustring());
void save_entry(const Glib::ustring& file, const entry& e);

#endif /* UTILS_HH */
