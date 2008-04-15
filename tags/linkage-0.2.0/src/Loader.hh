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

#ifndef LOADER_HH
#define LOADER_HH

#include "linkage/Engine.hh"
#include "linkage/SessionManager.hh"
#include "linkage/TorrentManager.hh"
#include "linkage/Torrent.hh"
#include "linkage/Utils.hh"

#include <glibmm/thread.h>
#include <glibmm/dispatcher.h>


class Loader
{
public:
	typedef std::list<Linkage::Torrent::InfoPtr> FailList;	
	typedef std::pair<Linkage::TorrentPtr, libtorrent::entry> ResumePair;
	typedef std::vector<ResumePair> ResumeList;

private:
	Glib::Dispatcher m_signal_done;
	Glib::Thread* thread;

	FailList failed;

	friend class load;

	void load_torrents();
	ResumePair load_torrent(const std::string& file);

public:
	void launch();
	void join();
	Glib::Dispatcher& signal_done();

	
	const FailList& get_failed();

	Loader();
	~Loader();
};

#endif /* LOADER_HH */

