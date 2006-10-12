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

#include "linkage/Alert.hh"

Alert::Alert()
{
  msg_ = "";
  active = false;
  code_ = ALERT_NONE;
  hash_ = 0;
}

Alert::Alert(const Glib::ustring& msg, AlertType code)
{
  msg_ = msg;
  active = true; 
  code_ = code;
  hash_ = 0;
}

Alert::Alert(const Glib::ustring& msg, AlertType code, const sha1_hash& hash)
{
  msg_ = msg;
  active = true;
  code_ = code;
  hash_ = hash;
}

Alert::~Alert()
{
}

bool Alert::get()
{
  return active;
}

const Glib::ustring& Alert::what()
{
  return msg_;
}

Alert::AlertType Alert::code()
{
  return code_;
}

const sha1_hash& Alert::hash()
{
  return hash_;
}
