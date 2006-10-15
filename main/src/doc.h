/*
  Q Light Controller
  doc.h
  
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

#ifndef DOC_H
#define DOC_H

#include <qobject.h>
#include <qptrlist.h>

#include "function.h"
#include "device.h"
#include "deviceclass.h"
#include "bus.h"

class Doc : public QObject
{
  Q_OBJECT

 public:
  Doc();
  ~Doc();

  //
  // Modified status
  //
  bool isModified() { return m_modified; }
  void setModified(bool modified);

  //
  // Load & Save
  //
  QString fileName() { return m_fileName; }
  bool loadWorkspaceAs(QString &);
  bool saveWorkspaceAs(QString &);
  bool saveWorkspace();

  //
  // Devices
  //
  Device* newDevice(DeviceClass* dc, QString name,
		    t_channel address, t_channel universe,
		    t_device_id id = KNoID);

  void deleteDevice(t_device_id);
  Device* device(t_device_id);

  //
  // Functions
  //
  Function* newFunction(Function::Type,
			t_device_id device,
			t_function_id id = KNoID);
  void deleteFunction(t_function_id);
  Function* function(t_function_id);

  // Emit a functionChanged() signal
  void emitFunctionChanged(t_function_id);

 private slots:
  void slotModeChanged();

 signals:
  void deviceAdded(t_device_id);
  void deviceRemoved(t_device_id);
  void deviceChanged(t_device_id);

  void functionAdded(t_function_id);
  void functionRemoved(t_function_id);
  void functionChanged(t_function_id);

 private:
  QString m_fileName;
  bool m_modified;

  Function** m_functionArray;
  Device** m_deviceArray;
};

#endif

