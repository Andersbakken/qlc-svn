/*
  Q Light Controller
  midiout.h

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

#ifndef SOUNDTOLIGHT_H
#define SOUNDTOLIGHT_H

#include "../common/inputplugin.h"
#include <qthread.h>
#include <qptrlist.h>

class QString;

extern "C" InputPlugin* create(t_plugin_id id);
extern "C" void destroy(InputPlugin* object);

class ConfigureSoundToLight;

class SoundToLight : public InputPlugin
{
  Q_OBJECT

    friend class ConfigureSoundToLight;

 public:
  SoundToLight(t_plugin_id id);
  virtual ~SoundToLight();

  int open();
  int close();
  bool isOpen();
  int configure();
  QString infoText();
  void contextMenu(QPoint pos);

  int setConfigDirectory(QString dir);
  int saveSettings();
  int loadSettings();

  // Own functions
  int setDeviceName(QString name);
  QString deviceName() { return m_deviceName; }

 protected:
  void setFileName(QString);
  void activate();
  void createContents(QPtrList <QString> &list);

 protected slots:
  void slotContextMenuCallback(int);


 private:
  QString m_deviceName;
  QString m_configDirectory;
  int m_dsp;

  QMutex m_mutex;
};

#endif

