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

#include <glibmm/ustring.h>
#include <glibmm/timeval.h>
#include <sigc++/signal.h>

#include "libtorrent/torrent.hpp"

using namespace libtorrent;

typedef std::list<sha1_hash> HashList;

class Torrent
{
public:
	enum Direction { DIR_UP, DIR_DOWN };
	enum State { 
							 CHECK_QUEUE,
							 CHECKING,
							 ANNOUNCING,
							 DOWNLOADING,
							 FINISHED,
							 SEEDING,
							 ALLOCATING,
							 STOPPED,
							 QUEUED
						 };

	const torrent_handle& get_handle() const;
	const Glib::ustring& get_tracker_reply() const;
	const Glib::ustring& get_name() const;
	const Glib::ustring& get_group() const;
	const Glib::ustring& get_path() const;
	const unsigned int get_position() const;
	const std::vector<bool>& get_filter() const;
	const int get_up_limit() const;
	const int get_down_limit() const;
	const sha1_hash& get_hash() const;
	
	const size_type get_total_downloaded() const;
	const size_type get_total_uploaded() const;
	
	const State get_state() const;
	const Glib::ustring get_state_string() const;
	const Glib::ustring get_state_string(State state) const;
	
	const torrent_info get_info() const;
	const torrent_status get_status() const;
	const std::vector<partial_piece_info> get_download_queue() const;
	const std::vector<float> get_file_progress();

	void set_tracker_reply(const Glib::ustring& reply);
	void set_group(const Glib::ustring& group);
	void set_position(unsigned int position);
	void set_filter(std::vector<bool>& filter);
	void filter_file(const Glib::ustring& name, bool filter = true);
	void set_up_limit(int limit);
	void set_down_limit(int limit);
	
	void queue();
	void unqueue();
	bool is_queued();
	bool is_running();
	
	const entry get_resume_entry(bool running = false);
	
	/* FIXME: Friend access only? */
	void set_handle(const torrent_handle& handle);
	
	Torrent(const entry& resume_data, bool queued = false);
	~Torrent();

private:
	struct ResumeInfo {
											//unsigned int time;
											size_type uploaded;
											size_type downloaded;
										} m_resume;

	torrent_handle m_handle;
	sha1_hash m_hash;
	
	Glib::TimeVal m_time_val;
	
	Glib::ustring m_tracker_reply;
	
	std::vector<bool> m_filter;
	
	bool m_is_queued;
	
	unsigned int m_position;
	Glib::ustring m_name, m_group, m_path;
	int m_up_limit, m_down_limit;
};

#endif /* TORRENT_HH */
