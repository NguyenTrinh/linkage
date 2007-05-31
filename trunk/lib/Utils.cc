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

#include <fstream>

#include "libtorrent/hasher.hpp"
#include "libtorrent/bencode.hpp"

#include "linkage/Utils.hh"

Glib::ustring suffix_value(float value)
{
	return suffix_value((size_type)value);
}

Glib::ustring suffix_value(size_type value)
{
	std::stringstream tc;
	if (value >= 1073741824)
		tc << std::fixed << std::setprecision(2) << value/1073741824.0 << " GB";
	else if (value >= 1048576)
		tc << std::fixed << std::setprecision(2) << value/1048576.0 << " MB";
	else
		tc << std::fixed << std::setprecision(2) << value/1024.0 << " kB";
	return tc.str();
}

Glib::ustring str(size_t value)
{
	std::stringstream tc;
	tc << value;
	return tc.str();
}

#if SIZEOF_UNSIGNED_INT != SIZEOF_SIZE_T
Glib::ustring str(unsigned int value)
{
	std::stringstream tc;
	tc << value;
	return tc.str();
}
#endif

Glib::ustring str(int value)
{
	std::stringstream tc;
	tc << value;
	return tc.str();
}

Glib::ustring str(size_type value)
{
	std::stringstream tc;
	tc << value;
	return tc.str();
}

Glib::ustring str(float value, int precision)
{
	std::stringstream tc;
	tc << std::fixed << std::setprecision(precision) << value;
	return tc.str();
}

Glib::ustring str(const sha1_hash& hash)
{
	std::stringstream tc;
	tc << hash;
	return tc.str();
}


Glib::ustring get_eta(size_type size, float rate)
{
	if (!rate || !size)
		return "\u221E";
	else if (rate < 0 || size < 0)
		return "";

	size_type seconds = (size_type)round(size/rate);
	return format_time(seconds);
}

Glib::ustring format_time(size_type seconds)
{
	long long int days, hours, minutes;
	std::lldiv_t div;

	div = std::div((long long int)seconds, 60ll);
	minutes = div.quot;
	seconds = div.rem;
	div = std::div(minutes, 60ll);
	hours = div.quot;
	minutes = div.rem;
	div = std::div(hours, 24ll);
	days = div.quot;
	hours = div.rem;

	std::stringstream tc;
	if (days==0)
	{
		tc << std::setw(2) << std::setfill('0') << hours << ":";
		tc << std::setw(2) << std::setfill('0') << minutes << ":";
		tc << std::setw(2) << std::setfill('0') << seconds;
		return tc.str();
	}
	else
	{
		Glib::ustring day_str;
		if (days == 1)
			day_str = "day";
		else
			 day_str = "days";
		tc << days << day_str << ", " << std::setw(2) << std::setfill('0') << hours << ":";
		tc << std::setw(2) << std::setfill('0') << minutes << ":" ;
		tc << std::setw(2) << std::setfill('0') << seconds;
		return tc.str();
	}
}

sha1_hash info_hash(const std::string& chars)
{
	sha1_hash hash;

	for (int i = 0; i < hash.size && (hash.size == chars.size()); i++)
		hash[i] = chars[i];

	return hash;
}

/* http://lists.alioth.debian.org/pipermail/pkg-gnome-maintainers/2005-June/014881.html */
std::list<Glib::ustring> get_interfaces()
{
	std::list<Glib::ustring> list;

	struct if_nameindex *ifs = if_nameindex();
	if (ifs == NULL)
		return list;

	for (int i = 0; (ifs[i].if_index != 0) || (ifs[i].if_name != NULL); i++)
	{
		ip_address ip;
		if (get_ip(ifs[i].if_name, ip))
			list.push_back(ifs[i].if_name);
	}
	if_freenameindex(ifs);

	return list;
}

/* http://www.linuxquestions.org/questions/showthread.php?t=425637 */
bool get_ip(const char *iface, ip_address ip)
{
	struct	ifreq				*ifr;
	struct	ifreq				ifrr;
	struct	sockaddr_in	sa;
	struct	sockaddr		ifaddr;
	int			sockfd;

	if((sockfd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))==-1)
	 return false;

	ifr = &ifrr;

	ifrr.ifr_addr.sa_family = AF_INET;

	strncpy(ifrr.ifr_name, iface, sizeof(ifrr.ifr_name));

	if (ioctl(sockfd, SIOCGIFADDR, ifr) < 0)
		return false;

	ifaddr = ifrr.ifr_addr;
	strncpy(ip,inet_ntoa(inaddrr(ifr_addr.sa_data)),sizeof(ip_address));
	return true;
}

Glib::ustring get_config_dir()
{
	return Glib::build_filename(g_get_user_config_dir(), "linkage");
}

Glib::ustring get_data_dir()
{
	return Glib::build_filename(get_config_dir(), "data");
}

void save_entry(const sha1_hash& hash, const entry& e, const Glib::ustring& suffix)
{
	save_entry(Glib::build_filename(get_data_dir(), str(hash)) + suffix, e);
}

void save_entry(const Glib::ustring& file, const entry& e)
{
	std::ofstream out(file.c_str(), std::ios_base::binary);
	out.unsetf(std::ios_base::skipws);
	bencode(std::ostream_iterator<char>(out), e);
	out.close();
}
