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

#include <glibmm/i18n.h>

#include "libtorrent/alert_types.hpp"

#include "linkage/Torrent.hh"
#include "linkage/Utils.hh"
#include "linkage/Engine.hh"
#include "linkage/SessionManager.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/TorrentManager.hh"
#include "linkage/compose.hpp"

using namespace Linkage;

static DBus::Path get_torrent_path(const libtorrent::sha1_hash& hash)
{
	return "/org/linkage/torrents/" + String::compose("%1", hash);
}

TorrentPtr Torrent::create(const libtorrent::entry& e, const Torrent::InfoPtr& info, bool queued)
{
	return TorrentPtr(new Torrent(e, info, queued));
}

Torrent::Torrent(const libtorrent::entry& e, const Torrent::InfoPtr& info, bool queued)
:
  DBus::ObjectAdaptor(Engine::get_bus(), get_torrent_path(info->info_hash())),
  m_prop_handle(*this, "handle"),
  m_prop_position(*this, "position"),
  m_prop_state(*this, "state", Torrent::STOPPED),
  m_info(info),
  m_cur_tier(0),
  m_is_queued(queued),
  m_stop_ratio(0)
{
	m_cache = std::auto_ptr<StoppedCache>(new StoppedCache());

	m_cache->file_progress.assign(m_info->num_files(), 0);
	/* try and get file and piece progress from resume file */
	if (e.find_key("slots"))
	{
		libtorrent::entry::list_type e_slots = e["slots"].list();
		libtorrent::entry::list_type e_unfinished;
		if (e.find_key("unfinished"))
			e_unfinished = e["unfinished"].list();
		int index = 0, piece = 0;
		libtorrent::size_type bytes = 0, bytes_done = 0;;
		for (libtorrent::entry::list_type::iterator iter = e_slots.begin();
			iter != e_slots.end(); ++iter)
		{
			g_assert(piece < m_info->num_pieces());

			/* check if the piece is partially done */
			bool unfinished = false;
			for (libtorrent::entry::list_type::iterator k = e_unfinished.begin();
				k != e_unfinished.end() && !unfinished; ++k)
			{
				libtorrent::entry::dictionary_type d = k->dict();
				unfinished = (d["piece"].integer() == m_cache->pieces.size());
			}

			/* a negative slot int means it's not done */
			bool piece_done = iter->integer() >= 0 && !unfinished;
			m_cache->pieces.push_back(piece_done);

			/* count bytes to keep track of which file where in */
			libtorrent::size_type ps = m_info->piece_size(piece);
			bytes += ps;
			bytes_done += piece_done ? ps : 0;
			libtorrent::size_type size = m_info->file_at(index).size;
			/* get progress for current file if we pass it */
			while (bytes >= size)
			{
				libtorrent::size_type diff = bytes - size;
				bytes_done -= piece_done ? diff : 0;

				float p = size > 0 ? (float)bytes_done/size : (float)piece_done;
				m_cache->file_progress[index] = p;

				bytes = diff;
				bytes_done = piece_done ? diff : 0;
				index++;

				if (index >= m_info->num_files())
					break;

				/* loop if last piece spans over small files */
				size = m_info->file_at(index).size;
			}
			piece++;
		}
	}
	else
		m_cache->pieces.assign(m_info->num_pieces(), false);

	g_assert((int)m_cache->pieces.size() == m_info->num_pieces());
	g_assert((int)m_cache->file_progress.size() == m_info->num_files());

	m_cache->status.pieces = &m_cache->pieces;

	m_downloaded = e["downloaded"].integer();
	m_uploaded = e["uploaded"].integer();

	m_path = e["path"].string();
	m_prop_position = e["position"].integer();
	if (e.find_key("group"))
		m_group = e["group"].string();
	if (e.find_key("name"))
		m_name = e["name"].string();

	m_up_limit = e["upload-limit"].integer();
	m_down_limit = e["download-limit"].integer();

	m_completed = (bool)e["completed"].integer();

	if (e.find_key("priorities"))
	{
		libtorrent::entry::list_type e_priorities = e["priorities"].list();
		for (libtorrent::entry::list_type::iterator iter = e_priorities.begin();
			iter != e_priorities.end(); ++iter)
		{
			m_priorities.push_back(iter->integer());
		}
	}
	else
		m_priorities.assign(m_info->num_files(), P_NORMAL);

	// make sure it's a list, old version used dictionary
	if (e.find_key("trackers") && e["trackers"].type() == libtorrent::entry::list_t)
	{
		libtorrent::entry::list_type e_trackers = e["trackers"].list();
		int tier = 0;
		for (libtorrent::entry::list_type::iterator iter = e_trackers.begin();
			iter != e_trackers.end(); ++iter)
		{
			libtorrent::announce_entry a(iter->string());
			a.tier = tier;
			m_cache->trackers.push_back(a);
			tier++;
		}
	}
	else
		m_cache->trackers = m_info->trackers();

	for (unsigned int i = 0; i < m_cache->trackers.size(); i++)
		m_replies[m_cache->trackers[i].url] = "";

	m_cache->status.progress = _get_stopped_progress();

	Engine::signal_tick().connect(sigc::mem_fun(this, &Torrent::on_tick));
}

Torrent::~Torrent()
{
}

Glib::PropertyProxy_ReadOnly<libtorrent::torrent_handle> Torrent::property_handle()
{
	Glib::ObjectBase* base = dynamic_cast<Glib::ObjectBase*>(this);
	return Glib::PropertyProxy_ReadOnly<libtorrent::torrent_handle>(base, "handle");
}

Glib::PropertyProxy<unsigned int> Torrent::property_position()
{
	return m_prop_position.get_proxy();
}

Glib::PropertyProxy_ReadOnly<int> Torrent::property_state()
{
	Glib::ObjectBase* base = dynamic_cast<Glib::ObjectBase*>(this);
	return Glib::PropertyProxy_ReadOnly<int>(base, "state");
}

libtorrent::torrent_handle Torrent::get_handle()
{
	return m_prop_handle.get_value();
}

void Torrent::on_tick()
{
	int new_state = _get_state(); /* possibly new state */
	int cur_state = get_state(); /* from m_prop_state */

	if (cur_state != new_state)
		m_prop_state = new_state;

	/* TODO: add more useful properties and update them here */

	if (m_stop_ratio >= 1.0f && get_state() & SEEDING)
	{
		float ratio = 0;
		if (m_downloaded)
			ratio = (1.0f*m_uploaded)/(1.0f*m_downloaded);

		if (ratio >= m_stop_ratio)
		{
			libtorrent::entry e = get_resume_entry();
			e["stopped"] = 1;
			save_entry(Glib::build_filename(get_data_dir(),
				String::compose("%1", get_hash()) + ".resume"), e);

			libtorrent::torrent_handle handle = get_handle();
			set_handle(libtorrent::torrent_handle());
			Engine::get_session_manager()->remove_torrent(handle);
		}
	}
}

int Torrent::_get_state()
{
	if (is_stopped())
		return STOPPED | (m_completed ? FINISHED : 0);

	libtorrent::torrent_status status = get_handle().status();

	libtorrent::torrent_status::state_t state = status.state;
	if (get_handle().is_paused())
	{
		if (m_is_queued)
		{
			/* Queued torrents can be in check state */
			if (state == libtorrent::torrent_status::checking_files)
				return CHECKING;
			else if (state == libtorrent::torrent_status::queued_for_checking)
				return CHECK_QUEUE;
			else
				return QUEUED;
		}
		else /* libtorrent paused this handle, something bad happened */
			return ERROR;
	}

	switch (state)
	{
		case libtorrent::torrent_status::queued_for_checking:
			return CHECK_QUEUE;
		case libtorrent::torrent_status::checking_files:
			return CHECKING;
		case libtorrent::torrent_status::connecting_to_tracker:
			return ANNOUNCING | (m_completed ? FINISHED : DOWNLOADING);
		case libtorrent::torrent_status::finished:
			return FINISHED;
		case libtorrent::torrent_status::seeding:
			return SEEDING;
		case libtorrent::torrent_status::allocating:
			return ALLOCATING;
		case libtorrent::torrent_status::downloading:
		default:
			return DOWNLOADING;
	}
}

float Torrent::_get_stopped_progress()
{
	libtorrent::size_type wanted_size = m_info->total_size();
	for (unsigned int i = 0; i < m_priorities.size(); i++)
	{
		// priority 0 means "don't download"
		if (!m_priorities[i])
			wanted_size -= m_info->file_at(i).size;
	}

	libtorrent::size_type down = get_previously_downloaded();
	float progress = 0;
	if (wanted_size)
		progress = (double)down/wanted_size;
	if (progress > 1)
		progress = 1;

	return progress;
}

Glib::ustring Torrent::get_name()
{
	return m_name.empty() ? Glib::ustring(m_info->name()) : m_name;
}

const Glib::ustring& Torrent::get_group()
{
	return m_group;
}

const Glib::ustring& Torrent::get_path()
{
	return m_path;
}

Glib::ustring Torrent::get_tracker_reply(const Glib::ustring& tracker)
{
	ReplyMap::iterator iter = m_replies.find(tracker);
	if (iter != m_replies.end())
		return iter->second;

	return Glib::ustring();
}

unsigned int Torrent::get_position()
{
	return m_prop_position.get_value();
}

const std::vector<int>& Torrent::get_priorities()
{
	return m_priorities;
}

int Torrent::get_up_limit()
{
	return m_up_limit;
}

int Torrent::get_down_limit()
{
	return m_down_limit;
}

float Torrent::get_stop_ratio()
{
	return m_stop_ratio;
}

libtorrent::sha1_hash Torrent::get_hash()
{
	return m_info->info_hash();
}

libtorrent::size_type Torrent::get_previously_downloaded()
{
	return m_downloaded;
}

libtorrent::size_type Torrent::get_previously_uploaded()
{
	return m_uploaded;
}

bool Torrent::is_completed()
{
	return m_completed;
}

int Torrent::get_state()
{
	return m_prop_state.get_value();
}

Glib::ustring Torrent::state_string(int state)
{
	//TODO: add SEEDING | FINISHED state
	//FIXME: don't do case, generalize with if state&E .. ", " if state&Q .. ", "
	switch (state)
	{
		case ERROR:
			return _("Error");
		case QUEUED:
			return _("Queued");
		case STOPPED:
			return _("Stopped");
		case CHECK_QUEUE:
			return _("Queued for checking");
		case CHECKING:
			return _("Checking");
		case ANNOUNCING:
			return _("Announcing");
		case ANNOUNCING | DOWNLOADING:
			return String::ucompose("%1, %2", _("Downloading"), _("Announcing"));
		case ANNOUNCING | FINISHED:
			return String::ucompose("%1, %2", _("Finished"), _("Announcing"));
		case FINISHED:
			return _("Finished");
		case SEEDING:
			return _("Seeding");
		case ALLOCATING:
			return _("Allocating");
		case STOPPED | FINISHED:
			return String::ucompose("%1, %2", _("Stopped"), _("Finished"));
		case DOWNLOADING:
			return _("Downloading");
		default:
			g_assert_not_reached();
	}
}

const Torrent::InfoPtr& Torrent::get_info()
{
	return m_info;
}

libtorrent::torrent_status Torrent::get_status()
{
	if (is_stopped())
	{
		return m_cache->status;
	}

	return get_handle().status();
}

std::vector<float> Torrent::get_file_progress()
{
	std::vector<float> fp;
	if (is_stopped())
	{
		fp = m_cache->file_progress;
	}
	else
		get_handle().file_progress(fp);

	return fp;
}

void Torrent::set_handle(const libtorrent::torrent_handle& handle)
{
	if (is_stopped() && handle.is_valid())
	{
		/* set cached trackers before we delete them */
		handle.replace_trackers(m_cache->trackers);
		/* free cache */
		m_cache = std::auto_ptr<StoppedCache>(NULL);
	}
	else if (!is_stopped() && !handle.is_valid())
	{
		m_downloaded += get_status().total_payload_download;
		m_uploaded += get_status().total_payload_upload;

		m_cache = std::auto_ptr<StoppedCache>(new StoppedCache());
		get_handle().file_progress(m_cache->file_progress);
		m_cache->trackers = get_handle().trackers();
		m_cache->status = get_handle().status();

		/* reset all values that are no longer valid */
		/* hack, set status to unused state for now */
		m_cache->status.state = libtorrent::torrent_status::downloading_metadata;
		m_cache->pieces = *(m_cache->status.pieces);
		m_cache->status.pieces = &m_cache->pieces;
		m_cache->status.progress = _get_stopped_progress();
		m_cache->status.next_announce = boost::posix_time::seconds(0);
		m_cache->status.announce_interval = boost::posix_time::seconds(0);
		m_cache->status.download_rate = 0;
		m_cache->status.upload_rate = 0;
		m_cache->status.download_payload_rate = 0;
		m_cache->status.upload_payload_rate = 0;
		m_cache->status.total_download = 0;
		m_cache->status.total_upload = 0;
		m_cache->status.total_payload_download = 0;
		m_cache->status.total_payload_upload = 0;
		m_cache->status.num_peers = 0;
		m_cache->status.num_seeds = 0;
		m_cache->status.distributed_copies = -1;
		m_cache->status.current_tracker = "";
		/* FIXME: reset more stuff */
	}

	m_prop_handle = handle;

	/* Reset all replies on stop */
	if (is_stopped())
	{
		for (ReplyMap::iterator iter = m_replies.begin(); iter != m_replies.end(); ++iter)
			iter->second = "";
		m_cur_tier = 0;
	}
	else
	{
		set_priorities(m_priorities);
		set_up_limit(m_up_limit);
		set_down_limit(m_down_limit);
	}
}

void Torrent::set_name(const Glib::ustring& name)
{
	m_name = name;
}

void Torrent::set_group(const Glib::ustring& group)
{
	m_group = group;
}

void Torrent::set_path(const Glib::ustring& path)
{
	g_return_if_fail(is_stopped());

	m_path = path;
	libtorrent::entry e = get_resume_entry();
	save_entry(Glib::build_filename(get_data_dir(), String::compose("%1", get_hash()) + ".resume"), e);
}

void Torrent::set_tracker_reply(const Glib::ustring& reply, const Glib::ustring& tracker, Torrent::ReplyType type)
{
	if (is_stopped())
		return;

	/* Get trackers from handle, since the list might have changed */
	std::vector<libtorrent::announce_entry> trackers = get_handle().trackers();

	if (tracker.empty())
	{
		for (unsigned int j = 0; j < trackers.size(); j++)
		{
			if (trackers[j].tier == m_cur_tier)
			{
				m_replies[trackers[j].url] = reply;
				m_cur_tier = (m_cur_tier + 1) % trackers.size();
				break;
			}
		}
	}
	else
		m_replies[tracker] = reply;

	/* A tracker responded and the announce cycle stops */
	if (type == REPLY_OK)
	{
		m_announcing = false;
 		m_cur_tier = 0;
	}
	/* All trackers failed, cycle stops */
	else if (m_cur_tier == 0 && type != REPLY_ANNOUNCING)
		m_announcing = false;
	else
		m_announcing = true;
}

void Torrent::set_position(unsigned int position)
{
	m_prop_position = position;
}

void Torrent::set_priorities(const std::vector<int>& priorities)
{
	m_priorities = priorities;

	if (is_stopped())
	{
		m_cache->status.progress = _get_stopped_progress();
		return;
	}

	get_handle().prioritize_files(m_priorities);

	if (Engine::get_settings_manager()->get_bool("files/prioritize_firstlast"))
	{
		//FIXME: prioritize 5%-10% at level 6?
		//prioritize the first and last 5% of the files size
		for (int i = 0; i < m_info->num_files(); i++)
		{
			if (!m_priorities[i])
				continue;

			const libtorrent::file_entry& file = m_info->file_at(i);
			libtorrent::size_type size_prio = (libtorrent::size_type)0.05*file.size;
			int front = m_info->map_file(i, 0, size_prio).piece;
			int end = m_info->map_file(i, file.size, 0).piece;
			int num_prio = size_prio/m_info->piece_length();
			//this means file.size > ~piece_length*20 
			while (num_prio--)
			{
				get_handle().piece_priority(front++, P_MAX);
				get_handle().piece_priority(end--, P_MAX);
			}
		}
	}
}

void Torrent::set_file_priority(int index, int priority)
{
	m_priorities[index] = priority;
	set_priorities(m_priorities);
}

void Torrent::set_up_limit(int limit)
{
	m_up_limit = limit;
	if (!is_stopped())
	{
		if (m_up_limit <= 0)
			get_handle().set_upload_limit(-1);
		else
			get_handle().set_upload_limit(m_up_limit*1024);
	}
}

void Torrent::set_down_limit(int limit)
{
	m_down_limit = limit;
	if (!is_stopped())
		{
		if (m_down_limit <= 0)
			get_handle().set_download_limit(-1);
		else
			get_handle().set_download_limit(m_down_limit*1024);
	}
}

void Torrent::set_stop_ratio(float ratio)
{
	/* ratio < 1 means unlimited */
	m_stop_ratio = ratio;
}

void Torrent::set_completed(bool completed)
{
	m_completed = completed;
}

void Torrent::add_tracker(const Glib::ustring& url)
{	
	libtorrent::announce_entry a(url);

	m_replies[url] = "";

	if (!is_stopped())
	{
		std::vector<libtorrent::announce_entry> trackers = get_trackers();
		a.tier = trackers.size();
		trackers.push_back(a);
		get_handle().replace_trackers(trackers);
	}
	else
	{
		a.tier = m_cache->trackers.size();
		m_cache->trackers.push_back(a);
	}
}

const std::vector<libtorrent::announce_entry>& Torrent::get_trackers()
{
	if (is_stopped())
	{
		return m_cache->trackers;
	}

	return get_handle().trackers();
}

void Torrent::queue()
{
	if (!is_stopped())
	{
		get_handle().pause();
		m_is_queued = true;
	}
}

void Torrent::unqueue()
{
	if (!is_stopped())
	{
		get_handle().resume();
		m_is_queued = false;
	}
}

bool Torrent::is_queued()
{
	return m_is_queued;
}

bool Torrent::is_stopped()
{
	return !get_handle().is_valid();
}

void Torrent::reannounce(const Glib::ustring& tracker)
{
	g_return_if_fail(!is_stopped());
	g_return_if_fail(!m_announcing);

	m_announcing = true;

	std::vector<libtorrent::announce_entry> trackers = get_trackers();
	if (!tracker.empty())
	{
		for (std::vector<libtorrent::announce_entry>::iterator i = trackers.begin();
			i != trackers.end(); ++i)
		{
			if (i->url == tracker)
			{
				m_cur_tier = i->tier;
				libtorrent::announce_entry a(tracker);
				a.tier = 0;
				trackers.erase(i);
				trackers.insert(trackers.begin(), a);
				break;
			}
		}
		for (unsigned int i = 1; i < trackers.size(); i++)
		{
			if (trackers[i].tier < m_cur_tier)
				trackers[i].tier++;
		}

		get_handle().replace_trackers(trackers);
	}

	m_cur_tier = 0;
	// FIXME: if someone else calls force_reannounce while we are announcing it
	// will mess up the iteration in set_tracker_reply maybe we should wrap the handle?
	get_handle().force_reannounce();
}

libtorrent::entry Torrent::get_resume_entry()
{
	libtorrent::entry::dictionary_type e;

	try
	{
		e = get_handle().write_resume_data().dict();
	}
	catch (std::exception& ex)
	{
		Glib::ustring file = Glib::build_filename(get_data_dir(),
		String::compose("%1", m_info->info_hash()) + ".resume");

		libtorrent::entry er;
		if (load_entry(file, er))
			e = er.dict();
	}

	if (e.empty())
		e["info-hash"] = std::string(m_info->info_hash().begin(),
			m_info->info_hash().end());

	libtorrent::size_type down = m_downloaded;
	libtorrent::size_type up = m_uploaded;
	/* asume that torrent will be stopped if the handle is paused */
	if (!is_stopped() && get_handle().is_paused())
	{
		down += get_status().total_payload_download;
		up += get_status().total_payload_upload;
	}

	e["path"] = m_path;
	e["position"] = get_position();
	e["stopped"] = is_stopped();
	e["downloaded"] = down;
	e["uploaded"] = up;
	e["download-limit"] = m_down_limit;
	e["upload-limit"] = m_up_limit;
	e["completed"] = m_completed;

	if (!m_name.empty())
		e["name"] = m_name;

	libtorrent::entry::list_type e_priorities(m_priorities.begin(), m_priorities.end());
	e["priorities"] = e_priorities;

	if (!m_group.empty())
		e["group"] = m_group;

	std::vector<libtorrent::announce_entry> trackers = get_trackers();
	libtorrent::entry::list_type e_trackers;
	for (unsigned int i = 0; i < trackers.size(); i++)
		e_trackers.push_back(trackers[i].url);
	e["trackers"] = e_trackers;

	return e;
}

libtorrent::size_type Torrent::get_eta()
{
	libtorrent::torrent_status status = get_status();
	libtorrent::size_type eta;
	libtorrent::size_type down = get_total_downloaded();
	float ratio = (1.0f*get_total_uploaded())/(1.0f*down);
	
	if (get_state() & Torrent::SEEDING || get_state() & Torrent::FINISHED)
	{
		// show the eta for reaching the desired ratio
		float desired_ratio = get_stop_ratio() < 0.0f ? get_stop_ratio() : 1.0f;
		if (ratio < desired_ratio)
			eta = status.upload_payload_rate
				? (down * desired_ratio) / status.upload_payload_rate 
				: 0;
		else
			eta = 0;
	}
	else
	{
		if (get_state() & Torrent::STOPPED)
			eta = 0;
		else
			eta = status.download_payload_rate
				? (status.total_wanted - status.total_wanted_done) / status.download_payload_rate
				: 0;
	}
	return eta;
	
}

libtorrent::size_type Torrent::get_total_downloaded()
{
	return get_status().total_payload_download + get_previously_downloaded();
}
libtorrent::size_type Torrent::get_total_uploaded()
{
	return get_status().total_payload_upload + get_previously_uploaded();
}

float Torrent::get_progress()
{
	if (get_state() & Torrent::SEEDING || get_state() & Torrent::FINISHED)
	{
		/// TODO: if preference is set
		float ratio = (1.0f*get_total_uploaded())/(1.0f*get_total_downloaded());
		float desired_ratio = get_stop_ratio() > 0.0f ? get_stop_ratio() : 1.0f;
		if (ratio < desired_ratio)
			return (ratio / desired_ratio) * 100;
		else
			return 100;
	}
	return get_status().progress*100;
}
