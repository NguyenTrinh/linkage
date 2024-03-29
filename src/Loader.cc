/*
Copyright (C) 2008	Christian Lundgren

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

#include "Loader.hh"

using namespace Linkage;

class load : public std::unary_function<const std::string&, void>
{
	Loader* loader;
	Loader::ResumeList resumes;
public:
	void operator()(const std::string& hash_str)
	{
		std::string file = Glib::build_filename(get_data_dir(), hash_str);
		/* only load torrents that has a .resume file */
		if (Glib::file_test(file + ".resume", Glib::FILE_TEST_EXISTS))
		{
			Loader::ResumePair p = loader->load_torrent(file);
			if (p.first)
				resumes.push_back(p);
		}
	}

	Loader::ResumeList get_resumes() { return resumes; }

	load(Loader* l) { loader = l; }
};

class pair_pred
: public std::binary_function<const Loader::ResumePair&, const Loader::ResumePair&, bool>
{
public:
	bool operator()(const Loader::ResumePair& rhs, const Loader::ResumePair& lhs)
	{
		return rhs.first->get_position() < lhs.first->get_position();
	}
};

Loader::Loader() : thread(NULL)
{
}

Loader::~Loader()
{
}

void Loader::launch()
{
	thread = Glib::Thread::create(
		sigc::mem_fun(this, &Loader::load_torrents),
		true);
}

void Loader::join()
{
	thread->join();
	thread = NULL;
}

Glib::Dispatcher& Loader::signal_done()
{
	return m_signal_done;
}

void Loader::load_torrents()
{
	ResumeList resumes;

	/* load torrent info from disk cache */
	Glib::Dir dir(get_data_dir());
	resumes = std::for_each(dir.begin(), dir.end(), load(this)).get_resumes();

	std::sort(resumes.begin(), resumes.end(), pair_pred());

	/* resume previously active torrents */
	for (ResumeList::iterator iter = resumes.begin(); iter != resumes.end(); ++iter)
	{
		/* FIXME: this is not thread safe */
		Engine::get_session_manager()->resume_torrent(iter->first, iter->second);
	}

	m_signal_done.emit();
}

Loader::ResumePair Loader::load_torrent(const std::string& file)
{
	libtorrent::entry e, er;

	/* skip if we can't load torrent file */
	if (!load_entry(file, e))
		return std::make_pair(TorrentPtr(), libtorrent::entry());

	Torrent::InfoPtr info(new libtorrent::torrent_info(e));

	/* signal if we can't load resume file */
	if (!load_entry(file + ".resume", er))
	{
		failed.push_back(info);
		return std::make_pair(TorrentPtr(), libtorrent::entry());
	}

	/* make sure we have not lost track of the content */
	if (!er.find_key("path"))
	{
		failed.push_back(info);
		return std::make_pair(TorrentPtr(), libtorrent::entry());
	}

	/* FIXME: this is not thread safe */
	TorrentPtr torrent = Engine::get_torrent_manager()->add_torrent(er, info);
	if (er.find_key("stopped") && !er["stopped"].integer())
		return std::make_pair(torrent, er);

	return std::make_pair(TorrentPtr(), libtorrent::entry());
}

const Loader::FailList& Loader::get_failed()
{
	assert(thread == NULL);

	return failed;
}

