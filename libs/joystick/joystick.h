/*
  Q Light Controller
  joystick.h

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

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <pthread.h>
#include <qobject.h>
#include <qevent.h>
#include <qlist.h>
#include <qapplication.h>

#define CONTROLLER_BUTTON 0x01
#define CONTROLLER_AXIS   0x02

#define MAX_AXES 16

struct js_info
{
  int buttons;
  int axis[MAX_AXES];
};

class QString;
class QFile;
class JoystickEvent;

class Control
{
 public:
  Control() { invert = false; value = 0; }
  ~Control() { };

 public:
  QString name;
  bool invert;
  int value;
};

class Joystick : public QObject
{
  Q_OBJECT

  friend void *joystickThread (void *);

 public:
  Joystick();
  virtual ~Joystick();

  unsigned long id() { return m_id; }

  virtual int open();
  virtual int close();

  virtual void saveToFile(QFile &file);
  virtual void createContents(QList <QString> &list);
  virtual void create(const char* deviceFileName);

  virtual void settings(QWidget*, QApplication*);

  virtual bool valid() { return m_valid; }

  virtual int start();
  virtual int stop();

  QList <Control> axesList() { return m_axesList; }
  QList <Control> buttonsList() { return m_buttonsList; }

  QString name() { return m_name; }
  QString fdName() { return m_fdName; }

 signals:
  void axisEvent(int, int);
  void buttonEvent(int, int);

 private:
  unsigned long m_id;
  bool m_valid;

  QString m_name;

  QList <Control> m_axesList;
  QList <Control> m_buttonsList;

  int m_fd;
  QString m_fdName;

  pthread_t m_thread;

 private:
  void fillButtonNames(int buttons);
  void fillAxisNames(int axes);
  int fd() { return m_fd; }
  void update();

  void emitAxisEvent(int axis, int value);
  void emitButtonEvent(int button, int value);

 private:
  void processJoystickEvent(struct js_event e);
};

class JoystickEvent
{
 public:
  unsigned long time;
  signed int value;

  char type;
  char number;
};

#endif
