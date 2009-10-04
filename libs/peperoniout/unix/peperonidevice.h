/*
  Q Light Controller
  peperonidevice.h

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

#ifndef PEPERONIDEVICE_H
#define PEPERONIDEVICE_H

#include <common/qlctypes.h>
#include <QObject>
#include <QMutex>
#include <QFile>

class QString;

class PeperoniDevice : public QObject
{
	Q_OBJECT

	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	PeperoniDevice(QObject* parent, const QString& path);
	virtual ~PeperoniDevice();

protected:
	void extractName();

	/********************************************************************
	 * Properties
	 ********************************************************************/
public:
	QString name() const;
	QString path() const;

protected:
	QString m_name;
	QString m_path;

	/********************************************************************
	 * Open & close
	 ********************************************************************/
public:
	void open();
	void close();

protected:
	QFile m_file;

	/********************************************************************
	 * Write
	 ********************************************************************/
public:
	void writeRange(t_value* values, t_channel num);
};

#endif
