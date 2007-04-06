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

using namespace libtorrent;

typedef std::list<sha1_hash> HashList;

class Torrent : public Glib::Object
{
public:
	enum State { 
							 CHECK_QUEUE,
							 CHECKING,
							 ANNOUNCING,
							 DOWNLOADING,
							 FINISHED,
							 SEEDING,
							 ALLOCATING,
							 STOPPED,
							 QUEUED,
							 ERROR
						 };
	struct ResumeInfo 
	{
		entry resume;
		torrent_info info;
		ResumeInfo(const entry& e, const torrent_info& i) : resume(e), info(i) {}
	};

	Glib::PropertyProxy<torrent_handle> property_handle();
	torrent_handle get_handle() const;
	const std::pair<Glib::ustring, Glib::ustring> get_tracker_reply();
	const Glib::ustring get_name() const;
	const Glib::ustring& get_group() const;
	const Glib::ustring& get_path() const;
	const unsigned int get_position() const;
	const std::vector<bool>& get_filter() const;
	const int get_up_limit() const;
	const int get_down_limit() const;
	const sha1_hash get_hash() const;
	
	const size_type get_total_downloaded() const;
	const size_type get_total_uploaded() const;
	
	const State get_state() const;
	const Glib::ustring get_state_string() const;
	const Glib::ustring get_state_string(State state) const;
	
	const torrent_info& get_info() const;
	const torrent_status get_status() const;
	const std::vector<partial_piece_info> get_download_queue() const;
	const std::vector<float> get_file_progress();

	void set_tracker_reply(const Glib::ustring& reply, const Glib::ustring& tracker = Glib::ustring());
	void set_group(const Glib::ustring& group);
	void set_position(unsigned int position);
	void set_filter(std::vector<bool>& filter);
	void filter_file(unsigned int index, bool filter = true);
	void set_up_limit(int limit);
	void set_down_limit(int limit);
	
	void queue();
	void unqueue();
	bool is_queued();
	bool is_running();

	void reannounce(const Glib::ustring& tracker = Glib::ustring());

	const entry get_resume_entry(bool running = false);
	
	/* FIXME: Friend access only? */
	void set_handle(const torrent_handle& handle);
	
	Torrent(const ResumeInfo& ri, bool queued = false);
	~Torrent();

private:
	size_type m_uploaded;
	size_type m_downloaded;

	Glib::Property<torrent_handle> m_prop_handle;
	torrent_info m_info;
	
	typedef std::map<Glib::ustring, Glib::ustring> ReplyMap;
	ReplyMap m_replies;
	int m_cur_tier;
	bool m_announcing;

	std::vector<bool> m_filter;
	
	bool m_is_queued;
	
	unsigned int m_position;
	Glib::ustring m_group, m_path;
	int m_up_limit, m_down_limit;
};

#endif /* TORRENT_HH */
