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

#include <boost/smart_ptr.hpp>

#include "libtorrent/intrusive_ptr_base.hpp"

#include "linkage/Torrent.hh"

namespace Linkage
{
class TorrentManager;
typedef boost::intrusive_ptr<TorrentManager> TorrentManagerPtr;

class Value;

class TorrentManager : public libtorrent::intrusive_ptr_base<TorrentManager>
{
public:
	typedef std::list<TorrentPtr> TorrentList;

	sigc::signal<void, TorrentPtr> signal_added();
	sigc::signal<void, TorrentPtr> signal_removed();
	
	bool exists(const libtorrent::sha1_hash& hash);
	bool exists(const Glib::ustring& hash_str);
	
	TorrentPtr get_torrent(const libtorrent::sha1_hash& hash);
	TorrentList get_torrents();
	
	unsigned int get_torrents_count();
	
	static TorrentManagerPtr create();
	~TorrentManager();

private:
	typedef std::vector<TorrentPtr> TorrentVector;
	typedef std::map<libtorrent::sha1_hash, TorrentPtr> TorrentMap;

	TorrentMap m_torrents;

	sigc::signal<void, TorrentPtr> m_signal_added;
	sigc::signal<void, TorrentPtr> m_signal_removed;
	sigc::signal<void, Glib::ustring, Torrent::InfoPtr> m_signal_load_failed;

	void on_tracker_announce(const Linkage::TorrentPtr& torrent, const Glib::ustring& msg);
	void on_tracker_reply(const Linkage::TorrentPtr& torrent, const Glib::ustring& reply, const Glib::ustring& tracker, int peers);
	void on_tracker_warning(const Linkage::TorrentPtr& torrent, const Glib::ustring& reply);
	void on_tracker_failed(const Linkage::TorrentPtr& torrent, const Glib::ustring& reply, const Glib::ustring& tracker, int code, int times);

	void on_update_queue(const Linkage::TorrentPtr& torrent, const Glib::ustring& msg);

	void on_handle_changed(const WeakTorrentPtr& torrent);
	void on_position_changed(const WeakTorrentPtr& torrent);
	void set_torrent_settings(const TorrentPtr& torrent);
	
	void on_key_changed(const Glib::ustring& key, const Value& value);

	void load_torrents();
	std::pair<TorrentPtr, libtorrent::entry> load_torrent(const Glib::ustring& file);

	TorrentManager();

	friend class SessionManager;

	TorrentPtr add_torrent(libtorrent::entry& er, const Torrent::InfoPtr& info);
	void remove_torrent(const TorrentPtr& torrent);
	void check_queue();
};

} /* namespace */

#endif /* TORRENTMANAGER_HH */
