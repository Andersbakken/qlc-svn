/*
  Q Light Controller
  joystickplugin.cpp

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

#include "joystickplugin.h"
#include "joystick.h"
#include "selectjoystick.h"

#include <qpopupmenu.h>

#define JS_MAX_NUM         32
#define ID_CONFIGURE       10
#define CONF_FILE          "joystickplugin.conf"

//
// Exported functions
//
extern "C" Plugin* create(t_plugin_id id)
{
  return new JoystickPlugin(id);
}

extern "C" void destroy(Plugin* object)
{
  delete object;
}

//
// Class implementation
//
JoystickPlugin::JoystickPlugin(t_plugin_id id) : Plugin(id)
{
  m_name = QString("Joystick Input");
  m_type = Plugin::InputType;
  m_version = 0x00000100;
  m_configDirectory = QString("~/.qlc/");
}

JoystickPlugin::~JoystickPlugin()
{
}

int JoystickPlugin::open()
{
  t_joystick_id i = 0;
  char fileName[256];
  Joystick* j = NULL;

  for (i = 0; i < JS_MAX_NUM; i++)
    {
      sprintf(fileName, "/dev/js%d", i);
      j = new Joystick();
      j->create((const char*) fileName);

      if (j->valid() == true)
	{
	  m_joystickList.append(j);
	}
      else
	{
	  delete j;
	  j = NULL;
	}
    }

  return 0;
}

int JoystickPlugin::close()
{
  while (m_joystickList.isEmpty() == false)
    {
      delete m_joystickList.take(0);
    }

  return 0;
}

bool JoystickPlugin::isOpen()
{
  return true;
}

int JoystickPlugin::configure()
{
  return 0;
}

QString JoystickPlugin::infoText()
{
  QString t;
  QString str = QString::null;
  str += QString("<HTML><HEAD><TITLE>Plugin Info</TITLE></HEAD><BODY>");
  str += QString("<TABLE COLS=\"1\" WIDTH=\"100%\"><TR>");
  str += QString("<TD BGCOLOR=\"black\"><FONT COLOR=\"white\" SIZE=\"5\">");
  str += name() + QString("</FONT></TD></TR></TABLE>");
  str += QString("<TABLE COLS=\"2\" WIDTH=\"100%\">");
  str += QString("<TR>\n");
  str += QString("<TD><B>Version</B></TD>");
  str += QString("<TD>");
  t.setNum((version() >> 16) & 0xff);
  str += t + QString(".");
  t.setNum((version() >> 8) & 0xff);
  str += t + QString(".");
  t.setNum(version() & 0xff);
  str += t + QString("</TD>");
  str += QString("</TR>");

  str += QString("</TR>");
  str += QString("</TABLE>");

  str += QString("<H3>NOTE</H3>");
  str += QString("<P>Joystick plugin is not fully implemented. ");
  str += QString("You cannot use or configure it yet.</P>");
  str += QString("</BODY></HTML>");

  return str;
}

int JoystickPlugin::setConfigDirectory(QString dir)
{
  m_configDirectory = dir;
  return 0;
}

int JoystickPlugin::saveSettings()
{
  return -1;
}

int JoystickPlugin::loadSettings()
{
  return -1;
}

void JoystickPlugin::contextMenu(QPoint pos)
{
  QPopupMenu* menu = new QPopupMenu();
  menu->insertItem("Configure...", ID_CONFIGURE);

  connect(menu, SIGNAL(activated(int)), 
	  this, SLOT(slotContextMenuCallback(int)));
  menu->exec(pos, 0);
  delete menu;
}

void JoystickPlugin::slotContextMenuCallback(int item)
{
  switch(item)
    {
    case ID_CONFIGURE:
      break;

    default:
      break;
    }
}

Joystick* JoystickPlugin::selectJoystick()
{
  SelectJoystick* sj = new SelectJoystick(this);
  sj->initView();

  if (sj->exec() == QDialog::Accepted)
    {
      return sj->joystick();
    }
  else
    {
      return NULL;
    }
}

Joystick* JoystickPlugin::search(QString &device)
{
  for (Joystick* j = m_joystickList.first(); j != NULL; 
       j = m_joystickList.next())
    {
      if (j->fdName() == device)
	{
	  return j;
	}
    }
  
  return NULL;
}
