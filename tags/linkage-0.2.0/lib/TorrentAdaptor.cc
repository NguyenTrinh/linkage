/*
Copyright (C) 2007	Christian Lundgren

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
#include "linkage/Engine.hh"
#include "linkage/SessionManager.hh"

using namespace Linkage;

DBus::String Torrent::GetName()
{
	return get_name();
}

DBus::String Torrent::GetState()
{
	return state_string(get_state());
}

DBus::Struct<DBus::UInt32, DBus::UInt32> Torrent::GetRates()
{
	DBus::Struct<DBus::UInt32, DBus::UInt32> rates;
	libtorrent::torrent_status status = get_status();
	// FIXME: return floats
	rates._1 = (int)status.download_payload_rate;
	rates._2 = (int)status.upload_payload_rate;
	return rates;
}

DBus::Struct<DBus::UInt64, DBus::UInt64> Torrent::GetTransfered()
{
	DBus::Struct<DBus::UInt64, DBus::UInt64> bytes;
	libtorrent::torrent_status status = get_status();
	libtorrent::size_type down = status.total_payload_download + m_downloaded;
	libtorrent::size_type up = status.total_payload_upload + m_uploaded;
	bytes._1 = down;
	bytes._2 = up;
	return bytes;
}

DBus::Double Torrent::GetProgress()
{
	return get_status().progress;
}

DBus::UInt32 Torrent::GetPosition()
{
	return get_position();
}

DBus::String Torrent::GetPath()
{
	return get_path();
}

void Torrent::Start()
{
	Engine::get_session_manager()->resume_torrent(TorrentPtr(this));
}

void Torrent::Stop()
{
	Engine::get_session_manager()->stop_torrent(TorrentPtr(this));
}

void Torrent::Remove(const DBus::Bool& erase_content)
{
	Engine::get_session_manager()->erase_torrent(TorrentPtr(this), erase_content);
}

