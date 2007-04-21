/*
  Q Light Controller
  deviceclass.h
  
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

#ifndef DEVICECLASS_H
#define DEVICECLASS_H

#include <qobject.h>
#include <qptrlist.h>
#include <qstring.h>

#include "types.h"

class LogicalChannel;

class DeviceClass;
class DeviceClass
{
 public:
	DeviceClass();
	DeviceClass(DeviceClass *dc);
	~DeviceClass();
	
	QString name() { return m_manufacturer + QString(" ") + m_model; }
	
	void setManufacturer(const QString mfg);
	QString manufacturer() const { return m_manufacturer; }

	void setModel(const QString model);
	QString model() const { return m_model; }

	void setImageFileName(const QString &fileName);
	QString imageFileName() const { return m_imageFileName; }

	void setType(const QString &type);
	QString type() const { return m_type; }

	void addChannel(LogicalChannel* channel);
	bool removeChannel(LogicalChannel* channel);
	QPtrList <LogicalChannel> *channels() { return &m_channels; }

	// These functions create the deviceclass' contents when reading
	// settings from a workspace file
	void createInfo(QPtrList <QString> &list);
	void createChannel(QPtrList <QString> &list);

	// Returns a QT error code
	int saveToFile(const QString &fileName);

 private:
	QString m_manufacturer;
	QString m_model;
	QString m_imageFileName;
	QString m_type;

	QPtrList <LogicalChannel> m_channels;

 public:
	static DeviceClass* createDeviceClass(QPtrList <QString> &list);
};

#endif
