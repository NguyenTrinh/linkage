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

#ifndef TORRENT_HH
#define TORRENT_HH

#define INVALID_HASH 0

#include <vector>

#include <glibmm/object.h>
#include <glibmm/ustring.h>
#include <glibmm/property.h>
#include <glibmm/propertyproxy.h>
#include <sigc++/signal.h>

#include "libtorrent/torrent.hpp"

typedef std::list<libtorrent::sha1_hash> HashList;

class Torrent : public Glib::Object
{
public:
	enum State
	{
		NONE = 0x0,
		ANNOUNCING = 0x1,
		DOWNLOADING = 0x2,
		FINISHED = 0x4,
		SEEDING = 0x8,
		CHECK_QUEUE = 0x10,
		CHECKING = 0x20,
		ALLOCATING = 0x40,
		STOPPED = 0x80,
		QUEUED = 0x100,
		ERROR = 0x200
	};

	struct ResumeInfo
	{
		libtorrent::entry resume;
		libtorrent::torrent_info info;
		ResumeInfo(const libtorrent::entry& e, const libtorrent::torrent_info& i) : resume(e), info(i) {}
	};

	Glib::PropertyProxy<libtorrent::torrent_handle> property_handle();
	libtorrent::torrent_handle get_handle();
	const std::pair<Glib::ustring, Glib::ustring> get_tracker_reply();
	const Glib::ustring get_name();
	const Glib::ustring& get_group();
	const Glib::ustring& get_path();
	const unsigned int get_position();
	//const std::vector<int>& get_priorities();
	const std::vector<bool>& get_filter();
	const int get_up_limit();
	const int get_down_limit();
	const libtorrent::sha1_hash get_hash();

	const libtorrent::size_type get_total_downloaded();
	const libtorrent::size_type get_total_uploaded();

	bool get_completed();

	const State get_state();
	const Glib::ustring get_state_string();
	const Glib::ustring get_state_string(State state);

	const libtorrent::torrent_info& get_info();
	const libtorrent::torrent_status get_status();
	const std::vector<libtorrent::partial_piece_info> get_download_queue();
	const std::vector<float> get_file_progress();

	void set_tracker_reply(const Glib::ustring& reply, const Glib::ustring& tracker = Glib::ustring());
	void set_group(const Glib::ustring& group);
	void set_path(const Glib::ustring& path);
	void set_position(unsigned int position);
	//void set_priorities(const std::vector<int>& p);
	void set_filter(const std::vector<bool>& filter);
	void filter_file(unsigned int index, bool filter = true);
	void set_up_limit(int limit);
	void set_down_limit(int limit);

	/* This is safe to have public since this isn't the same data the tracker gets */
	void set_total_downloaded(libtorrent::size_type bytes);
	void set_total_uploaded(libtorrent::size_type bytes);

	void set_completed(bool completed = true);

	void add_tracker(const Glib::ustring& url);

	void queue();
	void unqueue();
	bool is_queued();
	bool is_stopped();

	void reannounce(const Glib::ustring& tracker = Glib::ustring());
	const std::vector<libtorrent::announce_entry>& get_trackers();

	const libtorrent::entry get_resume_entry(bool running = false);

	/* FIXME: Friend access only? */
	void set_handle(const libtorrent::torrent_handle& handle);

	Torrent(const ResumeInfo& ri, bool queued = false);
	Torrent();
	~Torrent();

private:
	libtorrent::size_type m_uploaded;
	libtorrent::size_type m_downloaded;

	Glib::Property<libtorrent::torrent_handle> m_prop_handle;
	libtorrent::torrent_info m_info;

	typedef std::map<Glib::ustring, Glib::ustring> ReplyMap;
	ReplyMap m_replies;
	int m_cur_tier;
	bool m_announcing;

	//std::vector<int> m_priorities;
	std::vector<bool> m_filter;

	bool m_is_queued;

	unsigned int m_position;
	Glib::ustring m_group, m_path;
	int m_up_limit, m_down_limit;

	bool m_completed;

	std::vector<libtorrent::announce_entry> m_trackers;
};

#endif /* TORRENT_HH */
