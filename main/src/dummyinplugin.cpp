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
#include <assert.h>

#include "dummyinplugin.h"

#define ID_ACTIVATE        20

const QString DummyInPlugin::PluginName = QString("Dummy Input");

DummyInPlugin::DummyInPlugin(int id) : InputPlugin(id)
{
  m_open = false;
  m_version = 0x00010000;
  m_name = DummyInPlugin::PluginName;
  m_type = InputType;
}

DummyInPlugin::~DummyInPlugin()
{
}

int DummyInPlugin::open()
{
  qDebug("DummyIn Plugin opened");

  m_open = true;
  return 0;
}

int DummyInPlugin::close()
{
  qDebug("DummyIn Plugin closed");

  m_open = false;
  return 0;
}

bool DummyInPlugin::isOpen()
{
  return m_open;
}

int DummyInPlugin::configure()
{
  return 0;
}

QString DummyInPlugin::infoText()
{
  QString t;
  QString str = QString::null;
  str += QString("<HTML><HEAD><TITLE>Plugin Info</TITLE></HEAD><BODY>");
  str += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
  str += QString("<TR><TD BGCOLOR=\"black\">");
  str += QString("<FONT COLOR=\"white\" SIZE=\"5\">"); 
  str += name() + QString("</FONT></TD></TR></TABLE>");
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
  str += QString("<P>This plugin does absolutely nothing; ");
  str += QString("you can use this if you don't have ");
  str += QString("the necessary hardware for real control.</P>");

  str += QString("</BODY></HTML>");

  return str;
}

void DummyInPlugin::contextMenu(QPoint pos)
{
  QPopupMenu* menu = new QPopupMenu();
  menu->insertItem("Activate", ID_ACTIVATE);

  connect(menu, SIGNAL(activated(int)), 
	this, SLOT(slotContextMenuCallback(int)));
  menu->exec(pos, 0);
  delete menu;
}

void DummyInPlugin::slotContextMenuCallback(int item)
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

void DummyInPlugin::activate()
{
  emit activated(this);
}

int DummyInPlugin::setConfigDirectory(QString dir)
{
  return 0;
}

int DummyInPlugin::saveSettings()
{
  return -1;
}

int DummyInPlugin::loadSettings()
{
  return -1;
}
