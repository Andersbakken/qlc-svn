/*
  Q Light Controller
  ftdidmxdevice.h
  
  Copyright (c) Christopher Staite
  
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

#ifndef FTDIDMXDEVICE_H
#define FTDIDMXDEVICE_H

#ifdef WIN32
#include <windows.h>
#include "ftd2xx-win32.h"
#else
#include "ftd2xx.h"
#endif

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QString>
#include <QThread>

#include "common/qlctypes.h"

#define MAX_NUM_DEVICES 50

class FTDIDMXDevice : public QThread
{
	Q_OBJECT

	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	FTDIDMXDevice(QObject* parent, int vid, int pid, char *description,
		      t_output output);
	virtual ~FTDIDMXDevice();

	/********************************************************************
	 * Properties
	 ********************************************************************/
public:
	QString name() const;
	QString path() const;
	t_output output() const;

protected:
	QString m_name;
	QString m_path;
	int m_vid;
	int m_pid;
	BOOL m_isDmxPro;
	t_output m_output;

	/********************************************************************
	 * Threading
	 ********************************************************************/

public:
	virtual void run();

protected:
	bool m_threadRunning;

	/********************************************************************
	 * Open & close
	 ********************************************************************/
public:
	bool open();
	bool close();

protected:
	FT_HANDLE m_handle;

	/********************************************************************
	 * Read & write
	 ********************************************************************/
public:
	void write(t_channel channel, t_value value);
	void writeRange(t_channel channel, t_value* values, t_channel num);

	void read(t_channel channel, t_value* value);
	void readRange(t_channel channel, t_value* values, t_channel num);

protected:
	t_value m_values[512];
	bool m_dataChanged;
	QMutex m_mutex;
};

#endif
