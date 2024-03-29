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

#include "linkage/Interface.hh"
#include "linkage/Engine.hh"
#include "linkage/SessionManager.hh"

using namespace Linkage;

void Interface::Open(const DBus::String& file)
{
	open(file);
}

void Interface::Add(const DBus::String& file, const DBus::String& path)
{
	Engine::get_session_manager()->open_torrent(file, path);
}

DBus::Bool Interface::GetVisible()
{
	return get_visible();
}

void Interface::SetVisible(const DBus::Bool& visible)
{
	set_visible(visible);
}

void Interface::Quit()
{
	quit();
}

