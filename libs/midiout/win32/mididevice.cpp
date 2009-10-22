/*
  Q Light Controller
  mididevice.cpp

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

#include <QApplication>
#include <QSettings>
#include <QObject>
#include <QString>
#include <QDebug>

#include "midiprotocol.h"
#include "mididevice.h"
#include "midiout.h"

MIDIDevice::MIDIDevice(MIDIOut* parent, UINT id) : QObject(parent)
{
	m_id = id;
	m_handle = NULL;

	/* Start with all values zeroed */
	std::fill(m_values, m_values + MAX_MIDI_DMX_CHANNELS, 0);

	/* Get a name for this device */
	extractName();

	/* Load global settings */
	loadSettings();
}

MIDIDevice::~MIDIDevice()
{
	close();
}

void MIDIDevice::loadSettings()
{
	QSettings settings;
	QVariant value;
	QString key;

	/* Attempt to get a MIDI channel from settings */
	key = QString("/midiout/%1/midichannel").arg(m_name);
	value = settings.value(key);
	if (value.isValid() == true)
		setMidiChannel(value.toInt());
	else
		setMidiChannel(0);

	/* Attempt to get the mode from settings */
	key = QString("/midiout/%1/mode").arg(m_name);
	value = settings.value(key);
	if (value.isValid() == true)
		setMode(stringToMode(value.toString()));
	else
		setMode(ControlChange);
}

void MIDIDevice::saveSettings()
{
	QSettings settings;
	QString key;

	/* Store MIDI channel to settings */
	key = QString("/midiout/%1/midichannel").arg(m_name);
	settings.setValue(key, m_midiChannel);

	/* Store mode to settings */
	key = QString("/midiout/%1/mode").arg(m_name);
	settings.setValue(key, MIDIDevice::modeToString(m_mode));
}

/****************************************************************************
 * File operations
 ****************************************************************************/

bool MIDIDevice::open()
{
	bool result = false;
	MMRESULT res;

	if (m_handle != NULL)
		return true;

	res = midiOutOpen(&m_handle, m_id, 0, 0, 0);
	if (res == MMSYSERR_ALLOCATED)
	{
		qDebug() << QString("Unable to open %1:").arg(m_name)
			 << QString("Resource is already allocated.");
		m_handle = NULL;
		result = false;
	}
	else if (res == MMSYSERR_BADDEVICEID)
	{
		qDebug() << QString("Unable to open %1:").arg(m_name)
			 << QString("Bad device ID.");
		m_handle = NULL;
		result = false;
	}
	else if (res == MMSYSERR_INVALFLAG)
	{
		qDebug() << QString("Unable to open %1:").arg(m_name)
			 << QString("Invalid flags.");
		m_handle = NULL;
		result = false;
	}
	else if (res == MMSYSERR_INVALPARAM)
	{
		qDebug() << QString("Unable to open %1:").arg(m_name)
			 << QString("Invalid parameters.");
		m_handle = NULL;
		result = false;
	}
	else if (res == MMSYSERR_NOMEM)
	{
		qDebug() << QString("Unable to open %1:").arg(m_name)
			 << QString("Out of memory.");
		result = false;
	}
	else
	{
		qDebug() << QString("%1 opened.").arg(m_name);
		result = true;
	}

	return result;
}

void MIDIDevice::close()
{
	MMRESULT res;

	if (m_handle == NULL)
		return;

	res = midiOutClose(m_handle);
	if (res == MIDIERR_STILLPLAYING)
	{
		qDebug() << QString("Unable to close %1: Buffer not empty")
				.arg(m_name);
	}
	else if (res == MMSYSERR_INVALHANDLE)
	{
		qDebug() << QString("Unable to close %1: Invalid handle")
				.arg(m_name);
	}
	else if (res == MMSYSERR_NOMEM)
	{
		qDebug() << QString("Unable t close %1: Out of memory")
				.arg(m_name);
	}
	else
	{
		qDebug() << QString("%1 closed successfully.").arg(m_name);
		m_handle = NULL;
	}
}

/*****************************************************************************
 * Device info
 *****************************************************************************/

QString MIDIDevice::infoText()
{
	QString info;

	info += QString("<B>%1</B>").arg(name());
	info += QString("<P>");
	info += QString("Device is working correctly.");
	info += QString("</P>");
	info += QString("<P>");
	info += QString("<B>MIDI Channel: </B>%1<BR>").arg(m_midiChannel + 1);
	info += QString("<B>Mode: </B>%1").arg(modeToString(m_mode));
	info += QString("</P>");

	return info;
}

QString MIDIDevice::name() const
{
	return m_name;
}

void MIDIDevice::extractName()
{
	MMRESULT res;
	MIDIOUTCAPS caps;

	res = midiOutGetDevCaps(m_id, &caps, sizeof(MIDIOUTCAPS));
	if (res == MMSYSERR_BADDEVICEID)
	{
		m_name = QString("Bad device ID");
		qDebug() << "MIDI OUT" << m_id + 1 << "has bad device ID";
	}
	else if (res == MMSYSERR_INVALPARAM)
	{
		m_name = QString("Invalid parameters");
		qDebug() << "Invalid params for MIDI OUT device" << m_id + 1;
	}
	else if (res == MMSYSERR_NODRIVER)
	{
		m_name = QString("No driver installed");
		qDebug() << "MIDI OUT device" << m_id + 1 << "has no driver";
	}
	else if (res == MMSYSERR_NOMEM)
	{
		m_name = QString("Out of memory");
		qDebug() << "Out of memory while opening MIDI OUT" << m_id + 1;
	}
	else
	{
		m_name = QString::fromWCharArray(caps.szPname);
	}
}

/*****************************************************************************
 * Operational mode
 *****************************************************************************/

QString MIDIDevice::modeToString(Mode mode)
{
	switch (mode)
	{
	default:
	case ControlChange:
		return QString("Control Change");
		break;
	case Note:
		return QString("Note Velocity");
		break;
	}
}

MIDIDevice::Mode MIDIDevice::stringToMode(const QString& mode)
{
	if (mode == QString("Note Velocity"))
		return Note;
	else
		return ControlChange;
}

/****************************************************************************
 * Write
 ****************************************************************************/

void MIDIDevice::writeRange(t_value* values, t_channel num)
{
	int i;
	union
	{
		DWORD dwData;
		BYTE bData[4];
	} msg;

	switch (m_mode)
	{
	default:
	case ControlChange:
		/* Use control change numbers as DMX channels and
		   control values as DMX channel values */
		for (i = 0; i < MAX_MIDI_DMX_CHANNELS; i++)
		{
			if (m_values[i] != values[i])
			{
				m_values[i] = values[i];

				msg.bData[0] = MIDI_CONTROL_CHANGE;
				msg.bData[1] = (BYTE) i;
				msg.bData[2] = (BYTE) values[i] >> 1;
				msg.bData[3] = 0;

				/* Push the message out */
				midiOutShortMsg(m_handle, msg.dwData);
			}
		}
		break;

	case Note:
		/* Use note numbers as DMX channels and velocities as
		   DMX channel values. 0 is written as note off */
		for (i = 0; i < MAX_MIDI_DMX_CHANNELS; i++)
		{
			if (m_values[i] != values[i])
			{
				m_values[i] = values[i];

				if (values[i] == 0)
				{
					msg.bData[0] = MIDI_NOTE_OFF;
					msg.bData[1] = (BYTE) i;
					msg.bData[2] = 0;
					msg.bData[3] = 0;
				}
				else
				{
					msg.bData[0] = MIDI_NOTE_ON;
					msg.bData[1] = (BYTE) i;
					msg.bData[2] = (BYTE) values[i] >> 1;
					msg.bData[3] = 0;
				}

				/* Push the message out */
				midiOutShortMsg(m_handle, msg.dwData);
			}
		}
		break;
	}
}
