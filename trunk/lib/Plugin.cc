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

#include "linkage/Plugin.hh"

Plugin::Plugin()
{
}

Plugin::~Plugin()
{
	g_debug("destructor plugin");
}

Plugin::Info Plugin::get_info()
{
	return Info("Generic plugin",
		"Generic plugin description",
		"0.0",
		"Linkage",
		"http://code.google.com/p/linkage",
		false,
		PARENT_NONE);
}

