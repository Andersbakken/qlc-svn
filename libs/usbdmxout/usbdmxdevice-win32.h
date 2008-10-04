/*
  Q Light Controller
  usbdmxdevice-win32.h
  
  Copyright (c) Heikki Junnila
  
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

#ifndef USBDMXDEVICE_H
#define USBDMXDEVICE_H

#define WIN32_LEAN_AND_MEAN
#include <common/qlctypes.h>
#include <windows.h>
#include <QObject>
#include <QMutex>

class USBDMXDevice : public QObject
{
	Q_OBJECT

	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	USBDMXDevice(QObject* parent, struct usbdmx_functions* usbdmx,
		     int output);
	virtual ~USBDMXDevice();

protected:
	struct usbdmx_functions* m_usbdmx;

	/********************************************************************
	 * Properties
	 ********************************************************************/
public:
	QString name() const;
	int output() const;

protected:
	QString m_name;
	int m_output;

	/********************************************************************
	 * Open & close
	 ********************************************************************/
public:
	bool open();
	bool close();
	HANDLE handle() const;

protected:
	HANDLE m_handle;
	
	/********************************************************************
	 * Read & write
	 ********************************************************************/
public:
	void write(t_channel channel, t_value value);
	t_value read(t_channel channel);

protected:
	t_value m_values[512];
	QMutex m_mutex;
};

#endif
