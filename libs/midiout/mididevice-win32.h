/*
  Q Light Controller
  mididevice-win32.h

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
class MIDIOut;
class QString;

/*****************************************************************************
 * MIDIDevice
 *****************************************************************************/

class MIDIDevice : public QObject
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	MIDIDevice(MIDIOut* parent, UINT id);
	virtual ~MIDIDevice();

	/** Load global settings */
	void loadSettings();

	/** Save global settings */
	void saveSettings();

	/*********************************************************************
	 * File operations
	 *********************************************************************/
public:
	/** Attempt to open the device in write-only mode */
	bool open();

	/** Close the device */
	void close();

protected:
	HMIDIOUT m_handle;

	/*********************************************************************
	 * Output
	 *********************************************************************/
public:
	t_output output() const { return static_cast<t_output> (m_id); }

protected:
	UINT m_id;

	/*********************************************************************
	 * Device info
	 *********************************************************************/
public:
	/** Get device information string to be used in plugin manager */
	QString infoText();

	/** Get the device's name */
	QString name() const;

protected:
	/** Extract the name of this device */
	void extractName();

protected:
	/** The name of this MIDI device */
	QString m_name;

	/*********************************************************************
	 * Operational mode
	 *********************************************************************/
public:
	/**
	 * This device's operational mode.
	 *
	 * @ControlChange: Use MIDI ControlChange ID's as DMX channels
	 * @Note: Use MIDI Note ON/OFF commands as DMX channels
	 */
	enum Mode
	{
		ControlChange,
		Note
	};
	
	/** Get this device's operational mode */
	Mode mode() const { return m_mode; }

	/** Set this device's operational mode */
	void setMode(Mode m) { m_mode = m; }

	static QString modeToString(Mode mode);
	static Mode stringToMode(const QString& mode);

protected:
	Mode m_mode;

	/*********************************************************************
	 * MIDI channel
	 *********************************************************************/
public:
	/** Get this device's MIDI channel */
	t_channel midiChannel() const { return m_midiChannel; }

	/** Set this device's MIDI channel */
	void setMidiChannel(t_channel channel) { m_midiChannel = channel; }
	
protected:
	t_channel m_midiChannel;

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
};

#endif
