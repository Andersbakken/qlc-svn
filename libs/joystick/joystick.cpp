/*
  Q Light Controller
  joystick.cpp

  Copyright (C) 2000, 2001, 2002 Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <linux/joystick.h>
#include <qstring.h>
#include <qfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/errno.h>

#include "joystick.h"
#include "joysticksettings.h"
#include "../common/plugin.h"

#define JS_NAME_LENGTH    0x80

//
// Joystick poll thread
//
void* joystickThread(void* param);

//
// Constructor
//
Joystick::Joystick()
{
  m_valid = false;
  m_fd = -1;
  m_fdName = QString::null;
  m_name = QString::null;
  m_thread = 0;
}

void Joystick::create(const char* deviceFileName)
{
  int axes = 0;
  int buttons = 0;
  int version = 0x000800;
  char deviceName[JS_NAME_LENGTH + 1];

  /* Init the poll thread to NULL */
  m_thread = 0;

  m_fd = ::open(deviceFileName, O_RDONLY);
  if (m_fd != -1)
    {
      /* Copy the device file name to struct */
      m_fdName = QString(deviceFileName);

      ::usleep(2000);

      ioctl(m_fd, JSIOCGVERSION, &version);
      ioctl(m_fd, JSIOCGAXES, &axes);
      fillAxisNames(axes);

      ioctl(m_fd, JSIOCGBUTTONS, &buttons);
      fillButtonNames(buttons);

      ioctl(m_fd, JSIOCGNAME(JS_NAME_LENGTH), &deviceName);
      m_name = QString(deviceName);

      printf("Device: %s\nDriver version: %d.%d.%d\nName: %s\nAxes: %d\nButtons: %d\n",
	     (const char*) m_fdName,
	     (version >> 16), (version >> 8) & 0xFF, version & 0xFF,
	     (const char*) m_name, axes, buttons);
      
      /* Close this joystick until we really want to use it */
      close();
      
      m_valid = true;
    }
  else
    {
      m_valid = false;
    }
}

Joystick::~Joystick()
{
  stop();
  close();
}

void Joystick::fillButtonNames(int buttons)
{
  int i = 0;

  for (i = 0; i < buttons; i++)
    {
      Control* c = new Control();
      c->name.sprintf("Button %d", i);
      m_buttonsList.append(c);
    }
}

void Joystick::fillAxisNames(int axes)
{
  int i = 0;
  double axisnum = 1.0;
  char axis = 'X';

  for (i = 0; i < axes;)
    {
      Control* c = new Control();
      c->name.sprintf("%c Axis %d", axis, (int) axisnum);
      axisnum += 0.5;
      m_axesList.append(c);
      i++;
      axis = ('X') + i%2;
    }
}

//
// Close the joystick device
//
int Joystick::close()
{
  if (m_fd != -1)
    {
      if (::close(m_fd) == -1)
	{
	  perror("close");
	  return EIO;
	}
      
      m_fd = -1;
      return 0;
    }
  else
    {
      return EBADF;
    }
}


//
// Open the joystick device
//
int Joystick::open()
{
  if (m_fd == -1)
    {
      m_fd = ::open(m_fdName, O_RDONLY);

      if (m_fd == -1)
	{
	  perror("open");
	  return EBADF;
	}
      else
	{
	  return 0;
	}
    }
  else
    {
      return EMFILE;
    }
}

void Joystick::saveToFile(QFile &file)
{
  QString s;
  QString t;

  // Comment line
  s = QString("# Input device entry\n");
  file.writeBlock((const char*) s, s.length());

  // Entry type
  s = QString("Entry = Joystick") + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // File descriptor
  s = QString("FDName = ") + fdName() + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Name
  s = QString("Name = ") + name() + QString("\n");
  file.writeBlock((const char*) s, s.length());

  // Axis invert status
  for (unsigned int i = 0; i < m_axesList.count(); i++)
    {
      if (m_axesList.at(i)->invert == true)
	t = QString("true");
      else
	t = QString("false");
  
      s = QString("AxisInvert = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());
    }
}

void Joystick::createContents(QList <QString> &list)
{
  QString t;
  int axis = 0;

  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("AxisInvert"))
	{
	  t = *(list.next());
	  Control* c = m_axesList.at(axis);
	  if (c != NULL)
	    {
	      c->invert = (t == QString("true")) ? true : false;
	    }
	  else
	    {
	      qDebug("Too many axes defined for joystick \"" + name() + "\" in workspace file!");
	    }

	  axis++;
	}
      else
	{
	  // Unknown keyword, skip
	  list.next();
	}
    }
}


//
// Start poll thread
//
int Joystick::start()
{
  if (m_thread == 0)
    {
      if (pthread_create(&m_thread, 0, joystickThread, (void*) this) != 0)
	{
	  perror("pthread_create");
	  return -1;
	}
      else
	{
	  fprintf(stderr, "%s <%s> poll thread started\n", 
		  (const char*) m_name, (const char*) m_fdName);
	  return 0;
	}
    }
  else
    {
      return -1;
    }
}

//
// Stop poll thread
//
int Joystick::stop()
{
  /* Kill poll thread */
  if (m_thread != 0)
    {
      if (pthread_cancel(m_thread) != 0)
	{
	  perror("pthread_cancel");
	  return -1;
	}
      else
	{
	  fprintf(stderr, "%s <%s> poll thread killed\n", 
		  (const char*) m_name, (const char*) m_fdName);
	  m_thread = 0;
	  return 0;
	}
    }
  else
    {
      return 1;
    }
}

void Joystick::emitAxisEvent(int axis, int value)
{
  emit axisEvent(axis, value);
}

void Joystick::emitButtonEvent(int button, int value)
{
  emit buttonEvent(button, value);
}

void* joystickThread(void* param)
{
  Joystick* j = (Joystick*) param;

  struct js_event js;

  while (1)
    {
      if (read(j->fd(), &js, sizeof(struct js_event)) != sizeof(js_event))
	{
	  perror("Error reading joystick status");
	  j->stop();
	  break;
	}

      switch (js.type & ~JS_EVENT_INIT)
	{
	case JS_EVENT_BUTTON:
	  {
	    j->buttonsList().at(js.number)->value = js.value;
	    j->emitButtonEvent(js.number, js.value);
	  }
	  break;

	case JS_EVENT_AXIS:
	  {
	    j->axesList().at(js.number)->value = js.value;
	    j->emitAxisEvent(js.number, js.value);
	  }
	  break;
	}
    }

  return NULL;
}

void Joystick::settings(QWidget* parent, QApplication* qapp)
{
  JoystickSettings* js = new JoystickSettings(parent, qapp, this);
  js->show();
}
