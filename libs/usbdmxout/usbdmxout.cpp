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
  for(int n=0; n< MAXINTERFACES; n++)
      m_device[n] = -1;
  m_name = QString("USB DMX Peperoni Output");
  m_type = OutputType;
  m_version = 0x00010000;
  m_deviceName = QString("/dev/usbdmx");    
  m_firstDeviceID = 0;
  m_numOfDevices = 0;
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
  QString txt;
  int n =0;

  m_lastDeviceID = m_firstDeviceID - 1;
  
  for(n=0; n< MAXINTERFACES; n++){

    //check if device is already open:
    if (m_device[n] != -1)
      {
         txt.sprintf("Peperoni USB2DMX already open%d",m_firstDeviceID+n);
         qDebug("Peperoni USB2DMX already open");
         return false;
      }

   txt.sprintf("%d",m_firstDeviceID+n);
   txt = m_deviceName +txt;
   qDebug("Try to open USB2DMX Device:  "+ txt);
   m_device[n] = ::open((const char*) txt, O_RDWR | O_NONBLOCK);
   if (m_device[n] == -1)
     {
        //perror("open");
        //qDebug("USB-DMX output is not available");
     }
     else
     {
         m_lastDeviceID++;
     }
  }

  if(m_lastDeviceID < m_firstDeviceID)
    {
       qDebug("Error: no valid USB2DMX devices could be opened! USB-DMX output is not available!");
    }
    else 
    {
       for(n = m_firstDeviceID; n <= m_lastDeviceID; n++)
         {
	    txt.sprintf("USB2DMX Interface %d available for output!",n); 
            qDebug(txt);
         }
        m_numOfDevices = m_lastDeviceID - m_firstDeviceID +1;
    }
    return errno;
}

int UsbDmxOut::close()
{
 int n;
 for(n=0; n< MAXINTERFACES; n++){
  int r = 0;
  r = ::close(m_device[n]);
  if (r == -1)
    {
      perror("close");
    }
  else
    {
      m_device[n] = -1;
    }
}
  return 0;//r;
}

bool UsbDmxOut::isOpen()
{
  if (m_device[0] == -1)
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
  QString txt;
  _mutex.lock();

  m_values[channel] = value;

  //which interface should we write to?
  int ifaceNo = int(channel / 512);
  int channelNo = channel % 512;

  if(ifaceNo >= m_numOfDevices)
    {
         //qDebug(txt.sprintf("Error: Universe #%d  is not active on USB2DMX!",ifaceNo + 1));
    }
  else
    {
	//qDebug(txt.sprintf("Write to IFACE %d  on channel %d",ifaceNo, channelNo));
	ioctl(m_device[ifaceNo], DMX_MEM_MAP_SET, DMX_TX_MEM);
	lseek(m_device[ifaceNo], channelNo, SEEK_SET);
	r = write(m_device[ifaceNo], &value, 1);
	if (r == -1)
	  {
	      perror("write channel");
 	   }

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

  assert(values);
  int r = 0;
  QString txt;

  _mutex.lock();

  // which one is the first universe to write to?
  int device = int(address / 512) + m_firstDeviceID;
  //what is the first channel on that device?
  // eg. address is 512, it would be channel 0 on device 1
  int firstChannel = address % 512;
  // how many unverses?



  memcpy(m_values + address, values, num * sizeof(t_value));

   //write the first chunk
   int numToWrite = 512 - firstChannel;

   while(num > 0)
     {

         qDebug(txt.sprintf("writeRange:Universe #%d   FirstChannel #%d   Num #%d ", device, firstChannel, num));

         if(device > (m_lastDeviceID - m_firstDeviceID -1))
           {
              qDebug("Error: UsbDmxOut::writeRange() tried to write to a non-existing device!");
              _mutex.unlock();
              return -1;
           }
         else
           {
              lseek(m_device[device], firstChannel, SEEK_SET);
              r = write(m_device[device], values, numToWrite);
              if (r == -1)
                {
                    perror("write");
                }
              firstChannel = 0;
              device ++;
             num -= numToWrite;
             values += numToWrite;
             if(num >= 512)
                  numToWrite = 512;
             else
                  numToWrite = num;
	   }
      }

  
  _mutex.unlock();
  return r;


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
