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

#ifndef TORRENTMANAGER_HH
#define TORRENTMANAGER_HH

#include <map>
#include <vector>
#include <list>

#include <glibmm/object.h>
#include <glibmm/refptr.h>

#include "linkage/Torrent.hh"

namespace Linkage
{

class Value;

class TorrentManager : public Glib::Object
{
public:
	typedef std::list<Glib::RefPtr<Torrent> > TorrentList;

	sigc::signal<void, Glib::RefPtr<Torrent> > signal_added();
	sigc::signal<void, Glib::RefPtr<Torrent> > signal_removed();
	
	bool exists(const libtorrent::sha1_hash& hash);
	bool exists(const Glib::ustring& hash_str);
	
	Glib::RefPtr<Torrent> get_torrent(const libtorrent::sha1_hash& hash);
	TorrentList get_torrents();
	
	unsigned int get_torrents_count();
	
	static Glib::RefPtr<TorrentManager> create();
	~TorrentManager();

private:
	typedef std::vector<Glib::RefPtr<Torrent> > TorrentVector;
	typedef std::map<libtorrent::sha1_hash, Glib::RefPtr<Torrent> > TorrentMap;

	TorrentMap m_torrents;

	struct pred : public std::binary_function
		<const Glib::RefPtr<Torrent>&, const Glib::RefPtr<Torrent>&, bool>
	{
		bool operator()(const Glib::RefPtr<Torrent>& rhs, const Glib::RefPtr<Torrent>& lhs)
		{
			return rhs->get_position() < lhs->get_position();
		}
	};

	sigc::signal<void, Glib::RefPtr<Torrent> > m_signal_added;
	sigc::signal<void, Glib::RefPtr<Torrent> > m_signal_removed;
	sigc::signal<void, Glib::ustring, Torrent::InfoPtr> m_signal_load_failed;

	void on_tracker_announce(const libtorrent::sha1_hash& hash, const Glib::ustring& msg);
	void on_tracker_reply(const libtorrent::sha1_hash& hash, const Glib::ustring& reply, int peers);
	void on_tracker_warning(const libtorrent::sha1_hash& hash, const Glib::ustring& reply);
	void on_tracker_failed(const libtorrent::sha1_hash& hash, const Glib::ustring& reply, int code, int times);

	void on_update_queue(const libtorrent::sha1_hash& hash, const Glib::ustring& msg);

	void on_handle_changed(const libtorrent::sha1_hash& hash);
	void on_position_changed(const libtorrent::sha1_hash& hash);
	void set_torrent_settings(const Glib::RefPtr<Torrent>& torrent);
	
	void on_key_changed(const Glib::ustring& key, const Value& value);

	typedef std::map<Glib::RefPtr<Torrent>, libtorrent::entry> ResumeMap;
	void load_torrents();
	void load_torrent(const Glib::ustring& file, ResumeMap& resumes);

	TorrentManager();

	friend class SessionManager;

	Glib::RefPtr<Torrent> add_torrent(libtorrent::entry& er, const Torrent::InfoPtr& info);
	void remove_torrent(const Glib::RefPtr<Torrent>& torrent);
	void check_queue();
};

}; /* namespace */

#endif /* TORRENTMANAGER_HH */
