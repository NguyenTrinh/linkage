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
	rates._1 = status.download_payload_rate;
	rates._2 = status.upload_payload_rate;
	return rates;
}

DBus::Struct<DBus::UInt64, DBus::UInt64> Torrent::GetTransfered()
{
	DBus::Struct<DBus::UInt64, DBus::UInt64> bytes;
	bytes._1 = get_total_downloaded();
	bytes._2 = get_total_uploaded();
	return bytes;
}

DBus::Double Torrent::GetProgress()
{
	return get_progress();
}

DBus::UInt32 Torrent::GetPosition()
{
	return get_position();
}

void Torrent::Start()
{
	//Glib::RefPtr<Torrent> ref(this);
	//Engine::get_session_manager()->resume_torrent(ref);
}

void Torrent::Stop()
{
	//Glib::RefPtr<Torrent> ref(this);
	//Engine::get_session_manager()->stop_torrent(ref);
}

void Torrent::Remove(const DBus::Bool& erase_content)
{
	//Glib::RefPtr<Torrent> ref(this);
	//Engine::get_session_manager()->erase_torrent(ref, erase_content);
}

