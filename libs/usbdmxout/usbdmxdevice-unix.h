/*
  Q Light Controller
  usbdmxdevice-unix.h
  
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

#include <common/qlctypes.h>
#include <QObject>
#include <QMutex>
#include <QFile>

class QString;

class USBDMXDevice : public QObject
{
	Q_OBJECT

	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	USBDMXDevice(QObject* parent, const QString &path, t_output output);
	virtual ~USBDMXDevice();

protected:
	QString extractName();

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
	t_output m_output;

	/********************************************************************
	 * Open & close
	 ********************************************************************/
public:
	bool open();
	bool close();

protected:
	QFile m_file;

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
	QMutex m_mutex;
};

#endif
