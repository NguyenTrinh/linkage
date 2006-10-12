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

#include <vector>

#include "linkage/TorrentManager.hh"
#include "linkage/SettingsManager.hh"
#include "linkage/SessionManager.hh"

TorrentManager* TorrentManager::smInstance = NULL;

TorrentManager* TorrentManager::instance()
{
  static bool running = false;
  if (smInstance == NULL && running == false)
  {
    running = true;
    smInstance = new TorrentManager();
    running = false;
  }
  return smInstance;
}
                                                 
void TorrentManager::goodnight()
{
  if (smInstance != NULL)
  {
    delete smInstance;
  }
}

TorrentManager::TorrentManager()
{
  SessionManager::instance()->signal_update_queue().connect(sigc::mem_fun(*this, &TorrentManager::check_queue));
  //UI::signal_update_queue().connect(sigc::mem_fun(*this, &TorrentManager::check_queue));
}

TorrentManager::~TorrentManager()
{
  SettingsManager* sm = SettingsManager::instance();
  
  for (TorrentIter iter = torrents.begin(); iter != torrents.end(); ++iter)
  {
    sha1_hash hash = iter->first;
    Torrent* torrent = iter->second;
    
    Glib::ustring hash_str = str(hash);
    
    if (torrent->get_handle().is_valid())
    {
      torrent->get_handle().pause();
      torrent_status status = torrent->get_handle().status();
      
      int down = sm->get<int>(hash_str, "Down");
      int up = sm->get<int>(hash_str, "Up");

      sm->set(hash_str, "Down", int(down + status.total_download)); /* FIXME: Possible data loss/wrap on large ammounts */
      sm->set(hash_str, "Up", int(up + status.total_upload));
      sm->set(hash_str, "Time", torrent->get_time_active());

      SessionManager::instance()->save_fastresume(hash);
      SessionManager::instance()->remove_torrent(torrent->get_handle());
    }
    
    sm->set(hash_str, "DownLimit", torrent->get_down_limit());
    sm->set(hash_str, "UpLimit", torrent->get_up_limit());
    std::list<int> filtered_files;
    std::vector<bool> filter = torrent->get_filter();
    filtered_files.push_back(filter.size());
    for (int i = 0; i < filter.size(); i++)
      if (filter[i])
        filtered_files.push_back(i);
    sm->set(hash_str, "Filter", IntArray(filtered_files));
    sm->set(hash_str, "Group", torrent->get_group());
    
    delete torrent;
  }
}

sigc::signal<void, const sha1_hash&, int> TorrentManager::signal_position_changed()
{
  return signal_position_changed_;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&> TorrentManager::signal_group_changed()
{
  return signal_group_changed_;
}

sigc::signal<void, const sha1_hash&, const Glib::ustring&> TorrentManager::signal_response_changed()
{
  return signal_response_changed_;
}


sigc::signal<void, const sha1_hash&, const Glib::ustring&, const Glib::ustring&, int> TorrentManager::signal_added()
{
  return signal_added_;
}

sigc::signal<void, const sha1_hash&> TorrentManager::signal_removed()
{
  return signal_removed_;
}

void TorrentManager::on_torrent_position_changed(const sha1_hash& hash, Torrent::Direction direction)
{
  for (TorrentIter iter = torrents.begin(); iter != torrents.end(); ++iter)
  {
    int position = iter->second->get_position();
    if (position == torrents[hash]->get_position() && iter->first != hash)
    {
      if (direction == Torrent::DIR_UP)
        iter->second->set_position(position + 1);
      else if (direction == Torrent::DIR_DOWN)
        iter->second->set_position(position - 1);
    }  
  }
  signal_position_changed_.emit(hash, torrents[hash]->get_position());
}

void TorrentManager::on_torrent_group_changed(const sha1_hash& hash, const Glib::ustring& group)
{
  signal_group_changed_.emit(hash, group);
}

void TorrentManager::on_torrent_response_changed(const sha1_hash& hash, const Glib::ustring& response)
{
  signal_response_changed_.emit(hash, response);
}

bool TorrentManager::exists(const sha1_hash& hash)
{
  return (torrents.find(hash) != torrents.end());
}

bool TorrentManager::exists(const Glib::ustring& hash_str)
{
  for (TorrentIter iter = torrents.begin(); iter != torrents.end(); ++iter)
  {
    if (hash_str == str(iter->first))
      return true;
  }
  
  return false;
}

void TorrentManager::add_torrent(torrent_handle handle)
{
  if (torrents.find(handle.info_hash()) == torrents.end())
    add_torrent(handle.get_torrent_info());
  torrents[handle.info_hash()]->set_handle(handle);
  torrents[handle.info_hash()]->set_start_time(); //FIXME: This might aswell be private in Torrent class
}

void TorrentManager::add_torrent(torrent_info info)
{
  int position = torrents.size() + 1;
  int time = 0;
  std::vector<int> indices(1, info.num_files());
  
  if (SettingsManager::instance()->has_group(str(info.info_hash())))
    time = SettingsManager::instance()->get<int>(str(info.info_hash()), "Time");
  
  Torrent* torrent = new Torrent(position, info.info_hash(), SettingsManager::instance()->get<Glib::ustring>("Files", "DefGroup"), time);
  torrent->set_name(info.name());
  torrents[info.info_hash()] = torrent;
  
  if (SettingsManager::instance()->has_group(str(info.info_hash())))
    indices = SettingsManager::instance()->get<IntArray>(str(info.info_hash()), "Filter");
    
  std::vector<bool> filter(indices[0], false);
  for (int i = 1; i < indices.size(); i++)
    filter[indices[i]] = true;
    
  torrent->set_filter(filter);
  
  torrent->signal_position_changed().connect(sigc::mem_fun(*this, &TorrentManager::on_torrent_position_changed));
  torrent->signal_group_changed().connect(sigc::mem_fun(*this, &TorrentManager::on_torrent_group_changed));
  torrent->signal_response_changed().connect(sigc::mem_fun(*this, &TorrentManager::on_torrent_response_changed));

  signal_added_.emit(info.info_hash(), torrent->get_name(), torrent->get_group(), position);
}

void TorrentManager::remove_torrent(const sha1_hash& hash)
{
  delete torrents[hash];
  torrents.erase(hash);
  signal_removed_.emit(hash);
}

torrent_handle TorrentManager::get_handle(const sha1_hash& hash)
{
  torrent_handle handle;
  for (TorrentIter iter = torrents.begin(); iter != torrents.end(); ++iter)
  {
    if (iter->first == hash)
      handle = iter->second->get_handle();
  }
  return handle;
}

Torrent* TorrentManager::get_torrent(const sha1_hash& hash)
{
  for (TorrentIter iter = torrents.begin(); iter != torrents.end(); ++iter)
  {
    if (iter->first == hash)
      return iter->second;
  }
  return 0;
}

Torrent* TorrentManager::get_torrent(int position)
{
  for (TorrentIter iter = torrents.begin(); iter != torrents.end(); ++iter)
  {
    if (iter->second->get_position() == position)
      return iter->second;
  }
  return 0;
}

int TorrentManager::get_torrents_count()
{
  return torrents.size();
}

void TorrentManager::check_queue()
{
  std::vector<sha1_hash> order;
  std::vector<sha1_hash>::iterator order_iter;
  int num_active = 0;
  int max_active = SettingsManager::instance()->get<int>("Network", "MaxActive");
  //Sort torrents by position
  for (TorrentIter iter = torrents.begin(); iter != torrents.end(); ++iter)
  {
    if (iter->second->get_handle().is_valid())
    {
      if (!iter->second->get_handle().is_paused())
        num_active++;
    }
    else
      continue;
      
    if (order.empty())
      order.push_back(iter->second->get_handle().info_hash());
    else
    {
      int pos = 0;
      while (pos < order.size() && torrents[order[pos]]->get_position() < iter->second->get_position())
        pos++;
      order_iter = order.begin()+pos;
      order.insert(order_iter, iter->second->get_handle().info_hash());
    }
  }
  
  if (num_active == max_active) //Make sure queue is correct
  {
    for (int i = order.size()-1; i > 0; i--)
    {
      if (!torrents[order[i]]->get_handle().is_valid())
        continue;
      else if (torrents[order[i]]->get_handle().is_seed())
        continue;
      if (!torrents[order[i]]->get_handle().is_paused())
      {
        //Make sure everything above is also active
        bool should_queue = false;
        for (int j = i; j > 0; j--)
        {
          if (!torrents[order[i]]->get_handle().is_valid())
            continue;
          if (torrents[order[j]]->get_handle().is_paused())
          {  
            should_queue = true;
            torrents[order[j]]->get_handle().resume();
            num_active++;
          }
        }
        if (should_queue)
        {
          torrents[order[i]]->get_handle().pause();
          num_active--;
        }
      }
    }
  }
  if (num_active > max_active) //Stop everything from bottom until queue is ok
  {
    int i = order.size() - 1;
    while (num_active != max_active)
    {
      if (i < 0)
        break;
      if (torrents[order[i]]->get_handle().is_seed())
        continue;
      if (torrents[order[i]]->get_handle().is_valid())
        if (!torrents[order[i]]->get_handle().is_paused())
        {  
          torrents[order[i]]->get_handle().pause();
          num_active--;
        }
        i--;
    }
  }
  if (num_active < max_active) //Start everything paused until queue is maxed
  {
    int i = 0;
    while (num_active != max_active)
    {
      if (i >= order.size())
        break;
      if (torrents[order[i]]->get_handle().is_valid())
        if (torrents[order[i]]->get_handle().is_paused())
        {  
          torrents[order[i]]->get_handle().resume();
          num_active++;
        }
        i++;
    }
  }
}
