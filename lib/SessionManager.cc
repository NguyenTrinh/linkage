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

#include "linkage/SessionManager.hh"
#include "linkage/Engine.hh"
#include "linkage/AlertManager.hh"
#include "linkage/TorrentManager.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/Torrent.hh"
#include "linkage/Utils.hh"
#include "linkage/compose.hpp"

#include <libtorrent/extensions/ut_pex.hpp>

#include <glib/gstdio.h>
#include <glibmm/fileutils.h>
#include <glibmm/i18n.h>

using namespace libtorrent;
using namespace Linkage;

static GConfEnumStringPair enc_policy_table[] = {
  { pe_settings::forced, "POLICY_FORCED" },
  { pe_settings::enabled, "POLICY_ENABLED" },
  { pe_settings::disabled, "POLICY_DISABLED" }
};

static GConfEnumStringPair enc_level_table[] = {
  { pe_settings::plaintext, "LEVEL_PLAINTEXT" },
  { pe_settings::rc4, "LEVEL_RC4" },
  { pe_settings::both, "LEVEL_BOTH" }
};

static GConfEnumStringPair proxy_type_table[] = {
  { proxy_settings::none, "PROXY_NONE" },
  { proxy_settings::socks4, "PROXY_SOCKS4" },
  { proxy_settings::socks5, "PROXY_SOCKS5" },
  { proxy_settings::socks5_pw, "PROXY_SOCKS5_PW" },
  { proxy_settings::http, "PROXY_HTTP" },
  { proxy_settings::http_pw, "PROXY_HTTP_PW" }
};

Glib::RefPtr<SessionManager> SessionManager::create()
{
	return Glib::RefPtr<SessionManager>(new SessionManager());
}

SessionManager::SessionManager()
	:	session(fingerprint(
			"LK", 
			LINKAGE_VERSION_MAJOR, 
			LINKAGE_VERSION_MINOR, 
			LINKAGE_VERSION_MICRO, 
			0))
{
	set_severity_level(alert::info);
	Engine::get_alert_manager()->signal_torrent_finished().connect(sigc::mem_fun(this, &SessionManager::on_torrent_finished));
	Engine::get_settings_manager()->signal_key_changed().connect(sigc::mem_fun(this, &SessionManager::on_key_changed));
	//Apply settings on startup
	update_session_settings();
	update_proxy_settings();
	update_pe_settings();
}

SessionManager::~SessionManager()
{
	#ifndef TORRENT_DISABLE_DHT
	if (Engine::get_settings_manager()->get_bool("network/use_dht"))
	{
		entry e = dht_state();
		Glib::ustring file = Glib::build_filename(get_config_dir(), "dht.resume");
		save_entry(file, e);
	}
	#endif

	for (std::list<Glib::Thread*>::iterator iter = m_threads.begin();
				iter != m_threads.end(); ++iter)
	{
		Glib::Thread* thread = *iter;
		thread->join();
	}
	m_threads.clear();
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

void SessionManager::on_key_changed(const Glib::ustring& key, const Value& value)
{
	if (Glib::str_has_prefix(key, "network/encryption/"))
		update_pe_settings();
	else if (Glib::str_has_prefix(key, "network/proxy/"))
		update_proxy_settings();
	else
		update_session_settings();
}

void SessionManager::update_pe_settings()
{
	Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();

	pe_settings pe;
	pe_settings::enc_policy policy = (pe_settings::enc_policy)sm->get_enum("network/encryption/policy", enc_policy_table);
	pe.out_enc_policy = policy;
	pe.in_enc_policy = policy;
	pe_settings::enc_level level = (pe_settings::enc_level)sm->get_enum("network/encryption/level", enc_level_table);
	pe.allowed_enc_level = level;
	pe.prefer_rc4 = sm->get_bool("network/encryption/prefer_rc4");
	set_pe_settings(pe);
}

void SessionManager::update_proxy_settings()
{
	Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();

	Glib::ustring proxy = sm->get_string("network/proxy/host");
	proxy_settings px;
	if (!proxy.empty())
	{
		px.hostname = proxy;
		px.port = sm->get_int("network/proxy/port");
		px.username = sm->get_string("network/proxy/login");
		px.password = sm->get_string("network/proxy/pass");
		proxy_settings::proxy_type type = (proxy_settings::proxy_type)sm->get_enum("network/proxy/type", proxy_type_table);
		px.type = type;
	}
	else
		px.type = proxy_settings::none;

	set_peer_proxy(px);
	set_web_seed_proxy(px);
	set_tracker_proxy(px);
	set_dht_proxy(px);
}

void SessionManager::update_session_settings()
{
	Glib::RefPtr<SettingsManager> sm = Engine::get_settings_manager();

	int port = sm->get_int("network/port");
	/* Only call listen_on if we really need to */
	if (!is_listening() || port != listen_port())
	{
		try
		{
			Glib::ustring ip = get_ip(sm->get_string("network/interface"));
			if (!ip.empty())
				listen_on(std::make_pair(port, port), ip.c_str());
			else
				listen_on(std::make_pair(port, port));
		}
		catch (std::exception& e)
		{
			g_warning(_("Listen failed: %s"), e.what());
		}
	}

	#ifndef TORRENT_DISABLE_DHT
	dht_settings settings;
	settings.service_port = listen_port();

	Glib::ustring file = Glib::build_filename(get_config_dir(), "dht.resume");
	entry e;
	if (Glib::file_test(file, Glib::FILE_TEST_EXISTS))
		decode(file, e);

	if (sm->get_bool("network/use_dht"))
	{
		set_dht_settings(settings);
		start_dht(e);
		add_dht_router(std::pair<std::string, int>("router.bittorrent.com", 6881));
		add_dht_router(std::pair<std::string, int>("router.utorrent.com", 6881));
		add_dht_router(std::pair<std::string, int>("router.bitcoment.com", 6881));
	}
	else
		/* FIXME: check if dht is running, if so save state */
		stop_dht();
	#endif /* TORRENT_DISABLE_DHT */

	if (sm->get_bool("network/use_pex"))
		add_extension(&create_ut_pex_plugin);

	int up_rate = sm->get_int("network/max_up_rate")*1024;
	if (up_rate < 1)
		up_rate = -1;
	set_upload_rate_limit(up_rate);

	int down_rate = sm->get_int("network/max_down_rate")*1024;
	if (down_rate < 1)
		down_rate = -1;
	set_download_rate_limit(down_rate);

	int max_uploads = sm->get_int("network/max_uploads");
	if (max_uploads == 0)
		max_uploads = -1;
	set_max_uploads(max_uploads);

	int max_connections = sm->get_int("network/max_connections");
	if (max_connections == 0)
		max_connections = -1;
	set_max_connections(max_connections);

	session_settings sset;
	sset.user_agent = PACKAGE_NAME "/" PACKAGE_VERSION " libtorrent/" LIBTORRENT_VERSION;
	sset.tracker_completion_timeout = sm->get_int("network/tracker_timeout");
	sset.tracker_receive_timeout = sm->get_int("network/tracker_timeout");
	sset.stop_tracker_timeout = sm->get_int("network/tracker_timeout");
	sset.allow_multiple_connections_per_ip = sm->get_bool("network/multiple_connections_per_ip");
	sset.use_dht_as_fallback = sm->get_bool("network/dht_fallback");
	sset.file_pool_size = sm->get_int("files/max_open");
	sset.lazy_bitfields = sm->get_bool("torrent/lazy_bitfields");
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
		m_signal_missing_file.emit(String::ucompose(_("File not found, \"%1\""), file), file);
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
		m_signal_invalid_bencoding.emit(String::ucompose(_("Invalid bencoding in %1"), file), file);
		return false;
	}

	return true;
}

void SessionManager::on_torrent_finished(const sha1_hash& hash, const Glib::ustring& msg)
{
	if (Engine::get_settings_manager()->get_bool("files/move_finished"))
	{
		//FIXME: 0.13 uses an alert for move_storage, also set new path to torrent
		Glib::ustring path = Engine::get_settings_manager()->get_string("files/finished_path");
		Glib::RefPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
		bool ret = false;
		if (!torrent->is_stopped())
			torrent->get_handle().move_storage(path.c_str());
		//if (!ret)
		//	g_warning(_("Failed to move content for %s to %s"), torrent->get_name().c_str(), path.c_str());
	}
}

sha1_hash SessionManager::open_torrent(const Glib::ustring& file,
	const Glib::ustring& save_path)
{
	std::vector<char> buff;
	entry e;

	if (!decode(file, e, buff))
		return INVALID_HASH;

	boost::intrusive_ptr<torrent_info> info(new torrent_info(e));
	sha1_hash hash = info->info_hash();

	entry er;
	Glib::RefPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
	if (!torrent)
	{
		storage_mode_t storage_mode = storage_mode_sparse;
		if (Engine::get_settings_manager()->get_bool("files/allocate"))
			storage_mode = storage_mode_allocate;

		torrent_handle handle;

		// Check if a resume file exists and it's valid
		bool no_resume = true;
		Glib::ustring resume_file = Glib::build_filename(get_data_dir(), String::compose("%1", hash) + ".resume");
		if (Glib::file_test(resume_file, Glib::FILE_TEST_EXISTS))
		{
			decode(resume_file, er);
			if (er.find_key("path") && er["path"].string() == save_path)
			{
				handle = add_torrent(info, save_path.c_str(), er, storage_mode);
				torrent = Engine::get_torrent_manager()->add_torrent(er, info);
				no_resume = false;
			}
		}

		if (no_resume)
		{
			entry::dictionary_type de;
			de["path"] = save_path;
			handle = add_torrent(info, save_path.c_str(), entry(), storage_mode);
			torrent = Engine::get_torrent_manager()->add_torrent(de, info);
		}	

		// see below, get entry before we set the handle
		er = torrent->get_resume_entry(false);

		torrent->set_handle(handle);
	}
	else
	{
		std::vector<announce_entry> trackers = info->trackers();
		for (unsigned int i = 0; i < trackers.size(); i++)
		{
			torrent->add_tracker(trackers[i].url);
		}
		m_signal_duplicate_torrent.emit(String::ucompose(_(
			"Merged %1 with %2"), info->name(), torrent->get_name()), hash);
		return hash;
	}

	/* Save metadata to ~/.config/linkage/data/hash */
	Glib::ustring metafile = Glib::build_filename(get_data_dir(), String::compose("%1", hash));
	std::ofstream out;
	out.open(metafile.c_str(), std::ios_base::binary);
	out.write(&buff[0], buff.size());
	out.close();

	/*
		Save an almost empty .resume file, so the torrent is resumed next session
		even if something nasty happens to this session
	*/
	save_entry(Glib::build_filename(get_data_dir(), String::compose("%1", hash) + ".resume"), er);

	return hash;
}

sha1_hash SessionManager::resume_torrent(const sha1_hash& hash)
{
	Glib::ustring hash_str = String::compose("%1", hash);
	return resume_torrent(hash_str);
}

sha1_hash SessionManager::resume_torrent(const Glib::ustring& hash_str)
{
	std::ifstream in;
	Glib::ustring file = Glib::build_filename(get_data_dir(), hash_str);

	entry e, er;

	if (!decode(file, e))
		return INVALID_HASH;

	boost::intrusive_ptr<torrent_info> info(new torrent_info(e));

	/* Check if torrent is up an running, if so return */
	Glib::RefPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(info->info_hash());
	if (torrent && !torrent->is_stopped())
		return info->info_hash();

	file = file + ".resume";
	/* If this fails we need to handle it better below */
	decode(file, er);

	storage_mode_t storage_mode = storage_mode_sparse;
	if (Engine::get_settings_manager()->get_bool("files/allocate"))
		storage_mode = storage_mode_allocate;
	/* FIXME: make sure that path is not empty */
	Glib::ustring save_path = er.dict()["path"].string();

	if (torrent) //Torrent was resumed from stopped state
	{
		torrent_handle handle = add_torrent(info, save_path.c_str(), er, storage_mode);
		torrent = Engine::get_torrent_manager()->get_torrent(handle.info_hash());
		torrent->set_handle(handle);
	}
	else //Torrent was resumed from previous session
	{
		torrent = Engine::get_torrent_manager()->add_torrent(er, info);
		if (er.find_key("stopped") && !er.dict()["stopped"].integer())
		{
			torrent_handle handle = add_torrent(info, save_path.c_str(), er, storage_mode);
			torrent->set_handle(handle);
		}
	}

	return info->info_hash();
}

void SessionManager::recheck_torrent(const sha1_hash& hash)
{
	Glib::RefPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);
	if (torrent)
	{
		if (!torrent->is_stopped())
			stop_torrent(hash);

		boost::intrusive_ptr<torrent_info> info = torrent->get_info();
		Glib::ustring path = torrent->get_path();
		storage_mode_t storage_mode = storage_mode_sparse;
		if (Engine::get_settings_manager()->get_bool("files/allocate"))
			storage_mode = storage_mode_allocate;
		torrent_handle handle = add_torrent(info, path.c_str(), entry(), storage_mode);
		torrent->set_handle(handle);
	}
}

void SessionManager::stop_torrent(const sha1_hash& hash)
{
	if (Engine::get_torrent_manager()->exists(hash))
	{
		Glib::ustring hash_str = String::compose("%1", hash);
		Glib::RefPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);

		if (torrent->is_stopped())
			return;

		// to make sure Torrent::m_is_queued is false
		if (torrent->is_queued())
			torrent->unqueue();

		torrent->get_handle().pause();
		entry e = torrent->get_resume_entry();

		save_entry(Glib::build_filename(get_data_dir(), String::compose("%1", hash) + ".resume"), e);

		remove_torrent(torrent->get_handle());

		// Make the Torrent and TorrentManager aware of stopping it
		torrent->set_handle(torrent_handle());
	}
}

void SessionManager::erase_torrent(const sha1_hash& hash, bool erase_content)
{
	Glib::RefPtr<Torrent> torrent = Engine::get_torrent_manager()->get_torrent(hash);

	int options = none;
	if (erase_content)
		options = delete_files;

	if (!torrent->is_stopped())
		remove_torrent(torrent->get_handle(), options);

	Engine::get_torrent_manager()->remove_torrent(hash);

	Glib::ustring file = Glib::build_filename(get_data_dir(), String::compose("%1", hash));
	g_unlink(file.c_str());
	g_unlink((file + ".resume").c_str());
}

