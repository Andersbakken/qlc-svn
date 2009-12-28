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

#include "qlcinplugin.h"
#include "qlctypes.h"

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

protected:
	/**
	 * Create a new wing object from the given datagram packet. Looks up
	 * the exact wing type from data and creates an EPlaybackWing,
	 * EShortcutWing or an EProgramWing.
	 *
	 * @param parent The parent object that owns the new wing object
	 * @param address The address of the physical wing board
	 * @param data A UDP datagram packet originating from a wing
	 *
	 * @return A new EWing object or NULL if an error occurred
	 */
	static EWing* createWing(QObject* parent, const QHostAddress& address,
				const QByteArray& data);

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
