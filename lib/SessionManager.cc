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

#include "config.h"

#define LT_012 (LIBTORRENT_VERSION_MINOR == 12)

#include "linkage/SessionManager.hh"
#include "linkage/Engine.hh"
#include "linkage/Utils.hh"

#if LT_012
#include <libtorrent/extensions/ut_pex.hpp>
#endif

#include <glib/gstdio.h>
#include <glibmm/fileutils.h>

Glib::RefPtr<SessionManager> SessionManager::create()
{
	return Glib::RefPtr<SessionManager>(new SessionManager());
}

SessionManager::SessionManager() : RefCounter<SessionManager>::RefCounter(this),
	session(fingerprint("LK", LINKAGE_VERSION_MAJOR, LINKAGE_VERSION_MINOR, LINKAGE_VERSION_MICRO, 0))				
{
	if (!LT_012)
		boost::filesystem::path::default_name_check(boost::filesystem::native);

	set_severity_level(alert::info);

	Engine::get_alert_manager()->signal_torrent_finished().connect(sigc::mem_fun(this, &SessionManager::on_torrent_finished));

	Engine::get_settings_manager()->signal_update_settings().connect(sigc::mem_fun(this, &SessionManager::on_settings));

	on_settings(); //Apply settings on startup
}

SessionManager::~SessionManager()
{
	#ifndef TORRENT_DISABLE_DHT
	if (Engine::get_settings_manager()->get_bool("Network", "UseDHT"))
	{
		entry e = dht_state();
		Glib::ustring file = Glib::build_filename(get_config_dir(), "dht.resume");
		save_entry(file, e);
	}
	#endif
}

sigc::signal<void, const Glib::ustring&, const Glib::ustring&>
SessionManager::signal_invalid_bencoding()
{
	return m_signal_invalid_bencoding;
}

sigc::signal<void, const Glib::ustring&, const Glib::ustring&>
SessionManager::signal_missing_file()
{
	return m_signal_missing_file;
}

sigc::signal<void, const Glib::ustring&, const sha1_hash&>
SessionManager::signal_duplicate_torrent()
{
	return m_signal_duplicate_torrent;
}

void SessionManager::resume_session()
{
	Glib::Dir dir(get_data_dir());
	for (Glib::DirIterator iter = dir.begin(); iter != dir.end(); ++iter)
	{
		Glib::ustring file = Glib::build_filename(get_data_dir(), *iter + ".resume");
		if (Glib::file_test(file, Glib::FILE_TEST_EXISTS))
			resume_torrent((*iter).c_str());
	}
}

void SessionManager::on_settings()
{
	Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();

	Glib::ustring iface = sm->get_string("Network", "Interface");
	ip_address ip;
	int min_port = sm->get_int("Network", "MinPort");
	int max_port = sm->get_int("Network", "MaxPort");

	/* Only call listen_on if we really need to */
	int port = listen_port();
	if (!is_listening() || port < min_port || port > max_port)
	{
		if (get_ip(iface.c_str(), ip))
			listen_on(std::make_pair(min_port, max_port), ip);
		else
			listen_on(std::make_pair(min_port, max_port));
	}

	#ifndef TORRENT_DISABLE_DHT
	dht_settings settings;
	settings.service_port = listen_port();
	set_dht_settings(settings);

	Glib::ustring file = Glib::build_filename(get_config_dir(), "dht.resume");
	entry e;
	if (Glib::file_test(file, Glib::FILE_TEST_EXISTS))
		decode(file, e);

	if (sm->get_bool("Network", "UseDHT"))
	{
		try
		{
			start_dht(e);
			add_dht_router(std::pair<std::string, int>("router.bittorrent.com", 6881));
			add_dht_router(std::pair<std::string, int>("router.utorrent.com", 6881));
			add_dht_router(std::pair<std::string, int>("router.bitcoment.com", 6881));
		}
		catch (asio::error& error)
		{
			g_warning("Failed to start DHT");
		}
	}
	else
		/* FIXME: check if dht is running, if so save state */
		stop_dht();
	#endif /* TORRENT_DISABLE_DHT */

	#ifdef #LT_012
	if (sm->get_bool("Network", "UsePEX"))
		add_extension(&create_ut_pex_plugin);
	#endif

	int up_rate = sm->get_int("Network", "MaxUpRate")*1024;
	if (up_rate < 1)
		up_rate = -1;
	set_upload_rate_limit(up_rate);

	int down_rate = sm->get_int("Network", "MaxDownRate")*1024;
	if (down_rate < 1)
		down_rate = -1;
	set_download_rate_limit(down_rate);

	int max_uploads = sm->get_int("Network", "MaxUploads");
	if (max_uploads == 0)
		max_uploads = -1;
	set_max_uploads(max_uploads);

	int max_connections = sm->get_int("Network", "MaxConnections");
	if (max_connections == 0)
		max_connections = -1;
	set_max_connections(max_connections);

	session_settings sset;
	sset.user_agent = PACKAGE_NAME "/" PACKAGE_VERSION " libtorrent/" LIBTORRENT_VERSION;
	sset.tracker_completion_timeout = sm->get_int("Network", "TrackerTimeout");
	sset.tracker_receive_timeout = sm->get_int("Network", "TrackerTimeout");
	sset.stop_tracker_timeout = sm->get_int("Network", "TrackerTimeout");
	#ifdef #LT_012
	sset.allow_multiple_connections_per_ip = sm->get_bool("Network", "MultipleConnectionsPerIP");
	sset.use_dht_as_fallback = sm->get_int("Network", "DHTFallback");
	sset.file_pool_size = sm->get_int("Files", "MaxOpen");
	#endif
	Glib::ustring proxy = sm->get_string("Network", "ProxyIp");
	if (!proxy.empty())
	{
		sset.proxy_ip = proxy;
		sset.proxy_port = sm->get_int("Network", "ProxyPort");
		sset.proxy_login = sm->get_string("Network", "ProxyLogin");
		sset.proxy_password = sm->get_string("Network", "ProxyPass");
	}
	set_settings(sset);
}

bool SessionManager::decode(const Glib::ustring& file, entry& e)
{
	std::vector<char> buffer;
	return decode(file, e, buffer);
}

bool SessionManager::decode(const Glib::ustring& file,
														entry& e,
														std::vector<char>& buffer)
{
	static std::ifstream in;

	if (!Glib::file_test(file, Glib::FILE_TEST_EXISTS))
	{
		m_signal_missing_file.emit("File not found, \"" + file + "\"", file);
		return false;
	}

	in.open(file.c_str(), std::ios_base::binary|std::ios_base::ate);
	in.unsetf(std::ios_base::skipws);
	std::streampos size = in.tellg();
	in.seekg(0, std::ios::beg);
	buffer.resize(size);

	in.read(&buffer[0], size);
	in.close();

	try
	{
		e = bdecode(buffer.begin(), buffer.end());
	}
	catch (std::exception& err)
	{
		m_signal_invalid_bencoding.emit("Invalid bencoding in " + file, file);
		return false;
	}

	return true;
}

void SessionManager::on_torrent_finished(const sha1_hash& hash, const Glib::ustring& msg)
{
	if (Engine::get_settings_manager()->get_bool("Files", "MoveFinished"))
	{
		Glib::ustring path = Engine::get_settings_manager()->get_string("Files", "FinishedPath");
		WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
		bool ret = false;
		if (!torrent->is_stopped())
			ret = torrent->get_handle().move_storage(path.c_str());
		if (!ret)
			g_warning("Failed to move content for %s to %s", torrent->get_name().c_str(), path.c_str());
	}
}

sha1_hash SessionManager::open_torrent(const Glib::ustring& file,
																			 const Glib::ustring& save_path)
{
	std::vector<char> buff;
	entry e;

	if (!decode(file, e, buff))
		return INVALID_HASH;

	torrent_info info(e);
	sha1_hash hash = info.info_hash();

	entry er;
	WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
	if (!torrent)
	{
		torrent_handle handle = add_torrent(info, save_path.c_str(), entry(),
			!Engine::get_settings_manager()->get_bool("Files", "Allocate"));
		entry::dictionary_type de;
		de["path"] = save_path;

		torrent = Engine::get_torrent_manager()->add_torrent(de, info);
		er = torrent->get_resume_entry(true);
		torrent->set_handle(handle);
	}
	else
	{
		m_signal_duplicate_torrent.emit("Torrent already exists in session as " +
			torrent->get_name(), hash);
		return hash;
	}

	/* Save metadata to ~/.config/linkage/data/hash */
	Glib::ustring metafile = Glib::build_filename(get_data_dir(), str(hash));
	std::ofstream out;
	out.open(metafile.c_str(), std::ios_base::binary);
	out.write(&buff[0], buff.size());
	out.close();

	/*
		Save an almost empty .resume file, so the torrent is resumed next session
		even if something nasty happens to this session
	*/
	save_entry(hash, er, ".resume");

	return hash;
}

sha1_hash SessionManager::resume_torrent(const sha1_hash& hash)
{
	return resume_torrent(str(hash));
}

sha1_hash SessionManager::resume_torrent(const Glib::ustring& hash_str)
{
	std::ifstream in;
	Glib::ustring file = Glib::build_filename(get_data_dir(), hash_str);

	entry e, er;

	if (!decode(file, e))
		return INVALID_HASH;

	torrent_info info = torrent_info(e);
	/* Check if torrent is up an running, if so return */
	WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(info.info_hash());

	if (torrent && !torrent->is_stopped())
		return info.info_hash();

	file = file + ".resume";
	/* If this fails we need to handle it better below */
	decode(file, er);

	bool allocate = Engine::get_settings_manager()->get_bool("Files", "Allocate");
	Glib::ustring save_path = er.dict()["path"].string();

	if (torrent) //Torrent was resumed from stopped state
	{
		torrent_handle handle = add_torrent(info, save_path.c_str(), er, !allocate);
		torrent = Engine::get_torrent_manager()->get_torrent(handle.info_hash());
		torrent->set_handle(handle);
	}
	else //Torrent was resumed from previous session
	{
		torrent = Engine::get_torrent_manager()->add_torrent(er, info);
		if (er.dict()["running"].integer())
		{
			torrent_handle handle = add_torrent(info, save_path.c_str(), er, !allocate);
			torrent->set_handle(handle);
		}
	}

	return info.info_hash();
}

void SessionManager::recheck_torrent(const sha1_hash& hash)
{
	WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
	if (torrent)
	{
		if (!torrent->is_stopped())
			stop_torrent(hash);

		torrent_info info = torrent->get_info();
		Glib::ustring path = torrent->get_path();
		bool allocate = Engine::get_settings_manager()->get_bool("Files", "Allocate");
		torrent_handle handle = add_torrent(info, path.c_str(), entry(), !allocate);
		torrent->set_handle(handle);
	}
}

void SessionManager::stop_torrent(const sha1_hash& hash)
{
	if (Engine::get_torrent_manager()->exists(hash))
	{
		Glib::ustring hash_str = str(hash);
		WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);

		if (torrent->is_stopped())
			return;

		if (torrent->is_queued())
			torrent->unqueue();

		entry e = torrent->get_resume_entry();

		save_entry(hash, e, ".resume");

		remove_torrent(torrent->get_handle());
		/* FIXME: this is pretty stupid, just to get the TorrentManager's attention */
		torrent->set_handle(torrent_handle());
	}
}

void SessionManager::erase_torrent(const sha1_hash& hash, bool erase_content)
{
	WeakPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);

	if (!torrent->is_stopped())
		remove_torrent(torrent->get_handle());

	if (erase_content)
	{
		Glib::ustring root = torrent->get_path();
		torrent_info info = torrent->get_info();

		for (torrent_info::file_iterator iter = info.begin_files();
					iter != info.end_files(); ++iter)
		{
			file_entry fe = *iter;
			Glib::ustring file = fe.path.string();
			g_remove(Glib::build_filename(root, file).c_str());

			/* Try to remove parent dir */
			if (file.find("/") != Glib::ustring::npos)
				g_remove(Glib::build_filename(root, Glib::path_get_dirname(file)).c_str());
		}

		/* Multi file torrents have their own root folder */
		if (info.num_files() > 1)
			g_remove(Glib::build_filename(root, info.name()).c_str());
	}

	Engine::get_torrent_manager()->remove_torrent(hash);

	Glib::ustring file = Glib::build_filename(get_data_dir(), str(hash));
	g_unlink(file.c_str());
	g_unlink((file + ".resume").c_str());
}
