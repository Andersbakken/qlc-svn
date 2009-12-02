/*
  Q Light Controller
  enttecdmxusbproout.h

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,$
*/

#ifndef ENTTECDMXUSBPROOUT_H
#define ENTTECDMXUSBPROOUT_H

#include <QObject>

#include "common/qlcoutplugin.h"
#include "common/qlctypes.h"

class EnttecDMXUSBPro;

class EnttecDMXUSBProOut : public QObject, public QLCOutPlugin
{
	Q_OBJECT
	Q_INTERFACES(QLCOutPlugin)

	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	/** @reimp */
	void init();

	/** @reimp */
	void open(t_output output);

	/** @reimp */
	void close(t_output output);

#ifdef DBUS_ENABLED
protected slots:
	void slotDeviceAdded(const QString& name);
	void slotDeviceRemoved(const QString& name);
#endif

	/********************************************************************
	 * Devices (ENTTEC calls them "widgets" and so shall we)
	 ********************************************************************/
public:
	/** Find out which widgets are present in the system */
	void rescanWidgets();

	/** Find a widget that's using the given device path */
	EnttecDMXUSBPro* widget(const QString& path) const;

	/** @reimp */
	QStringList outputs();

protected:
	QList <EnttecDMXUSBPro*> m_widgets;

	/********************************************************************
	 * Name
	 ********************************************************************/
public:
	/** @reimp */
	QString name();

	/********************************************************************
	 * Configuration
	 ********************************************************************/
public:
	/** @reimp */
	void configure();

	/********************************************************************
	 * Plugin status
	 ********************************************************************/
public:
	/** @reimp */
	QString infoText(t_output output = KOutputInvalid);

	/********************************************************************
	 * Write
	 ********************************************************************/
public:
	/** @reimp */
	void outputDMX(t_output, const QByteArray& universe);
};

#endif
