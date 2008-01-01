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

#include "libtorrent/torrent.hpp"
#include "libtorrent/intrusive_ptr_base.hpp"

#include "linkage/DBusAdaptorGlue.hh"

namespace Linkage
{
class Torrent;
typedef boost::intrusive_ptr<Torrent> TorrentPtr;

typedef std::list<libtorrent::sha1_hash> HashList;

class Torrent
:
  public libtorrent::intrusive_ptr_base<Torrent>,
  public Glib::Object,
  public org::linkage::Torrent,
  public DBus::IntrospectableAdaptor,
  public DBus::ObjectAdaptor
{
public:
	void reference() {}
	void unreference() {}

	typedef boost::intrusive_ptr<libtorrent::torrent_info> InfoPtr;

	enum State
	{
		NONE = 0,
		ANNOUNCING = 1 << 0,
		DOWNLOADING = 1 << 1,
		FINISHED = 1 << 2,
		SEEDING = 1 << 3,
		CHECK_QUEUE = 1 << 4,
		CHECKING = 1 << 5,
		ALLOCATING = 1 << 6,
		STOPPED = 1 << 7,
		QUEUED = 1 << 8,
		ERROR = 1 << 9
	};
	enum Priority
	{
		P_SKIP,
		P_NORMAL,
		P_FAVOURED,
		P_AS_PARTIAL,
		P_HIGH,
		/* P_HIGHER, same as 4 see docs */
		P_AS_RARE = 6,
		P_MAX = 7
	};

	Glib::PropertyProxy<libtorrent::torrent_handle> property_handle();
	Glib::PropertyProxy<unsigned int> property_position();
	//Glib::PropertyProxy<State> property_state();

	libtorrent::torrent_handle get_handle();
	Glib::ustring get_tracker_reply(const Glib::ustring& tracker);
	Glib::ustring get_name();
	const Glib::ustring& get_group();
	const Glib::ustring& get_path();
	unsigned int get_position();
	const std::vector<int>& get_priorities();
	int get_up_limit();
	int get_down_limit();
	libtorrent::sha1_hash get_hash();
	float get_progress();

	libtorrent::size_type get_total_downloaded();
	libtorrent::size_type get_total_uploaded();

	//FIXME: remove this, use composite States (w/ FINISHED) instead
	bool is_completed();

	int get_state();
	Glib::ustring get_state_string();
	static Glib::ustring state_string(int state);
	Glib::ustring get_state_string(int state); /* FIXME: remove this */

	const Torrent::InfoPtr& get_info();
	libtorrent::torrent_status get_status();
	std::vector<float> get_file_progress();

	void set_name(const Glib::ustring& name);
	void set_group(const Glib::ustring& group);
	void set_path(const Glib::ustring& path); /* This does NOT move storage */
	void set_position(unsigned int position);
	void set_priorities(const std::vector<int>& priorities);
	void set_file_priority(int index, int priority);
	void set_up_limit(int limit);
	void set_down_limit(int limit);

	void set_completed(bool completed = true);

	void add_tracker(const Glib::ustring& url);

	void queue();
	void unqueue();
	bool is_queued();
	bool is_stopped();

	void reannounce(const Glib::ustring& tracker = Glib::ustring());
	const std::vector<libtorrent::announce_entry>& get_trackers();

	const libtorrent::entry get_resume_entry(bool stopping = true, bool quitting = false);

	static TorrentPtr create(const libtorrent::entry& e, const InfoPtr& info, bool queued = false);
	~Torrent();

private:
	/* DBUS */
	DBus::String GetName();
	DBus::String GetState();
	DBus::Struct<DBus::UInt32, DBus::UInt32> GetRates();
	DBus::Struct<DBus::UInt64, DBus::UInt64> GetTransfered();
	DBus::Double GetProgress();
	DBus::UInt32 GetPosition();
	void Start();
	void Stop();
	void Remove(const DBus::Bool& erase_content);

	friend class TorrentManager;
	friend class SessionManager;

	enum ReplyType { REPLY_ANY, REPLY_OK, REPLY_ANNOUNCING };
	void set_tracker_reply(const Glib::ustring& reply, const Glib::ustring& tracker = Glib::ustring(), ReplyType type = REPLY_ANY);

	void set_handle(const libtorrent::torrent_handle& handle);

	/* FIXME: use a StoppedInfo struct so we don't duplicate info with handles */
	struct StoppedCache
	{
		std::vector<libtorrent::announce_entry> trackers;
		std::vector<float> file_progress;
		libtorrent::torrent_status status;
		std::vector<bool> pieces;
		/* no need to have m_info here since it's a ref ptr */
	};
	std::auto_ptr<StoppedCache> m_cache;

	libtorrent::size_type m_uploaded;
	libtorrent::size_type m_downloaded;

	Glib::Property<libtorrent::torrent_handle> m_prop_handle;
	Glib::Property<unsigned int> m_prop_position;
	//Glib::Property<State> m_prop_state;

	InfoPtr m_info;

	std::vector<int> m_priorities;

	typedef std::map<Glib::ustring, Glib::ustring> ReplyMap;
	ReplyMap m_replies;
	int m_cur_tier;
	bool m_announcing;

	bool m_is_queued;

	Glib::ustring m_group, m_path, m_name;
	int m_up_limit, m_down_limit;

	bool m_completed;

	Torrent(const Torrent&);
	Torrent& operator=(const Torrent);
	Torrent(const libtorrent::entry& e, const InfoPtr& info, bool queued);
};

}; /* namespace */

#endif /* TORRENT_HH */
