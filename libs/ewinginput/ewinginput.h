/*
  Q Light Controller
  ewinginput.h

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

#ifndef EWINGINPUT_H
#define EWINGINPUT_H

#include <QHostAddress>
#include <QStringList>
#include <QList>

#include "common/qlcinplugin.h"
#include "common/qlctypes.h"

class QUdpSocket;
class EWing;

/*****************************************************************************
 * EWingInput
 *****************************************************************************/

class EWingInput : public QObject, public QLCInPlugin
{
	Q_OBJECT
	Q_INTERFACES(QLCInPlugin)

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	void init();
	~EWingInput();

	void open(t_input input = 0);
	void close(t_input input = 0);

protected slots:
	void slotReadSocket();

	/*********************************************************************
	 * Devices
	 *********************************************************************/
protected:
	EWing* device(const QHostAddress& address);
	EWing* device(int index);

	void addDevice(EWing* device);
	void removeDevice(EWing* device);

signals:
	void deviceAdded(EWing* device);
	void deviceRemoved(EWing* device);

protected:
	QList <EWing*> m_devices;
	QUdpSocket* m_socket;
	
	/*********************************************************************
	 * Name
	 *********************************************************************/
public:
	QString name();

	/*********************************************************************
	 * Inputs
	 *********************************************************************/
public:
	QStringList inputs();

	/*********************************************************************
	 * Configuration
	 *********************************************************************/
public:
	void configure();

	/*********************************************************************
	 * Status
	 *********************************************************************/
public:
	QString infoText(t_input input = KInputInvalid);

	/*********************************************************************
	 * Input data
	 *********************************************************************/
protected slots:
	void slotValueChanged(t_input_channel channel, t_input_value value);

signals:
	void valueChanged(QLCInPlugin* plugin, t_input line,
			  t_input_channel channel, t_input_value value);

public:
	void connectInputData(QObject* listener);

	void feedBack(t_input input, t_input_channel channel,
		      t_input_value value);
};

#endif
