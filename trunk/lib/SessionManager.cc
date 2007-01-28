/*
Copyright (C) 2006  Christian Lundgren

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "linkage/SessionManager.hh"
#include "linkage/Engine.hh"
#include "linkage/Utils.hh"

#include <glib/gstdio.h>
#include <glibmm/fileutils.h>

Glib::RefPtr<SessionManager> SessionManager::create()
{
	return Glib::RefPtr<SessionManager>(new SessionManager());
}

SessionManager::SessionManager() : RefCounter<SessionManager>::RefCounter(this)
{
  boost::filesystem::path::default_name_check(boost::filesystem::native);
  
  set_severity_level(alert::debug);
  
  Engine::instance()->get_settings_manager()->signal_update_settings().connect(sigc::mem_fun(this, &SessionManager::on_settings));
  
  on_settings(); //Apply settings on startup
}

SessionManager::~SessionManager()
{
  /* FIXME: Save Session settings and clean? */
}

sigc::signal<void> SessionManager::signal_update_queue()
{
  return m_signal_update_queue;
}

sigc::signal<void> SessionManager::signal_session_resumed()
{
  return m_signal_session_resumed;
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
    Glib::ustring hash_str = *iter;
    if (hash_str.find(".resume", hash_str.size()-7) == Glib::ustring::npos)
      continue;
      
    resume_torrent(hash_str.substr(0, hash_str.size()-7));
  }
  m_signal_session_resumed.emit();
}

void SessionManager::on_settings()
{
  Glib::RefPtr<SettingsManager> settings = Engine::instance()->get_settings_manager();

  Glib::ustring iface = settings->get_string("Network", "Interface");
  ip_address ip;
  get_ip(iface.c_str(), ip);
  
  if (iface.size())
    listen_on(std::make_pair(settings->get_int("Network", "MinPort"),
                             settings->get_int("Network", "MaxPort")), ip);
  else
    listen_on(std::make_pair(settings->get_int("Network", "MinPort"),
                             settings->get_int("Network", "MaxPort")));
    
  int up_rate = settings->get_int("Network", "MaxUpRate")*1024;
  if (up_rate < 1)
    up_rate = -1;
  set_upload_rate_limit(up_rate);
  
  int down_rate = settings->get_int("Network", "MaxDownRate")*1024;
  if (down_rate < 1)
    down_rate = -1;
  set_download_rate_limit(down_rate);
  
  int max_uploads = settings->get_int("Network", "MaxUploads");
  if (max_uploads == 0)
    max_uploads = -1;      
  set_max_uploads(max_uploads);
  
  int max_connections = settings->get_int("Network", "MaxConnections");
  if (max_connections == 0)
    max_connections = -1;
  set_max_connections(max_connections);
  
  session_settings sset;
  fingerprint id("LK", 0, 19, 0, 0);
  sset.user_agent = id.to_string();
  sset.tracker_completion_timeout = settings->get_int("Network", "TrackerTimeout");
  sset.tracker_receive_timeout = settings->get_int("Network", "TrackerTimeout");
  sset.stop_tracker_timeout = settings->get_int("Network", "TrackerTimeout");
  if (settings->get_bool("Network", "UseProxy"))
  {
    sset.proxy_port = settings->get_int("Network", "ProxyPort");
    sset.proxy_ip = settings->get_string("Network", "ProxyPort");
    sset.proxy_login = settings->get_string("Network", "ProxyLogin");
    sset.proxy_password = settings->get_string("Network", "ProxyPass");
  }
  set_settings(sset);
}

sha1_hash SessionManager::open_torrent(const Glib::ustring& file, 
                                       const Glib::ustring& save_path)
{
  std::ifstream in(file.c_str(), std::ios::binary|std::ios::ate);
  in.unsetf(std::ios_base::skipws);
  std::streampos sz = in.tellg();
  in.seekg(0, std::ios::beg);
  std::vector<char> buff(sz);

  in.read(&buff[0], sz);
  
  entry e;
  
  try
  {
    e = bdecode(buff.begin(), buff.end());
  }
  catch (std::exception& e)
  {
    m_signal_invalid_bencoding.emit("Invalid bencoding in " + file, file);
    return INVALID_HASH;
  }
  
  torrent_info info(e);
  sha1_hash hash = info.info_hash();
  
  if (!Engine::instance()->get_torrent_manager()->exists(hash))
  {
    torrent_handle handle = add_torrent(info, save_path.c_str(), entry(), 
      !Engine::instance()->get_settings_manager()->get_bool("Files", "Allocate"));
    entry::dictionary_type de;
    de["path"] = save_path;
    entry::list_type filter;
    filter.push_back(info.num_files());
    de["filter"] = filter;
    de["name"] = info.name();
    /* Must use std::string beacuse Glib::ustring is UTF-8 */
    de["info-hash"] = std::string((char*)info.info_hash().begin(), (char*)info.info_hash().end());;
    
    Engine::instance()->get_torrent_manager()->add_torrent(handle, de);
  }
  else
  {
    m_signal_duplicate_torrent.emit("Torrent already exists in session as " + 
      Engine::instance()->get_torrent_manager()->get_torrent(hash)->get_name(), hash);
    return INVALID_HASH;
  }
  
  //Save metadata to data_dir/hash
  Glib::ustring metafile = Glib::build_filename(get_data_dir(), str(hash));
  std::ofstream out;
  out.open(metafile.c_str(), std::ios_base::binary);
  out.write(&buff[0], buff.size());
  out.close();

  m_signal_update_queue.emit();
  
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
  
  try
  {
    in.open(file.c_str(), std::ios_base::binary);
    in.unsetf(std::ios_base::skipws);
    e = bdecode(std::istream_iterator<char>(in), std::istream_iterator<char>());
    in.close();
  }
  catch (std::exception& e)
  {
    m_signal_invalid_bencoding.emit("Invalid bencoding in " + file, file);
    return INVALID_HASH;
  }
  
  torrent_info info = torrent_info(e);
  /* Check if torrent is up an running, if so return */
  if (Engine::instance()->get_torrent_manager()->get_handle(info.info_hash()).is_valid())
    return INVALID_HASH;
    
  file = file + ".resume";
  try
  {
    in.open(file.c_str(), std::ios_base::binary);  
    er = bdecode(std::istream_iterator<char>(in), std::istream_iterator<char>());
    in.close();             
  }
  catch (std::exception& e)
  {
    /* If this happens we need to handle it better below */
    m_signal_invalid_bencoding.emit("Invalid bencoding in " + file, file);
  }
  
  bool allocate = Engine::instance()->get_settings_manager()->get_bool("Files", "Allocate");
  Glib::ustring save_path = er.dict()["path"].string();
  er.dict()["name"] = info.name();
  
  if (Engine::instance()->get_torrent_manager()->exists(hash_str)) //Torrent was resumed from stopped state
  {
    torrent_handle handle = add_torrent(info, save_path.c_str(), er, !allocate);
    Engine::instance()->get_torrent_manager()->add_torrent(handle, er.dict());
    m_signal_update_queue.emit();
  }
  else //Torrent was resumed from previous session
  {
    Engine::instance()->get_torrent_manager()->add_torrent(er.dict());
    if (er.dict()["running"].integer())
    {
      torrent_handle handle = add_torrent(info, save_path.c_str(), er, !allocate);
      Engine::instance()->get_torrent_manager()->add_torrent(handle, er.dict());
      m_signal_update_queue.emit();
    }
  }
  return info.info_hash();
}

void SessionManager::stop_torrent(const sha1_hash& hash)
{
  if (Engine::instance()->get_torrent_manager()->exists(hash))
  {
    Glib::ustring hash_str = str(hash);
    WeakPtr<Torrent> torrent = Engine::instance()->get_torrent_manager()->get_torrent(hash);
    
    if (!torrent->is_running())
      return;
         
    torrent->stop();
		entry e = torrent->get_resume_entry();
		
    Engine::instance()->get_torrent_manager()->save_fastresume(hash, e);
    
    remove_torrent(torrent->get_handle());
    
    m_signal_update_queue.emit();
  }
}

void SessionManager::erase_torrent(const sha1_hash& hash)
{
  torrent_handle handle = Engine::instance()->get_torrent_manager()->get_handle(hash);
  
  if (handle.is_valid())
    remove_torrent(handle);
  
  
  Engine::instance()->get_torrent_manager()->remove_torrent(hash);
  
  Glib::ustring file = Glib::build_filename(get_data_dir(), str(hash));
  g_remove(file.c_str());
  g_remove((file + ".resume").c_str());
}
