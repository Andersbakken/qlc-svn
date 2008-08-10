/*
  Q Light Controller
  hidinput.h

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

#ifndef HIDINPUT_H
#define HIDINPUT_H

#include <QEvent>
#include <QList>

#include "common/qlcinplugin.h"
#include "common/qlctypes.h"

#include "hiddevice.h"
#include "hidpoller.h"

/*****************************************************************************
 * HIDInputEvent
 *****************************************************************************/

class HIDInputEvent : public QEvent
{
public:
	HIDInputEvent(t_input input, t_input_channel channel,
		      t_input_value value);
	~HIDInputEvent();

	t_input m_input;
	t_input_channel m_channel;
	t_input_value m_value;
};

/*****************************************************************************
 * HIDInput
 *****************************************************************************/

class HIDInput : public QObject, public QLCInPlugin
{
	Q_OBJECT
	Q_INTERFACES(QLCInPlugin)

	friend class ConfigureHIDInput;
	friend class HIDPoller;

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	void init();
	~HIDInput();

	/*********************************************************************
	 * Devices
	 *********************************************************************/
protected:
	HIDDevice* device(const QString& path);
	HIDDevice* device(const unsigned int index);

protected:
	QList <HIDDevice*> m_devices;

	/*********************************************************************
	 * Name
	 *********************************************************************/
public:
	QString name();

	/*********************************************************************
	 * Inputs & channels
	 *********************************************************************/
public:
	t_input inputs();
	t_input_channel channels(t_input input);

	/*********************************************************************
	 * Configuration
	 *********************************************************************/
public:
	void configure();

	/*********************************************************************
	 * Status
	 *********************************************************************/
public:
	QString infoText();

	/*********************************************************************
	 * Device poller
	 *********************************************************************/
public:
	void addPollDevice(HIDDevice* device);
	void removePollDevice(HIDDevice* device);

protected:
	HIDPoller* m_poller;

	/*********************************************************************
	 * Input data
	 *********************************************************************/
protected:
	void customEvent(QEvent* event);

signals:
	void valueChanged(QLCInPlugin* plugin, t_input line,
			  t_input_channel channel, t_input_value value);

public:
	void connectInputData(QObject* listener);

	void feedBack(t_input input, t_input_channel channel,
		      t_input_value value);
};

#endif
