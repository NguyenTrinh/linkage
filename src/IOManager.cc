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

#include "IOManager.hh"
#include "UI.hh"
#include "linkage/Utils.hh"

#include <glibmm/miscutils.h>
#include <glibmm/fileutils.h>
#include <gtkmm/messagedialog.h>

#include <glib/gstdio.h>
#include <fcntl.h>

IOManager::IOManager(int argc, char *argv[])
{
  int fd = create_fifo();
  
  if (fd != -1)
  {
    ioc = Glib::IOChannel::create_from_fd(fd);
    
    if ((ioc->get_flags() & Glib::IO_FLAG_IS_WRITEABLE) == 0)
    {
      /* IOChannel is read-only, which means we fire up a new instance */
      Glib::signal_io().connect(sigc::mem_fun(this, &IOManager::on_io_input), ioc, Glib::IO_IN);
      
      for (int i = 1; i < argc; i++)
      {
        Glib::ustring file = argv[i];
        if (file.substr(0, 1) == "-" || file.substr(0, 2) == "--")
        {
          throw Glib::OptionError(Glib::OptionError::UNKNOWN_OPTION, 
                                  "Unknown flag: \n"
                                  "Usage:\n  linkage [TORRENTS...]");
        }
        /* Check for relative paths */
        if (file.substr(0, 1) != "/")
          file.insert(0,g_getenv("PWD") +  Glib::ustring("/"));

        /* Pass file(s) to running instance */
        UI::instance()->add_torrent(file);
      }
      
    }
    else
    {
        
      if (argc > 1)
      {
        for (int i = 1; i < argc; i++)
        {
          Glib::ustring file = argv[i];
          if (file.substr(0, 1) == "-" || file.substr(0, 2) == "--")
          {
            throw Glib::OptionError(Glib::OptionError::UNKNOWN_OPTION, 
                                    "Unknown flag: \n"
                                    "Usage:\n  linkage [TORRENTS...]");
          }
          /* Check for relative paths */
          if (file.substr(0, 1) != "/")
            file.insert(0,g_getenv("PWD") +  Glib::ustring("/"));

          /* Pass file(s) to running instance */
          if (ioc->write(file + "\n") != Glib::IO_STATUS_NORMAL) 
          {
            throw Glib::FileError(Glib::FileError::FAILED, "error writing fifo");	
          }
        }
        throw Glib::IOChannelError(Glib::IOChannelError::FAILED, "file(s) passed to running instance");
      }
      else  /* Assume that previous session died */
      {
        Gtk::MessageDialog dialog("Instance error", false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO);
        dialog.set_secondary_text("It seems a previous instance crashed or is unresponsive, do want to start a new instance?");
        
        if (dialog.run() == Gtk::RESPONSE_NO)
        {
          throw Glib::IOChannelError(Glib::IOChannelError::FAILED, "quitting");
        }
        else
        {
          ioc->close();
          Glib::ustring fifo = Glib::build_filename(get_config_dir(), "fifo");
          if (g_unlink(fifo.c_str()) != 0)
            throw Glib::FileError(Glib::FileError::FAILED, "error removing fifo");
          fd = create_fifo();
          if (fd != -1)
          {
            ioc = Glib::IOChannel::create_from_fd(fd);
            Glib::signal_io().connect(sigc::mem_fun(this, &IOManager::on_io_input), ioc, Glib::IO_IN);
          }
          else
            throw Glib::IOChannelError(Glib::IOChannelError::IO_ERROR, "invalid file descriptor");
        }
      }
    }
  }
  else  /* fd is invalid so we bail */
  {
    throw Glib::IOChannelError(Glib::IOChannelError::IO_ERROR, "invalid file descriptor");
  }
}

IOManager::~IOManager()
{
  clean();
}

void IOManager::clean()
{
  Glib::ustring fifo = Glib::build_filename(get_config_dir(), "fifo");
  
  /* Only remove a read-only fifo so we don't remove a running session's fifo */
  if ((ioc->get_flags() & Glib::IO_FLAG_IS_WRITEABLE) == 0)
    if (g_unlink(fifo.c_str()) != 0)
      throw Glib::FileError(Glib::FileError::FAILED, "error removing fifo");
  
  ioc->close();
}

int IOManager::create_fifo()
{
  /* FIXME: Use socket instead of fifo? */
  fd = -1;
  Glib::ustring fifo = Glib::build_filename(get_config_dir(), "fifo");

  if (!Glib::file_test(fifo, Glib::FILE_TEST_EXISTS)) 
  {
    if (mkfifo(fifo.c_str(), 0644) != 0) 
      throw Glib::FileError(Glib::FileError::FAILED, "error creating fifo");

    fd = g_open(fifo.c_str(), O_NONBLOCK|O_RDONLY);
    if (fd == -1)
      throw Glib::FileError(Glib::FileError::FAILED, "error opening fifo");
  }
  else /* fifo exists, another instance is running */
  {
    fd = g_open(fifo.c_str(), O_RDWR);
    if (fd == -1)
      throw Glib::FileError(Glib::FileError::FAILED, "error opening fifo");
  }
    
  return fd;
}

bool IOManager::on_io_input(Glib::IOCondition condition)
{
  if ((condition & Glib::IO_IN) != 0)
  {
    Glib::ustring file;

    ioc->read_line(file);

    /* Get rid of trailing '\n' */
    file.erase(file.size()-1, 1);

    /* Pass to UI instead of SessionManager because we might need to ask for a save path */
    UI::instance()->add_torrent(file);
  }
  
  return true;
}
