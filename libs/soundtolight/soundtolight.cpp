/*
  Q Light Controller
  midiout.cpp

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

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <stdio.h>
#include <errno.h>

#include <qstring.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qfile.h>

#include "../common/plugin.h"
#include "../common/filehandler.h"
#include "../../main/src/types.h"

#include "soundtolight.h"
#include "configuresoundtolight.h"

#define CONF_FILE          "soundtolight.conf"

#define ID_CONFIGURE       10
#define ID_ACTIVATE        20

//
// Exported functions
//
extern "C" InputPlugin* create(t_plugin_id id)
{
  return new SoundToLight(id);
}

extern "C" void destroy(InputPlugin* object)
{
  delete object;
}

//
// Class implementation
//
SoundToLight::SoundToLight(t_plugin_id id) : InputPlugin(id)
{
  m_dsp = -1;
  m_name = QString("Sound2Light");
  m_type = InputType;
  m_version = 0x00000100;
  m_deviceName = QString("/dev/dsp");
  m_configDirectory = QString("~/.qlc/");
}

SoundToLight::~SoundToLight()
{

}

void SoundToLight::setFileName(QString fileName)
{
  m_deviceName = fileName;
}

int SoundToLight::setDeviceName(QString name)
{
  m_deviceName = name;
  return 0;
}

int SoundToLight::open()
{
  int format, tstereo, speed, caps;
  bool fullduplex;

  if (m_dsp != -1)
    {
      qDebug("Sound device already open");
      return false;
    }

  if((m_dsp = ::open((const char*) m_deviceName, O_RDWR | O_NONBLOCK)) == -1)
    {
      perror("open");
      qDebug("Sound device is not available");
      return false;
    }
  else
    {
      if (fcntl(m_dsp, F_SETFL, 0) < 0)
        {
	  ::close(m_dsp);
	  perror("can't switch to blocking mode");
	  return false;
	}
      qDebug("Found soundcard on /dev/dsp");
    }

  format = AFMT_S16_LE;

  tstereo = 1; /* only stereo _DSP_ in/out */
  speed = 44100; // SAMPLE_RATE 44100hz mixing */

  ioctl(m_dsp, SNDCTL_DSP_GETCAPS, &caps);

  if (caps & DSP_CAP_DUPLEX)
    {
      fullduplex = true;
      qDebug("full duplex supported. good");
      ioctl(m_dsp, SNDCTL_DSP_SETDUPLEX, 0);
      ioctl(m_dsp, DSP_CAP_DUPLEX, 0);
    }
  else
    {
      qDebug("only halfduplex is supported");
      fullduplex = false;
    }

  if (ioctl(m_dsp, SNDCTL_DSP_SAMPLESIZE, &format) <0)
    perror("failed to set dsp samplesize");

  /* CHANNELS */
  if (ioctl(m_dsp, SNDCTL_DSP_STEREO, &tstereo) == -1)
    perror("something went wrong with the stereo setting");

  /* SAMPLERATE */
  if (ioctl(m_dsp, SNDCTL_DSP_SPEED, &speed) == -1)
    perror("speed setting failed");

  qDebug("mixing 16bit %dHz stereo",speed);


  return errno;
}

int SoundToLight::close()
{
  int r = 0;

  r = ::close(m_dsp);
  if (r == -1)
    {
      perror("close");
    }
  else
    {
      m_dsp = -1;
    }

  return r;
}

bool SoundToLight::isOpen()
{
  if (m_dsp == -1)
    {
      return false;
    }
  else
    {
      return true;
    }
}

QString SoundToLight::infoText()
{
  QString t;
  QString str = QString::null;
  str += QString("<HTML><HEAD><TITLE>Plugin Info</TITLE></HEAD><BODY>");
  str += QString("<TABLE COLS=\"1\" WIDTH=\"100%\"><TR>");
  str += QString("<TD BGCOLOR=\"black\"><FONT COLOR=\"white\" SIZE=\"5\">");
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

/*  str += QString("<TR><TD><B>Midi Channel</B></TD>");
  t.setNum(m_midiChannel);
  str += QString("<TD>") + t + QString("</TD></TR>");

  str += QString("<TR><TD><B>First Note Number</B></TD>");
  t.setNum(m_firstNote);
  str += QString("<TD>") + t + QString("</TD></TR>"); */

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
  str += QString("</BODY></HTML>");

  return str;
}

int SoundToLight::setConfigDirectory(QString dir)
{
  m_configDirectory = dir;
  return 0;
}

int SoundToLight::saveSettings()
{
  QString s;
  QString t;

  QString fileName = m_configDirectory + QString(CONF_FILE);
  qDebug(fileName);
  QFile file(fileName);

  if (file.open(IO_WriteOnly))
    {
      // Comment
      s = QString("# Sound to Light Plugin Configuration\n");
      file.writeBlock((const char*) s, s.length());

      // Entry type
      s = QString("Entry = ") + name() + QString("\n");
      file.writeBlock((const char*) s, s.length());

      s = QString("Device = ") + m_deviceName + QString("\n");
      file.writeBlock((const char*) s, s.length());

      file.close();
    }
  else
    {
      perror("file.open");
    }

  return -1;
}

int SoundToLight::loadSettings()
{
  QString fileName;
  QPtrList <QString> list;

  fileName = m_configDirectory + QString(CONF_FILE);

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

void SoundToLight::createContents(QPtrList <QString> &list)
{
  QString t;

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
    }
}

int SoundToLight::configure()
{
  ConfigureSoundToLight* conf = new ConfigureSoundToLight(this);

  if (conf->exec() == QDialog::Accepted)
    {
      m_deviceName = conf->device();
      saveSettings();
    }

  return 0;
}

void SoundToLight::activate()
{
  emit activated(this);
}

void SoundToLight::contextMenu(QPoint pos)
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

void SoundToLight::slotContextMenuCallback(int item)
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
