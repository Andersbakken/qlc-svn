/*
  Q Light Controller
  dummyoutplugin.cpp
  
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

#include <qstring.h>
#include <qpopupmenu.h>

#include "dummyoutplugin.h"

#define ID_ACTIVATE        20

DummyOutPlugin::DummyOutPlugin(int id) : OutputPlugin(id)
{
  m_open = false;
  m_version = 0x00010000;
  m_name = QString("Dummy Output");
}

DummyOutPlugin::~DummyOutPlugin()
{
}

bool DummyOutPlugin::open()
{
  qDebug("DummyOut Plugin opened");

  m_open = true;
  return true;
}

bool DummyOutPlugin::close()
{
  qDebug("DummyOut Plugin closed");

  m_open = false;
  return true;
}

bool DummyOutPlugin::isOpen()
{
  return m_open;
}

void DummyOutPlugin::configure()
{
}

QString DummyOutPlugin::infoText()
{
  QString t;
  QString str = QString::null;
  str += QString("<HTML><HEAD><TITLE>Plugin Info</TITLE></HEAD><BODY>");
  str += QString("<TABLE COLS=\"1\" WIDTH=\"100%\"><TR><TD BGCOLOR=\"black\"><FONT COLOR=\"white\" SIZE=\"5\">") + name() + QString("</FONT></TD></TR></TABLE>");
  str += QString("<TABLE COLS=\"2\" WIDTH=\"100%\">");

  str += QString("<TR><TD><B>Version</B></TD>");
  str += QString("<TD>");
  t.setNum((version() >> 16) & 0xff);
  str += t + QString(".");
  t.setNum((version() >> 8) & 0xff);
  str += t + QString(".");
  t.setNum(version() & 0xff);
  str += t + QString("</TD></TR>");

  str += QString("<TR>\n");
  str += QString("<TD><B>Status</B></TD>");
  str += QString("<TD>");
  if (isOpen() == true)
    {
      str += QString("<I>Active</I></TD>");
    }
  else
    {
      str += QString("Not Active</TD>");
    }
  str += QString("</TR>");
  str += QString("</TABLE>");

  str += QString("<H3>NOTE</H3>");
  str += QString("<P>This plugin does absolutely nothing; you can use this if you don't have ");
  str += QString("the necessary hardware for real DMX output.</P>");

  str += QString("</BODY></HTML>");

  return str;
}

void DummyOutPlugin::contextMenu(QPoint pos)
{
  QPopupMenu* menu = new QPopupMenu();
  menu->insertItem("Activate", ID_ACTIVATE);

  connect(menu, SIGNAL(activated(int)), this, SLOT(slotContextMenuCallback(int)));
  menu->exec(pos, 0);
  delete menu;
}

void DummyOutPlugin::slotContextMenuCallback(int item)
{
  switch(item)
    {
    case ID_ACTIVATE:
      activate();
      break;

    default:
      break;
    }
}

void DummyOutPlugin::activate()
{
  emit activated(this);
}

void DummyOutPlugin::setConfigDirectory(QString dir)
{
}

void DummyOutPlugin::saveSettings()
{
}

void DummyOutPlugin::loadSettings()
{
}

bool DummyOutPlugin::writeChannel(t_channel channel, t_value value)
{
  return true;
}

bool DummyOutPlugin::writeRange(t_channel address, t_value* values, t_channel num)
{
  return true;
}
