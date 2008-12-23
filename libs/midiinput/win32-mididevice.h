/*
  Q Light Controller
  win32-mididevice.h

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

#ifndef MIDIDEVICE_H
#define MIDIDEVICE_H

#include <Windows.h>
#include <QObject>

#include "common/qlctypes.h"

class MIDIDevice;
class MIDIInput;
class QEvent;

/*****************************************************************************
 * MIDIDevice
 *****************************************************************************/

class MIDIDevice : public QObject
{
	Q_OBJECT

public:
	MIDIDevice(MIDIInput* parent, UINT id);
	virtual ~MIDIDevice();

	/*********************************************************************
	 * File operations
	 *********************************************************************/
public:
	/** Open the device for input data */
	bool open();

	/** Close the input device */
	void close();
	
protected:
	HMIDIIN m_handle;

	/*********************************************************************
	 * ID/Input
	 *********************************************************************/
public:
	/** Get this input device's input line number */
	t_input input() const;

protected:
	UINT m_id;

	/*********************************************************************
	 * Device info
	 *********************************************************************/
public:
	/**
	 * Get device information string to be used in plugin manager
	 */
	QString infoText();

	/**
	 * Get the device's name
	 */
	QString name();

protected:
	QString m_name;

	/*********************************************************************
	 * Input data
	 *********************************************************************/
protected:
	void customEvent(QEvent* event);

signals:
	/** Signal that is emitted when an input channel's value is changed */
	void valueChanged(MIDIDevice* device, t_input_channel channel,
			  t_input_value value);

	/*********************************************************************
	 * Feedback
	 *********************************************************************/
public:
	/** Get this input device's feedback output line number */
	UINT feedBackId() const;
	
	/** Set this input device's feedback output line number */
	void setFeedBackId(UINT id);

	/** Open the device for feedback output */
	void openOutput();

	/** Close the feedback device */
	void closeOutput();

	/** Get a list of available outputs for feedback */
	static QStringList feedBackNames();

public:
	/** Send input values back to the device to move motorized faders */
	void feedBack(t_input_channel channel, t_input_value value);

protected:
	UINT m_feedBackId;
	HMIDIOUT m_feedBackHandle;
};

#endif
