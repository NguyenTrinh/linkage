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

/* dbus-c++ defines these ... */
#ifdef HAVE_CONFIG_H 
#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef VERSION

#include "config.h"
#endif

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

SessionManagerPtr SessionManager::create()
{
	return SessionManagerPtr(new SessionManager());
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

	/* apply settings on startup */
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
	SettingsManagerPtr sm = Engine::get_settings_manager();

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
	SettingsManagerPtr sm = Engine::get_settings_manager();

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
	SettingsManagerPtr sm = Engine::get_settings_manager();

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
		catch (std::exception& ex)
		{
			g_warning(_("Listen failed: %s"), ex.what());
		}
	}

	#ifndef TORRENT_DISABLE_DHT
	dht_settings settings;
	settings.service_port = listen_port();

	Glib::ustring file = Glib::build_filename(get_config_dir(), "dht.resume");
	entry e;
	if (Glib::file_test(file, Glib::FILE_TEST_EXISTS))
		load_entry(file, e);

	if (sm->get_bool("network/use_dht"))
	{
		set_dht_settings(settings);
		try
		{
			start_dht(e);
		}
		catch (asio::system_error& ex)
		{
			/* FIXME: notify user about failure */
		}
		/* TODO: check if we are allowed to use these routers */
		add_dht_router(std::pair<std::string, int>("router.bittorrent.com", 6881));
		add_dht_router(std::pair<std::string, int>("router.utorrent.com", 6881));
		add_dht_router(std::pair<std::string, int>("router.bitcomet.net", 554));
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

void SessionManager::on_torrent_finished(const sha1_hash& hash, const Glib::ustring& msg)
{
	if (Engine::get_settings_manager()->get_bool("files/move_finished"))
	{
		//FIXME: 0.13 uses an alert for move_storage, also set new path to torrent
		Glib::ustring path = Engine::get_settings_manager()->get_string("files/finished_path");
		TorrentPtr torrent = Engine::get_torrent_manager()->get_torrent(hash);
		if (!torrent->is_stopped())
		{
			torrent->get_handle().move_storage(path.c_str());
			// FIXME: add wait for alert
			// FIXME: set path only on success
			torrent->set_path(path);
		}
	}
}

void SessionManager::resume_torrent(const TorrentPtr& torrent, const entry& resume)
{
	g_return_if_fail(torrent);

	/* simply unpause torrents in error state */
	if (torrent->get_state() & Torrent::ERROR)
	{
		torrent->get_handle().resume();
		return;
	}

	entry er;
	if (resume.type() == entry::undefined_t)
	{
		Glib::ustring file = Glib::build_filename(get_data_dir(), 
			String::compose("%1", torrent->get_hash()) + ".resume");
		/* FIXME: handle failed load_entry */
		load_entry(file, er);
	}
	else
		er = resume;

	storage_mode_t storage_mode = storage_mode_sparse;
	if (Engine::get_settings_manager()->get_bool("files/allocate"))
		storage_mode = storage_mode_allocate;


	torrent_handle handle = add_torrent(torrent->get_info(),
		torrent->get_path().c_str(),
		er,
		storage_mode);
	torrent->set_handle(handle);
}

TorrentPtr SessionManager::open_torrent(
	const Glib::ustring& file,
	const Glib::ustring& save_path)
{
	std::vector<char> buff;
	entry e, er;

	if (!load_entry(file, e, buff))
		return TorrentPtr();

	Torrent::InfoPtr info(new torrent_info(e));
	sha1_hash hash = info->info_hash();

	TorrentPtr torrent = Engine::get_torrent_manager()->get_torrent(hash);
	/* merge trackers and return if torrent exists */
	if (torrent)
	{
		std::vector<announce_entry> trackers = info->trackers();
		for (unsigned int i = 0; i < trackers.size(); i++)
		{
			torrent->add_tracker(trackers[i].url);
		}
		m_signal_duplicate_torrent.emit(String::ucompose(_(
			"Merged %1 with %2"), info->name(), torrent->get_name()), hash);
		return torrent;
	}

	/* use a .resume file if we find a valid one */
	Glib::ustring resume_file = Glib::build_filename(get_data_dir(),
		String::compose("%1", hash) + ".resume");
	if (Glib::file_test(resume_file, Glib::FILE_TEST_EXISTS))
	{
		load_entry(resume_file, er);
		if (!(er.find_key("path") && er["path"].string() == save_path))
			er = entry();
	}

	storage_mode_t storage_mode = storage_mode_sparse;
	if (Engine::get_settings_manager()->get_bool("files/allocate"))
		storage_mode = storage_mode_allocate;

	torrent_handle handle = add_torrent(info, save_path.c_str(), er, storage_mode);

	if (er.type() == entry::undefined_t)
		er["path"] = save_path;

	torrent = Engine::get_torrent_manager()->add_torrent(er, info);

	/* see below, get entry before we set the handle */
	er = torrent->get_resume_entry();
	er["stopped"] = 0;

	torrent->set_handle(handle);

	/* Save metadata to ~/.config/linkage/data/hash */
	Glib::ustring metafile = Glib::build_filename(get_data_dir(), String::compose("%1", hash));
	std::ofstream out;
	out.open(metafile.c_str(), std::ios_base::binary);
	out.write(&buff[0], buff.size());
	out.close();

	/* Save an almost empty .resume file, so the torrent is resumed next session
	 * even if something nasty happens to this session
	 */
	save_entry(Glib::build_filename(get_data_dir(), String::compose("%1", hash) + ".resume"), er);

	return torrent;
}

void SessionManager::stop_torrent(const TorrentPtr& torrent)
{
	g_return_if_fail(torrent);

	if (torrent->is_stopped())
		return;

	// to make sure Torrent::m_is_queued is false
	if (torrent->is_queued())
		torrent->unqueue();

	torrent->get_handle().pause();
	entry e = torrent->get_resume_entry();
	e["stopped"] = 1;

	Glib::ustring file = Glib::build_filename(get_data_dir(),
		String::compose("%1", torrent->get_hash()) + ".resume");
	save_entry(file, e);

	/* notify TorrentManager and Torrent before we invalidate the old handle */
	torrent_handle handle = torrent->get_handle();
	torrent->set_handle(torrent_handle());

	remove_torrent(handle);
}

void SessionManager::erase_torrent(const TorrentPtr& torrent, bool erase_content)
{
	g_return_if_fail(torrent);

	int options = none;
	if (erase_content)
		options = delete_files;

	Engine::get_torrent_manager()->remove_torrent(torrent);

	if (!torrent->is_stopped())
		remove_torrent(torrent->get_handle(), options);

	/* remove metadata and .resume file */
	Glib::ustring file = Glib::build_filename(get_data_dir(),
		String::compose("%1", torrent->get_hash()));
	g_unlink(file.c_str());
	g_unlink((file + ".resume").c_str());
}

void SessionManager::recheck_torrent(const TorrentPtr& torrent)
{
	g_return_if_fail(torrent);

	if (!torrent->is_stopped())
		stop_torrent(torrent);

	Torrent::InfoPtr info = torrent->get_info();
	Glib::ustring path = torrent->get_path();
	storage_mode_t storage_mode = storage_mode_sparse;
	if (Engine::get_settings_manager()->get_bool("files/allocate"))
		storage_mode = storage_mode_allocate;
	torrent_handle handle = add_torrent(info, path.c_str(), entry(), storage_mode);
	torrent->set_handle(handle);
}

