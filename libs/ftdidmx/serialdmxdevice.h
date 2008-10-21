/*
  Q Light Controller
  serialdmxdevice.h
  
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

#ifndef SERIALDMXDEVICE_H
#define SERIALDMXDEVICE_H

#include <common/qlctypes.h>
#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QString>

class SerialDMXDevice : public QObject
{
	Q_OBJECT

	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	SerialDMXDevice(QObject* parent, const QString &path, t_output output);
	virtual ~SerialDMXDevice();

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

	/*******************************************************************
	 * Device code
	 *******************************************************************/
	
public slots:
	void performDeviceWrite();
	
private:
	bool detectDeviceType(const QString &path);
	void applyDeviceOptions();
	void dataChanged();

protected:
	QTimer *m_updateTimer;
	
	// How long should I wait between updates if no data is changed
	// in milliseconds set to < 0 to only update on data change
	int m_updateTimeout;
	// Force the update to wait for timer rather than new data
	bool m_forceTimeoutWait;
	// Which function should I run to write the data to the device
	// NOTE: the mutex is LOCKED when this function is called, and
	//       automatically unlocked on return.  Therefore the values
	//       will not change during execution.
	void (*m_updateFunction)(t_value values[]);
	int  (*m_openFunction)(const char *path);
	int  (*m_closeFunction)();
	
	/********************************************************************
	 * Open & close
	 ********************************************************************/
public:
	bool open();
	bool close();

	/********************************************************************
	 * Read & write
	 ********************************************************************/
public:
	void write(t_channel channel, t_value value);
	void writeRange(t_channel channel, t_value* values, t_channel num);

	void read(t_channel channel, t_value* value);
	void readRange(t_channel channel, t_value* values, t_channel num);

protected:
	// Extra byte for start byte
	t_value m_values[513];
	QMutex m_mutex;
};

#endif
