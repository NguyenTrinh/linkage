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
#include <list>

#include "linkage/Torrent.hh"
#include "linkage/RefCounter.hh"
#include "linkage/WeakPtr.hh"
#include "linkage/SessionManager.hh"

class TorrentManager : public RefCounter<TorrentManager>
{
private:
	typedef std::map<libtorrent::sha1_hash, Torrent*> TorrentMap;
	typedef TorrentMap::iterator TorrentIter;
	
	TorrentMap m_torrents;

	void sort(std::vector<Torrent*>& torrents);

	sigc::signal<void, const libtorrent::sha1_hash&, const Glib::ustring&, unsigned int> m_signal_added;
	sigc::signal<void, const libtorrent::sha1_hash&> m_signal_removed;

	void on_tracker_announce(const libtorrent::sha1_hash& hash, const Glib::ustring& msg);
	void on_tracker_reply(const libtorrent::sha1_hash& hash, const Glib::ustring& reply, int peers);
	void on_tracker_warning(const libtorrent::sha1_hash& hash, const Glib::ustring& reply);
	void on_tracker_failed(const libtorrent::sha1_hash& hash, const Glib::ustring& reply, int code, int times);

	void on_update_queue(const libtorrent::sha1_hash& hash, const Glib::ustring& msg);

	void on_handle_changed(Torrent* torrent);
	void set_torrent_settings(Torrent* torrent);
	
	void on_settings();

	TorrentManager();
	
	/* FIXME: hack to make sure SessionManager goes out of reference _after_ TorrentManager */
	Glib::RefPtr<SessionManager> m_session_manager;

protected:
	friend class SessionManager;

	WeakPtr<Torrent> add_torrent(const libtorrent::entry& e, const libtorrent::torrent_info& info);
	void remove_torrent(const libtorrent::sha1_hash& hash);
	void check_queue();

public:
	typedef std::list<WeakPtr<Torrent> > TorrentList;

	sigc::signal<void, const libtorrent::sha1_hash&, const Glib::ustring&, unsigned int> signal_added();
	sigc::signal<void, const libtorrent::sha1_hash&> signal_removed();
	
	void set_torrent_position(const libtorrent::sha1_hash& hash, int diff);
	
	bool exists(const libtorrent::sha1_hash& hash);
	bool exists(const Glib::ustring& hash_str);
	
	//FIXME: Should be removed, or at least friends only.
	libtorrent::torrent_handle get_handle(const libtorrent::sha1_hash& hash);
	
	WeakPtr<Torrent> get_torrent(const libtorrent::sha1_hash& hash);
	TorrentList get_torrents();
	
	unsigned int get_torrents_count();
	
	static Glib::RefPtr<TorrentManager> create();
	~TorrentManager();
};

#endif /* TORRENTMANAGER_HH */
