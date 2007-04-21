/*
  Q Light Controller
  deviceconsole.h
  
  Copyright (C) Heikki Junnila
  
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

#ifndef DEVICECONSOLE_H
#define DEVICECONSOLE_H

#include <qwidget.h>
#include <qptrlist.h>

#include "common/types.h"
#include "consolechannel.h"

class QCloseEvent;
class QHBoxLayout;

class Device;
class SceneEditor;

class DeviceConsole : public QWidget
{
	Q_OBJECT

public: 
	DeviceConsole(QWidget *parent);
	~DeviceConsole();

	void setDevice(t_device_id);

	QPtrList <ConsoleChannel> unitList() { return m_unitList; }
	SceneEditor* sceneEditor() { return m_sceneEditor; }

signals:
	void closed();

protected:
	void closeEvent(QCloseEvent*);

protected:
	QHBoxLayout* m_layout;

	SceneEditor* m_sceneEditor;
	t_device_id m_deviceID;

	QPtrList <ConsoleChannel> m_unitList;
};

#endif
