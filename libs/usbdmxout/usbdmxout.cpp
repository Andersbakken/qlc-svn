/*
  Q Light Controller
  dmx4linuxout.cpp
  
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

#include "usbdmxout.h"
#include "configureusbdmxout.h"
#include "../common/filehandler.h"
//#include "usbdmx.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <qthread.h>
#include <qstring.h>
#include <qpoint.h>
#include <qpopupmenu.h>
#include <qfile.h>
#include <assert.h>
#include <errno.h>
#include <linux/errno.h>

#define CONF_FILE "usbdmxout.conf"

#define ID_CONFIGURE      10
#define ID_ACTIVATE       20

static QMutex _mutex;

//
// Exported functions
//
extern "C" OutputPlugin* create(t_plugin_id id)
{
  return new UsbDmxOut(id);
}

extern "C" void destroy(OutputPlugin* object)
{
  delete object;
}

//
// Class implementation
//
UsbDmxOut::UsbDmxOut(t_plugin_id id) : OutputPlugin(id)
{
  m_device = -1;
  m_name = QString("USB DMX Rodin Output");
  m_type = OutputType;
  m_version = 0x00010000;
  m_deviceName = QString("/dev/usbdmx");    
  m_firstDeviceID = 0;
  m_configDir = QString("~/.qlc/");

  for (t_channel i = 0; i < KChannelMax; i++)
    {
      m_values[i] = 0;
    }
}

UsbDmxOut::~UsbDmxOut()
{
}

/* Attempt to open dmx device */
int UsbDmxOut::open()
{
  if (m_device != -1)
    {
      qDebug("DMX4Linux already open");
      return false;
    }
 QString txt;
 txt.sprintf("%d",m_firstDeviceID);
txt = m_deviceName +txt;
  qDebug("First USB2DMX Device:  "+ txt);
  m_device = ::open((const char*) txt, O_RDWR | O_NONBLOCK);
  if (m_device == -1)
    {
      perror("open");
      qDebug("USB-DMX output is not available");
    }

  return errno;
}

int UsbDmxOut::close()
{
  int r = 0;
  r = ::close(m_device);
  if (r == -1)
    {
      perror("close");
    }
  else
    {
      m_device = -1;
    }

  return r;
}

bool UsbDmxOut::isOpen()
{
  if (m_device == -1)
    {
      return false;
    }
  else
    {
      return true;
    }
}

int UsbDmxOut::configure()
{
  ConfigureUsbDmxOut* conf = new ConfigureUsbDmxOut(this);

  if (conf->exec() == QDialog::Accepted)
    {
      m_deviceName = conf->device();
      m_firstDeviceID = conf->firstDeviceID();
      saveSettings();
    }

  delete conf;

  return 0;
}

QString UsbDmxOut::infoText()
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

  str += QString("</TR>");
  str += QString("</TABLE>");
  str += QString("</BODY></HTML>");

  return str;
}

int UsbDmxOut::setConfigDirectory(QString dir)
{
  m_configDir = dir;
  return 0;
}

int UsbDmxOut::saveSettings()
{
  QString s;
  QString t;

  QString fileName = m_configDir + QString(CONF_FILE);
  qDebug(fileName);
  QFile file(fileName);

  if (file.open(IO_WriteOnly))
    {
      // Comment
      s = QString("# UsbDmxOut Plugin Configuration\n");
      file.writeBlock((const char*) s, s.length());

      // Entry type
      s = QString("Entry = ") + name() + QString("\n");
      file.writeBlock((const char*) s, s.length());

      s = QString("Device = ") + m_deviceName + QString("\n");
      file.writeBlock((const char*) s, s.length());

      t.sprintf("%d",m_firstDeviceID);
      s = QString("DeviceFirstID = ") + t + QString("\n");
      file.writeBlock((const char*) s, s.length());


      file.close();
    }
  else
    {
      perror("file.open");
      qDebug("Unable to save UsbDmxOut configuration");
    }

  return errno;
}

int UsbDmxOut::loadSettings()
{
  QString fileName;
  QPtrList <QString> list;

  fileName = m_configDir + QString(CONF_FILE);

  FileHandler::readFileToList(fileName, list);

  for (QString* s = list.first(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.next();
	  if (*s == name())
	    {
	      createContents(list);
	    }
	}
    }

  return 0;
}

void UsbDmxOut::createContents(QPtrList <QString> &list)
{
  for (QString* s = list.next(); s != NULL; s = list.next())
    {
      if (*s == QString("Entry"))
	{
	  s = list.prev();
	  break;
	}
      else if (*s == QString("Device"))
	{
	  m_deviceName = *(list.next());
	}
       else if (*s == QString("DeviceFirstID"))
	{
	  m_firstDeviceID = list.next()->toInt();
	}
    }
}

void UsbDmxOut::contextMenu(QPoint pos)
{
  QPopupMenu* menu = new QPopupMenu();
  menu->insertItem("Configure...", ID_CONFIGURE);
  menu->insertSeparator();
  menu->insertItem("Activate", ID_ACTIVATE);

  connect(menu, SIGNAL(activated(int)), 
	  this, SLOT(slotContextMenuCallback(int)));
  menu->exec(pos, 0);
  delete menu;
}

void UsbDmxOut::slotContextMenuCallback(int item)
{
  switch(item)
    {
    case ID_CONFIGURE:
      configure();
      break;

    case ID_ACTIVATE:
      activate();
      break;

    default:
      break;
    }
}

void UsbDmxOut::activate()
{
  emit activated(this);
}

//
// Write a value to a channel
//
int UsbDmxOut::writeChannel(t_channel channel, t_value value)
{
  int r = 0;
  unsigned char buf[512];

  _mutex.lock();

  m_values[channel] = value;

  ioctl(m_device, DMX_MEM_MAP_SET, DMX_TX_MEM);
  lseek(m_device, channel, SEEK_SET);
  r = write(m_device, &value, 1);
  if (r == -1)
    {
      perror("write channel");
    }


/* read in for testing purpose
   ioctl(m_device, DMX_BLOCKING_SET, 0);
   ioctl(m_device, DMX_MEM_MAP_SET, DMX_RX_MEM);
   lseek(m_device, 0, SEEK_SET);   */

  /* read data */
/*  read(m_device, buf, sizeof(buf));*/
 /* QString txt;
  
  txt.sprintf("Channel %d   value %d",channel,value);
  qDebug(txt);*/


	 
  _mutex.unlock();

  return r;
}

//
// Write num values starting from address
//
int UsbDmxOut::writeRange(t_channel address, t_value* values, t_channel num)
{
/*
  assert(values);
  int r = 0;

  _mutex.lock();

  memcpy(m_values + address, values, num * sizeof(t_value));

  lseek(m_device, address, SEEK_SET);
  r = write(m_device, values, num);
  if (r == -1)
    {
      perror("write");
    }

  _mutex.unlock();
  return r;

*/
  return 0;
}

//
// Read a channel's value
//
int UsbDmxOut::readChannel(t_channel channel, t_value &value)
{
  _mutex.lock();
  value = m_values[channel];
  _mutex.unlock();

  return 0;
}

//
// Read num channel's values starting from address
//
int UsbDmxOut::readRange(t_channel address, t_value* values, t_channel num)
{
  assert(values);

  _mutex.lock();
  memcpy(values, m_values + address, num * sizeof(t_value));
  _mutex.unlock();

  return 0;
}
