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

#include "linkage/Torrent.hh"
#include "linkage/Utils.hh"
#include "linkage/Engine.hh"

Torrent::Torrent(const Torrent::ResumeInfo& ri, bool queued) : m_prop_handle(*this, "handle")
{
	m_cur_tier = 0;
	m_announcing = false;

	m_is_queued = queued;

	m_info = ri.info;

	libtorrent::entry::dictionary_type e = ri.resume.dict();
	m_downloaded = e["downloaded"].integer();
	m_uploaded = e["uploaded"].integer();

	m_path = e["path"].string();
	m_position = e["position"].integer();
	if (ri.resume.find_key("group"))
		m_group = e["group"].string();

	m_up_limit = e["upload-limit"].integer();
	m_down_limit = e["download-limit"].integer();

	m_completed = (bool)e["completed"].integer();

	std::list<libtorrent::entry> f;
	try
	{
		f = e["filter"].list();
	}
	catch (std::exception& e) {}

	m_filter.assign(m_info.num_files(), false);
	std::list<libtorrent::entry>::iterator iter = f.begin();
	while (iter != f.end())
	{
		libtorrent::entry e = *iter;
		m_filter[e.integer()] = true;
		iter++;
	}

	// make sure it's a list, old version used dictionary
	if (ri.resume.find_key("trackers") && e["trackers"].type() == libtorrent::entry::list_t)
	{
		libtorrent::entry::list_type e_trackers = e["trackers"].list();
		int tier = 0;
		for (libtorrent::entry::list_type::iterator iter = e_trackers.begin();
			iter != e_trackers.end(); ++iter)
		{
			libtorrent::announce_entry a(iter->string());
			a.tier = tier;
			m_trackers.push_back(a);
			tier++;
		}
	}
	else
		m_trackers = m_info.trackers();

	for (int i = 0; i < m_trackers.size(); i++)
		m_replies[m_trackers[i].url] = "";
}

Torrent::Torrent() : m_prop_handle(*this, "handle")
{
}

Torrent::~Torrent()
{
}

Glib::PropertyProxy<libtorrent::torrent_handle> Torrent::property_handle()
{
	return m_prop_handle.get_proxy();
}

libtorrent::torrent_handle Torrent::get_handle()
{
	return m_prop_handle.get_value();
}

const Glib::ustring Torrent::get_name()
{
	return m_info.name();
}

const Glib::ustring& Torrent::get_group()
{
	return m_group;
}

const Glib::ustring& Torrent::get_path()
{
	return m_path;
}

const std::pair<Glib::ustring, Glib::ustring> Torrent::get_tracker_reply()
{
	static int offset = 0;

	ReplyMap::const_iterator iter = m_replies.begin();
	for (int i = 0; i < offset; i++)
		iter++;
	if (iter == m_replies.end())
		iter = m_replies.begin();

	offset = (offset + 1) % m_replies.size();

	return *iter;
}

const unsigned int Torrent::get_position()
{
	return m_position;
}

const std::vector<bool>& Torrent::get_filter()
{
	return m_filter;
}

const int Torrent::get_up_limit()
{
	return m_up_limit;
}

const int Torrent::get_down_limit()
{
	return m_down_limit;
}

const libtorrent::sha1_hash Torrent::get_hash()
{
	return m_info.info_hash();
}

const libtorrent::size_type Torrent::get_total_downloaded()
{
	libtorrent::size_type total = m_downloaded;
	if (!is_stopped())
		total += get_handle().status().total_download;
	return total;
}

const libtorrent::size_type Torrent::get_total_uploaded()
{
	libtorrent::size_type total = m_uploaded;
	if (!is_stopped())
		total += get_handle().status().total_upload;
	return total;
}

bool Torrent::get_completed()
{
	return m_completed;
}

const Torrent::State Torrent::get_state()
{
	if (!is_stopped())
	{
		libtorrent::torrent_status status = get_handle().status();

		/* libtorrent only says it's seeding after it's announced to the tracker */
		if (status.total_done == m_info.total_size())
			return SEEDING;

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
			/* libtorrent paused this handle, something bad happened */
			else
				return ERROR;
		}

		switch (state)
		{
			case libtorrent::torrent_status::queued_for_checking:
				return CHECK_QUEUE;
			case libtorrent::torrent_status::checking_files:
				return CHECKING;
			case libtorrent::torrent_status::connecting_to_tracker:
				return ANNOUNCING;
			case libtorrent::torrent_status::downloading:
				return DOWNLOADING;
			case libtorrent::torrent_status::finished:
				return FINISHED;
			case libtorrent::torrent_status::seeding:
				return SEEDING;
			case libtorrent::torrent_status::allocating:
				return ALLOCATING;
		}
	}
	else
		return STOPPED;
}

const Glib::ustring Torrent::get_state_string()
{
	return get_state_string(get_state());
}

const Glib::ustring Torrent::get_state_string(State state)
{
	switch (state)
	{
		case ERROR:
			return "Error";
		case QUEUED:
			return "Queued";
		case STOPPED:
			return "Stopped";
		case CHECK_QUEUE:
			return "Queued for checking";
		case CHECKING:
			return "Checking";
		case ANNOUNCING:
			return "Announcing";
		case FINISHED:
			return "Finished";
		case SEEDING:
			return "Seeding";
		case ALLOCATING:
			return "Allocating";
		case DOWNLOADING:
		default:
			return "Downloading";
	}
}

const libtorrent::torrent_info& Torrent::get_info()
{
	return m_info;
}

const libtorrent::torrent_status Torrent::get_status()
{
	if (!is_stopped())
		return get_handle().status();
	else
		return libtorrent::torrent_status();
}

const std::vector<libtorrent::partial_piece_info> Torrent::get_download_queue()
{
	std::vector<libtorrent::partial_piece_info> queue;
	if (!is_stopped())
		get_handle().get_download_queue(queue);

	return queue;
}

const std::vector<float> Torrent::get_file_progress()
{
	std::vector<float> fp;
	if (!is_stopped())
		get_handle().file_progress(fp);
	else
		fp.assign(m_info.num_files(), 0);

	return fp;
}

void Torrent::set_handle(const libtorrent::torrent_handle& handle)
{
	m_prop_handle = handle;

	set_filter(m_filter);
	set_up_limit(m_up_limit);
	set_down_limit(m_down_limit);

	/* Reset all replies on stop */
	if (is_stopped())
	{
		for (ReplyMap::iterator iter = m_replies.begin(); iter != m_replies.end(); ++iter)
			iter->second = "";
		m_cur_tier = 0;
	}
	else
		get_handle().replace_trackers(m_trackers);
}

void Torrent::set_group(const Glib::ustring& group)
{
	m_group = group;
}

void Torrent::set_path(const Glib::ustring& path)
{
	if (m_path != path)
	{
		libtorrent::sha1_hash hash = m_info.info_hash();

		bool stopped = is_stopped();
		if (!stopped)
			Engine::get_session_manager()->stop_torrent(hash);

		m_path = path;
		libtorrent::entry e = get_resume_entry(false);
		save_entry(Glib::build_filename(get_data_dir(), str(hash) + ".resume"), e);

		if (!stopped)
			Engine::get_session_manager()->resume_torrent(hash);
	}
}

void Torrent::set_tracker_reply(const Glib::ustring& reply, const Glib::ustring& tracker)
{
	if (is_stopped())
		return;


	/* Get trackers from handle, since the list might have changed */
	std::vector<libtorrent::announce_entry> trackers = get_handle().trackers();

	if (tracker.empty())
	{
		for (int j = 0; j < trackers.size(); j++)
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
	if (Glib::str_has_prefix(reply, "OK, got "))
	{
		m_announcing = false;
		m_cur_tier = 0;
	}
	/* All trackers failed, cycle stops */
	else if (m_cur_tier == 0 && reply != "Announcing")
		m_announcing = false;
	else
		m_announcing = true;
}

void Torrent::set_position(unsigned int position)
{
	int diff = m_position - position;
	m_position = position;
	Engine::get_torrent_manager()->set_torrent_position(m_info.info_hash(), diff);
}

void Torrent::set_filter(const std::vector<bool>& filter)
{
	if (filter != m_filter)
		m_filter.assign(filter.begin(), filter.end());

	/* TODO: Thread this? It completly freezes UI on large files.. */
	if (!is_stopped())
		get_handle().filter_files(m_filter);
}

void Torrent::filter_file(unsigned int index, bool filter)
{
	m_filter[index] = filter;
	set_filter(m_filter);
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

void Torrent::set_total_downloaded(libtorrent::size_type bytes)
{
	m_downloaded = bytes;
}

void Torrent::set_total_uploaded(libtorrent::size_type bytes)
{
	m_uploaded = bytes;
}

void Torrent::set_completed(bool completed)
{
	m_completed = completed;
}

void Torrent::add_tracker(const Glib::ustring& url)
{
	bool stopped = is_stopped();
	if (!stopped)
	{
		/* Check if someone else changed the trackers */
		bool diff = false;
		std::vector<libtorrent::announce_entry> trackers = get_handle().trackers();
		if (m_trackers.size() < trackers.size())
			diff = true;
		else if (m_trackers.size() == trackers.size())
		{
			for (int i = 0; i < m_trackers.size() && !diff; i++)
			{
				for (int j = 0; j < trackers.size(); j++)
				{
					if (m_trackers[i].url == trackers[j].url)
					{
						diff = (m_trackers[i].tier != trackers[j].tier);
						if (diff)
							break;
					}
					else if (j == trackers.size() - 1)
					{
						diff = true;
						break;
					}
				}
			}
		}

		if (diff)
			m_trackers = trackers;
	}

	libtorrent::announce_entry a(url);
	a.tier = m_trackers.size();
	m_trackers.push_back(a);

	if (!stopped)
		get_handle().replace_trackers(m_trackers);
}

const std::vector<libtorrent::announce_entry>& Torrent::get_trackers()
{
	return m_trackers;
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
	if (is_stopped())
		return;

	if (m_announcing)
		return;
	else
		m_announcing = true;

	std::vector<libtorrent::announce_entry> trackers = get_handle().trackers();
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
		for (int i = 1; i < trackers.size(); i++)
		{
			if (trackers[i].tier < m_cur_tier)
				trackers[i].tier++;
		}

		m_trackers = trackers;

		get_handle().replace_trackers(m_trackers);
	}

	m_cur_tier = 0;
	get_handle().force_reannounce();
}

const libtorrent::entry Torrent::get_resume_entry(bool running)
{
	libtorrent::entry::dictionary_type resume_entry;

	if (!is_stopped())
	{
		if (!get_handle().is_paused())
		{
			/* FIXME: This gives torrent State::Error (for a brief moment) */
			get_handle().pause();
			m_downloaded += get_handle().status().total_download;
			m_uploaded += get_handle().status().total_upload;
		}

		try
		{
			resume_entry = get_handle().write_resume_data().dict();
		}
		catch (std::exception& e)
		{
			g_warning(e.what());
		}
	}
	else
	{
		std::ifstream in;
		Glib::ustring file = Glib::build_filename(get_data_dir(), str(m_info.info_hash()) + ".resume");
		try
		{
			in.open(file.c_str(), std::ios_base::binary);
			in.unsetf(std::ios_base::skipws);
			libtorrent::entry er = libtorrent::bdecode(std::istream_iterator<char>(in), std::istream_iterator<char>());
			resume_entry = er.dict();
		}
		catch (std::exception& e)
		{
			g_warning(e.what());
		}
		in.close();
	}

	if (resume_entry.empty())
	{
		resume_entry["info-hash"] = std::string(m_info.info_hash().begin(), m_info.info_hash().end());
	}

	resume_entry["path"] = m_path;
	resume_entry["position"] = m_position;
	resume_entry["running"] = running;
	resume_entry["downloaded"] = m_downloaded;
	resume_entry["uploaded"] = m_uploaded;
	resume_entry["download-limit"] = m_down_limit;
	resume_entry["upload-limit"] = m_up_limit;
	resume_entry["completed"] = m_completed;

	libtorrent::entry::list_type e_filter;
	for (int i = 0; i < m_filter.size(); i++)
		if (m_filter[i])
			e_filter.push_back(i);
	resume_entry["filter"] = e_filter;

	if (!m_group.empty())
		resume_entry["group"] = m_group;

	libtorrent::entry::list_type e_trackers;
	for (int i = 0; i < m_trackers.size(); i++)
		e_trackers.push_back(m_trackers[i].url);
	resume_entry["trackers"] = e_trackers;

	return resume_entry;
}
