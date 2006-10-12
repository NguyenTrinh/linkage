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
#include "linkage/TorrentManager.hh"
#include "linkage/Utils.hh"

#include <glib/gstdio.h>

SessionManager* SessionManager::smInstance = NULL;

SessionManager* SessionManager::instance()
{
  static bool running = false;
  if (smInstance == NULL && running == false)
  {
    running = true;
    smInstance = new SessionManager();
    running = false;
  }
  return smInstance;
}

void SessionManager::goodnight()
{
  if (smInstance != NULL)
  {
    delete smInstance;
  }
}

SessionManager::SessionManager()
{
  boost::filesystem::path::default_name_check(boost::filesystem::native);
  
  set_severity_level(alert::debug);
  
  SettingsManager::instance()->signal_update_settings().connect(sigc::mem_fun(this, &SessionManager::on_settings));
  
  on_settings(); //Apply settings on startup
}

SessionManager::~SessionManager()
{
  /* FIXME: Save Session settings and clean? */
}

sigc::signal<void> SessionManager::signal_update_queue()
{
  return signal_update_queue_;
}

sigc::signal<void> SessionManager::signal_session_resumed()
{
  return signal_session_resumed_;
}

sigc::signal<void, const Glib::ustring&, const Glib::ustring&> 
SessionManager::signal_invalid_bencoding()
{
  return signal_invalid_bencoding_;
}

sigc::signal<void, const Glib::ustring&, const Glib::ustring&> 
SessionManager::signal_missing_file()
{
  return signal_missing_file_;
}

sigc::signal<void, const Glib::ustring&, const sha1_hash&> 
SessionManager::signal_duplicate_torrent()
{
  return signal_duplicate_torrent_;
}
  
void SessionManager::resume_session()
{
  std::vector<Glib::ustring> hash_strs = SettingsManager::instance()->get_groups();
  for (int i = NUM_DEFAULT_GROUPS; i < hash_strs.size(); i++)
  {
    sha1_hash hash = resume_torrent(hash_strs[i]);
  }
  signal_session_resumed_.emit();
}

void SessionManager::on_settings()
{
  SettingsManager* settings = SettingsManager::instance();
  
  Glib::ustring iface = settings->get<Glib::ustring>("Network", "Interface");
  ip_address ip;
  get_ip(iface.c_str(), ip);
  
  if (iface.size())
    listen_on(std::make_pair(settings->get<int>("Network", "MinPort"),
                             settings->get<int>("Network", "MaxPort")), ip);
  else
    listen_on(std::make_pair(settings->get<int>("Network", "MinPort"),
                             settings->get<int>("Network", "MaxPort")));
    
  int up_rate = settings->get<int>("Network", "MaxUpRate")*1024;
  if (up_rate < 1)
    up_rate = -1;
  set_upload_rate_limit(up_rate);
  
  int down_rate = settings->get<int>("Network", "MaxDownRate")*1024;
  if (down_rate < 1)
    down_rate = -1;
  set_download_rate_limit(down_rate);
  
  int max_uploads = settings->get<int>("Network", "MaxUploads");
  if (max_uploads == 0)
    max_uploads = -1;      
  set_max_uploads(max_uploads);
  
  int max_connections = settings->get<int>("Network", "MaxConnections");
  if (max_connections == 0)
    max_connections = -1;
  set_max_connections(max_connections);
  
  session_settings sset;
  fingerprint id("LK", 0, 19, 0, 0);
  sset.user_agent = id.to_string();
  sset.tracker_completion_timeout = settings->get<int>("Network", "TrackerTimeout");
  sset.tracker_receive_timeout = settings->get<int>("Network", "TrackerTimeout");
  sset.stop_tracker_timeout = settings->get<int>("Network", "TrackerTimeout");
  if (settings->get<bool>("Network", "UseProxy"))
  {
    sset.proxy_port = settings->get<int>("Network", "ProxyPort");
    sset.proxy_ip = settings->get<Glib::ustring>("Network", "ProxyPort");
    sset.proxy_login = settings->get<Glib::ustring>("Network", "ProxyLogin");
    sset.proxy_password = settings->get<Glib::ustring>("Network", "ProxyPass");
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
    signal_invalid_bencoding_.emit("Invalid bencoding in " + file, file);
    return INVALID_HASH;
  }
  
  torrent_info info(e);
  sha1_hash hash = info.info_hash();
  
  if (!TorrentManager::instance()->exists(hash))
  {
    torrent_handle handle = add_torrent(info, save_path.c_str(), entry(), 
                              !SettingsManager::instance()->get<bool>("Files", "Allocate"));
    TorrentManager::instance()->add_torrent(handle);
    SettingsManager::instance()->set(str(hash), "Path", save_path);
  }
  else
  {
    signal_duplicate_torrent_.emit("Torrent already exists in session as " + 
      TorrentManager::instance()->get_torrent(hash)->get_name(), hash);
    return INVALID_HASH;
  }
  
  //FIXME: clean this up...
  Glib::ustring hash_str = str(info.info_hash());
  SettingsManager* sm = SettingsManager::instance();
  sm->set(hash_str, "Stopped", false);
  sm->set(hash_str, "Path", save_path);
  sm->set(hash_str, "Down", 0);
  sm->set(hash_str, "Up", 0);
  sm->set(hash_str, "Time", 0);
  sm->set(hash_str, "DownLimit", 0);
  sm->set(hash_str, "UpLimit", 0);
  std::list<int> filter;
  filter.push_back(info.num_files());
  sm->set(hash_str, "Filter", IntArray(filter));
  sm->set(hash_str, "Group", sm->get<Glib::ustring>("Files", "DefGroup"));
  
  //Save metadata to data_dir/hash
  Glib::ustring metafile = Glib::build_filename(get_data_dir(), str(hash));
  std::ofstream out;
  out.open(metafile.c_str(), std::ios_base::binary);
  out.write(&buff[0], buff.size());
  out.close();

  signal_update_queue_.emit();
  
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
    signal_invalid_bencoding_.emit("Invalid bencoding in " + file, file);
    return INVALID_HASH;
  }
  
  torrent_info info = torrent_info(e);
  /* Check if torrent is up an running, if so return */
  if (TorrentManager::instance()->get_handle(info.info_hash()).is_valid())
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
    signal_invalid_bencoding_.emit("Invalid bencoding in " + file, file);
  }
  
  bool allocate = SettingsManager::instance()->get<bool>("Files", "Allocate");
  Glib::ustring save_path = SettingsManager::instance()->get<Glib::ustring>(hash_str, "Path");
  
  if (TorrentManager::instance()->exists(hash_str)) //Torrent was resumed from stopped state
  {
    /*//Only look for missing files if we're on full allocation. Doesn't fastresume cover this?
    if (allocate)
    {
      Glib::ustring file;
      if (!files_available(save_path, file, info.begin_files(), info.end_files()))
        signal_missing_file_.emit("File not found " + file, file);
    }*/
    SettingsManager::instance()->set(hash_str, "Stopped", false);
    torrent_handle handle = add_torrent(info, save_path.c_str(), er, !allocate);
    TorrentManager::instance()->add_torrent(handle);
    signal_update_queue_.emit();
  }
  else //Torrent was resumed from previous session
  {
    TorrentManager::instance()->add_torrent(info);
    if (!SettingsManager::instance()->get<bool>(str(info.info_hash()), "Stopped"))
    {
      /*if (allocate)
      {
        Glib::ustring file;
        if (!files_available(save_path, file, info.begin_files(), info.end_files()))
          signal_missing_file_.emit("File not found " + file, file);
      }*/
      torrent_handle handle = add_torrent(info, save_path.c_str(), er, !allocate);
      TorrentManager::instance()->add_torrent(handle);
      signal_update_queue_.emit();
    }
  }
  return info.info_hash();
}

void SessionManager::stop_torrent(const sha1_hash& hash)
{
  if (TorrentManager::instance()->exists(hash))
  {
    SettingsManager* sm = SettingsManager::instance();
    
    Glib::ustring hash_str = str(hash);
    Torrent* torrent = TorrentManager::instance()->get_torrent(hash);
    
    if (!torrent->get_handle().is_valid())
      return;
      
    int down = sm->get<int>(hash_str, "Down");
    int up = sm->get<int>(hash_str, "Up");
    
    torrent->get_handle().pause();
    torrent->set_stop_time();
    torrent_status status = torrent->get_handle().status();
    
    std::list<int> filtered_files;
    std::vector<bool> filter = torrent->get_filter();
    filtered_files.push_back(filter.size());
    for (int i = 0; i < filter.size(); i++)
      if (filter[i])
        filtered_files.push_back(i);
    
    sm->set(hash_str, "Stopped", true);
    sm->set(hash_str, "Down", int(down + status.total_download)); /* Hmm do we loose bytes here if files are big? */
    sm->set(hash_str, "Up", int(up + status.total_upload));
    sm->set(hash_str, "Time", torrent->get_time_active());
    sm->set(hash_str, "DownLimit", torrent->get_down_limit());
    sm->set(hash_str, "UpLimit", torrent->get_up_limit());
    sm->set(hash_str, "Filter", IntArray(filtered_files));
    sm->set(hash_str, "Group", torrent->get_group());

    save_fastresume(hash);
    remove_torrent(torrent->get_handle());
    
    signal_update_queue_.emit();
  }
}

void SessionManager::erase_torrent(const sha1_hash& hash)
{
  torrent_handle handle = TorrentManager::instance()->get_handle(hash);
  
  if (handle.is_valid())
    remove_torrent(handle);
  
  Glib::ustring hash_str = str(hash);
  TorrentManager::instance()->remove_torrent(hash);
  SettingsManager::instance()->remove_group(hash_str);
  
  g_remove(Glib::build_filename(get_data_dir(), hash_str).c_str());
}

Glib::ustring SessionManager::get_data_dir()
{
  return Glib::build_filename(get_config_dir(), "data");
}

void SessionManager::save_fastresume(const sha1_hash& hash)
{
  entry er = TorrentManager::instance()->get_handle(hash).write_resume_data();
  Glib::ustring file = Glib::build_filename(get_data_dir(), str(hash) + ".resume");
  
  std::ofstream out(file.c_str(), std::ios_base::binary);
  out.unsetf(std::ios_base::skipws);
  bencode(std::ostream_iterator<char>(out), er);
  out.close();
}


