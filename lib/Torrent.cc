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

Torrent::Torrent(const Torrent::ResumeInfo& ri, bool queued)
{
	m_is_queued = queued;

	m_info = ri.info;

	entry::dictionary_type e = ri.resume.dict();
	m_downloaded = e["downloaded"].integer();
	m_uploaded = e["uploaded"].integer();

	m_path = e["path"].string();
	m_position = e["position"].integer();
	if (ri.resume.find_key("group"))
		m_group = e["group"].string();

	m_up_limit = e["upload-limit"].integer();
	m_down_limit = e["download-limit"].integer();

	/*
		A note for the curious, the "filter" key in the resume file
		stores the indices of the files that are filtered
	*/
	std::list<entry> fl;
	try
	{
		fl = e["filter"].list();
	}
	catch (std::exception& e) {}

	m_filter.assign(m_info.num_files(), false);
	for (std::list<entry>::iterator iter = fl.begin(); iter != fl.end(); ++iter)
	{
		entry e = *iter;
		m_filter[e.integer()] = true;
	}
}

Torrent::~Torrent()
{
}

const torrent_handle& Torrent::get_handle() const
{
	return m_handle;
}

const Glib::ustring Torrent::get_name() const
{
	return m_info.name();
}

const Glib::ustring& Torrent::get_group() const
{
	return m_group;
}

const Glib::ustring& Torrent::get_path() const
{
	return m_path;
}

const Glib::ustring& Torrent::get_tracker_reply() const
{
	return m_tracker_reply;
}

const unsigned int Torrent::get_position() const
{
	return m_position;
}

const std::vector<bool>& Torrent::get_filter() const
{
	return m_filter;
}

const int Torrent::get_up_limit() const
{
	return m_up_limit;
}

const int Torrent::get_down_limit() const
{
	return m_down_limit;
}

const sha1_hash Torrent::get_hash() const
{
	return m_info.info_hash();
}

const size_type Torrent::get_total_downloaded() const
{
	size_type total = m_downloaded;
	if (m_handle.is_valid())
		total += m_handle.status().total_download;
	return total;
}

const size_type Torrent::get_total_uploaded() const
{
	size_type total = m_uploaded;
	if (m_handle.is_valid())
		total += m_handle.status().total_upload;
	return total;
}

const Torrent::State Torrent::get_state() const
{
	if (m_handle.is_valid())
	{
		torrent_status status = m_handle.status();
		/* libtorrent only says it's seeding after it's announced to the tracker */
		if (status.total_done == m_info.total_size())
			return SEEDING;

		if (m_handle.is_paused() && m_is_queued)
			return QUEUED;
		else if (m_handle.is_paused()) /* libtorrent paused this handle, something bad happened */
			return ERROR;

		unsigned int state = status.state;
		switch (state)
		{
			case torrent_status::queued_for_checking:
				return CHECK_QUEUE;
			case torrent_status::checking_files:
				return CHECKING;
			case torrent_status::connecting_to_tracker:
				return ANNOUNCING;
			case torrent_status::downloading:
					return DOWNLOADING;
			case torrent_status::finished:
				return FINISHED;
			case torrent_status::seeding:
				return SEEDING;
			case torrent_status::allocating:
				return ALLOCATING;
		}
	}
	else
		return STOPPED;
}

const Glib::ustring Torrent::get_state_string() const
{
	return get_state_string(get_state());
}

const Glib::ustring Torrent::get_state_string(State state) const
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
			return "Queued";
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

const torrent_info& Torrent::get_info() const
{
	return m_info;
}

const torrent_status Torrent::get_status() const
{
	if (m_handle.is_valid())
		return m_handle.status();
	else
		return torrent_status();
}

const std::vector<partial_piece_info> Torrent::get_download_queue() const
{
	std::vector<partial_piece_info> queue;
	if (m_handle.is_valid())
		m_handle.get_download_queue(queue);

	return queue;
}

const std::vector<float> Torrent::get_file_progress()
{
	std::vector<float> fp;
	if (m_handle.is_valid())
		m_handle.file_progress(fp);
	else
		fp.assign(m_info.num_files(), 0);

	return fp;
}

void Torrent::set_handle(const torrent_handle& handle)
{
	m_handle = handle;

	if (m_handle.is_valid())
		m_handle.filter_files(m_filter);
}

void Torrent::set_group(const Glib::ustring& group)
{
	m_group = group;
}

void Torrent::set_tracker_reply(const Glib::ustring& reply)
{
	m_tracker_reply = reply;
}

void Torrent::set_position(unsigned int position)
{
	Direction direction = (position < m_position) ? DIR_UP : DIR_DOWN;
	m_position = position;
	Engine::get_torrent_manager()->set_torrent_position(m_info.info_hash(), direction);
}

void Torrent::set_filter(std::vector<bool>& filter)
{
	if (filter != m_filter)
		m_filter.assign(filter.begin(), filter.end());

	/* TODO: Thread this? It completly freezes UI on large files.. */
	if (m_handle.is_valid())
		m_handle.filter_files(m_filter);
}

void Torrent::filter_file(unsigned int index, bool filter)
{
	torrent_info info = m_handle.get_torrent_info();

	m_filter[index] = filter;

	set_filter(m_filter);
}

void Torrent::set_up_limit(int limit)
{
	m_up_limit = limit;
	if (m_handle.is_valid())
		m_handle.set_upload_limit(m_up_limit*1024);
}

void Torrent::set_down_limit(int limit)
{
	m_down_limit = limit;
	if (m_handle.is_valid())
		m_handle.set_download_limit(m_down_limit*1024);
}

void Torrent::queue()
{
	if (m_handle.is_valid())
	{
		m_handle.pause();
		m_is_queued = true;
	}
}

void Torrent::unqueue()
{
	if (m_handle.is_valid())
	{
		m_handle.resume();
		m_is_queued = false;
	}
}

bool Torrent::is_queued()
{
	return m_is_queued;
}

bool Torrent::is_running()
{
	m_handle.is_valid();
}

const entry Torrent::get_resume_entry(bool running)
{
	entry::dictionary_type resume_entry;

	/* This shouldn't happen */
	if (m_handle.is_valid())
	{
		if (!m_handle.is_paused())
		{
			m_handle.pause();
			m_downloaded += m_handle.status().total_download;
			m_uploaded += m_handle.status().total_upload;
		}

		try
		{
			resume_entry = m_handle.write_resume_data().dict();
		}
		catch (std::exception& e)
		{
			g_warning(e.what());

			resume_entry["info-hash"] = std::string(m_info.info_hash().begin(), m_info.info_hash().end());
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
			entry er = bdecode(std::istream_iterator<char>(in), std::istream_iterator<char>());
			in.close();
			resume_entry = er.dict();
		}
		catch (std::exception& e)
		{
			g_warning(e.what());

			resume_entry["info-hash"] = std::string(m_info.info_hash().begin(), m_info.info_hash().end());
		}
	}

	resume_entry["path"] = m_path;
	resume_entry["position"] = m_position;
	resume_entry["running"] = running;
	resume_entry["downloaded"] = m_downloaded;
	resume_entry["uploaded"] = m_uploaded;
	resume_entry["download-limit"] = m_down_limit;
	resume_entry["upload-limit"] = m_up_limit;
	entry::list_type el;
	for (unsigned int i = 0; i < m_filter.size(); i++)
		if (m_filter[i])
			el.push_back(i);
	resume_entry["filter"] = el;
	if (!m_group.empty())
		resume_entry["group"] = m_group;

	return resume_entry;
}
