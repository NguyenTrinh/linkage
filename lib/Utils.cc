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

#include <glibmm/convert.h>
#include <glibmm/i18n.h>

#include "libtorrent/hasher.hpp"
#include "libtorrent/bencode.hpp"

#include "linkage/Utils.hh"

Glib::ustring Linkage::suffix_value(float value)
{
	return suffix_value((libtorrent::size_type)value);
}

Glib::ustring Linkage::suffix_value(libtorrent::size_type value)
{
	std::stringstream ss;
	ss.imbue(std::locale(""));
	if (value >= 1073741824)
		ss << std::fixed << std::setprecision(2) << value/1073741824.0 << " " << _("GB");
	else if (value >= 1048576)
		ss << std::fixed << std::setprecision(2) << value/1048576.0 << " " << _("MB");
	else
		ss << std::fixed << std::setprecision(2) << value/1024.0 << " " << _("kB");
	return Glib::locale_to_utf8(ss.str());
}

Glib::ustring Linkage::get_eta(libtorrent::size_type size, float rate)
{
	if (!rate || !size)
		return "\u221E";
	else if (rate < 0 || size < 0)
		return Glib::ustring();

	libtorrent::size_type seconds = (libtorrent::size_type)round(size/rate);
	return format_time(seconds);
}

Glib::ustring Linkage::format_time(libtorrent::size_type seconds)
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

	std::stringstream ss;
	ss.imbue(std::locale(""));
	if (days == 0)
	{
		ss << std::setw(2) << std::setfill('0') << hours << ":"
			<< std::setw(2) << std::setfill('0') << minutes << ":"
			<< std::setw(2) << std::setfill('0') << seconds;
		return Glib::locale_to_utf8(ss.str());
	}
	else
	{
		// FIXME: this is not so good for translations
		Glib::ustring day_str;
		if (days == 1)
			day_str = _("day");
		else
			 day_str = _("days");
		ss << days << " " << day_str << ", " << std::setw(2)
			<< std::setfill('0') << hours << ":"
			<< std::setw(2) << std::setfill('0') << minutes << ":"
			<< std::setw(2) << std::setfill('0') << seconds;
		return Glib::locale_to_utf8(ss.str());
	}
}

libtorrent::sha1_hash Linkage::info_hash(const std::string& chars)
{
	libtorrent::sha1_hash hash;

	for (int i = 0; i < hash.size && (hash.size == chars.size()); i++)
		hash[i] = chars[i];

	return hash;
}

/* http://lists.alioth.debian.org/pipermail/pkg-gnome-maintainers/2005-June/014881.html */
std::list<Glib::ustring> Linkage::get_interfaces()
{
	std::list<Glib::ustring> list;

	struct if_nameindex *ifs = if_nameindex();
	if (ifs == NULL)
		return list;

	for (int i = 0; (ifs[i].if_index != 0) || (ifs[i].if_name != NULL); i++)
	{
		if (!get_ip(ifs[i].if_name).empty())
			list.push_back(ifs[i].if_name);
	}

	if_freenameindex(ifs);

	return list;
}

/* http://www.linuxquestions.org/questions/showthread.php?t=425637 */
Glib::ustring Linkage::get_ip(const Glib::ustring& iface)
{
	int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd == -1)
		return Glib::ustring();

	ifreq ifr;
	ifr.ifr_addr.sa_family = AF_INET;

	strncpy(ifr.ifr_name, iface.c_str(), sizeof(ifr.ifr_name));

	if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0)
		return Glib::ustring();

	sockaddr_in sa;

	const char* ip = inet_ntoa(*(in_addr*)&ifr.ifr_addr.sa_data[sizeof(sa.sin_port)]);

	close(sockfd);

	return ip;
}

Glib::ustring Linkage::get_config_dir()
{
	return Glib::build_filename(g_get_user_config_dir(), "linkage");
}

Glib::ustring Linkage::get_data_dir()
{
	return Glib::build_filename(get_config_dir(), "data");
}

bool Linkage::load_entry(const Glib::ustring& file, libtorrent::entry& e)
{
	bool ret = true;
	std::ifstream in(file.c_str(), std::ios_base::binary);
	in.unsetf(std::ios_base::skipws);
	try
	{
		e = libtorrent::bdecode(std::istream_iterator<char>(in),
			std::istream_iterator<char>());
	}
	catch (libtorrent::invalid_encoding& ex)
	{
		ret = false;
	}
	in.close();

	return ret;
}

void Linkage::save_entry(const Glib::ustring& file, const libtorrent::entry& e)
{
	std::ofstream out(file.c_str(), std::ios_base::binary);
	out.unsetf(std::ios_base::skipws);
	libtorrent::bencode(std::ostream_iterator<char>(out), e);
	out.close();
}

static void get_hsv(const Gdk::Color& color, double& h, double& s, double& v)
{
	// thanks to wikipedia, why doesn't GdkColor do this?

	gint r = color.get_red();
	gint g = color.get_green();
	gint b = color.get_blue();
	
	gint max = std::max(std::max(r, g), b);
	gint min = std::min(std::min(r, g), b);

	if (max == min)
		return;

	gint delta = max - min;

	// hue
	if (max == r)
	{
		if (g >= b)
			h = 60 * ((double)(g - b)/delta);
		else
			h = 60 * ((double)(g - b)/delta) + 360;
	}
	else if (max == g)
		h = 60 * ((double)(b - r)/delta) + 120;
	else if (max == b)
		h = 60 * ((double)(r - g)/delta) + 240;

	// saturation
	if (max > 0)
		s = 1 - (double)min/max;
	else
		s = 0;

	// value
	v = (double)(max)/G_MAXUSHORT;
}

// Derived from QColor::light, Copyright (C) 1992-2006 Trolltech ASA
Gdk::Color Linkage::lighter(const Gdk::Color& color, double fac)
{
	double hue = 0, sat = 0, val = 0;
	get_hsv(color, hue, sat,  val);

	val *= fac;
	if (val > 1)
	{
		sat -= val - 1;
		if (sat < 0)
			sat = 0;
		val = 1;
	}

	Gdk::Color c;
	c.set_hsv(hue, sat, val);
	return c;
}

Glib::ustring Linkage::hex_color(const Gdk::Color& color)
{
	char s[8];
	s[0] = '#';

	sprintf(&s[1], "%.2X", color.get_red() / 256);
	sprintf(&s[3], "%.2X", color.get_green() / 256);
	sprintf(&s[5], "%.2X", color.get_blue() / 256);

	return s;
}

