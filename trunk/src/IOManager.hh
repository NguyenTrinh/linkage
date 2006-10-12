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

#ifndef IOMANAGER_HH
#define IOMANAGER_HH

#include <glibmm/iochannel.h>

/* A mini class handling the fifo */

class IOManager
{
  int fd;
  Glib::RefPtr<Glib::IOChannel> ioc;
  
  int create_fifo();

  bool on_io_input(Glib::IOCondition condition);
  
  void clean();
  
public:
  IOManager(int argc, char *argv[]);
  ~IOManager();
};

#endif  /*  IOMANAGER_HH  */
