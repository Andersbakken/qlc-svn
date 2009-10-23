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
#include <Windows.h>
#include <QSettings>
#include <QVariant>
#include <QObject>
#include <QString>
#include <QDebug>

#include "mididevice.h"
#include "midiinput.h"
#include "midiinputevent.h"
#include "midiprotocol.h"

MIDIDevice::MIDIDevice(MIDIInput* parent, UINT id) : QObject(parent)
{
	m_id = id;
	m_feedBackId = UINT_MAX;
	m_handle = NULL;
	m_feedBackHandle = NULL;
	m_isOK = false;
	m_mode = ControlChange;
	m_midiChannel = 0;

	extractName();
	loadSettings();
}

MIDIDevice::~MIDIDevice()
{
	saveSettings();
	close();
}

void MIDIDevice::loadSettings()
{
	QSettings settings;
	QVariant value;
	QString key;

	/* Attempt to load feedback line number */
	key = QString("/MIDIInput/Inputs/%1/FeedBackLine").arg(m_id);
	value = settings.value(key);
	if (value.isValid() == true && value.toUInt() != UINT_MAX)
		setFeedBackId(value.toUInt());


	/* Attempt to get a MIDI channel from settings */
	key = QString("/midiinput/%1/midichannel").arg(m_id);
	value = settings.value(key);
	if (value.isValid() == true)
		setMidiChannel(value.toInt());
	else
		setMidiChannel(0);

	/* Attempt to get the mode from settings */
	key = QString("/midiinput/%1/mode").arg(m_id);
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
	key = QString("/midiinput/%1/midichannel").arg(m_id);
	settings.setValue(key, m_midiChannel);

	/* Store mode to settings */
	key = QString("/midiinput/%1/mode").arg(m_id);
	settings.setValue(key, MIDIDevice::modeToString(m_mode));
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

static void postEvent(MIDIDevice* self, BYTE data1, BYTE data2)
{
	t_input_channel channel = 0;
	t_input_value value = 0;

	Q_ASSERT(self != NULL);
	
	channel = static_cast<t_input_channel> (data1);
	value = t_input_value(SCALE(double(data2),
				    double(0),
				    double(127),
				    double(0),
				    double(KInputValueMax)));

	/* Create, post and forget. Qt takes care of event de-allocation. */
	MIDIInputEvent* event = new MIDIInputEvent(self, channel, value);
	QApplication::postEvent(self, event);
}
 
static void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg,
				DWORD_PTR dwInstance, DWORD_PTR dwParam1,
				DWORD_PTR dwParam2)
{
	MIDIDevice* self = (MIDIDevice*) dwInstance;
	Q_ASSERT(self != NULL);

	if (wMsg == MIM_DATA)
	{
		BYTE status = dwParam1 & 0xFF;
		BYTE data1 = (dwParam1 & 0xFF00) >> 8;
		BYTE data2 = (dwParam1 & 0xFF0000) >> 16;

		if (self->mode() == MIDIDevice::Note
		    && ((status & 0xF0) == MIDI_NOTE_ON ||
			(status & 0xF0) == MIDI_NOTE_OFF)
		    && ((status & 0x0F) == (BYTE) self->midiChannel()))
		{
			postEvent(self, data1, data2);
		}
		else if (self->mode() == MIDIDevice::ControlChange
			 && ((status & 0xF0) == MIDI_CONTROL_CHANGE)
			 && ((status & 0x0F) == (BYTE) self->midiChannel()))
		{
			postEvent(self, data1, data2);
		}
	}
}

bool MIDIDevice::open()
{
	bool result = false;
	MMRESULT res;

	res = midiInOpen(&m_handle, m_id, (DWORD) MidiInProc,
			 (DWORD_PTR) this, CALLBACK_FUNCTION);
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
		midiInStart(m_handle);
		result = true;

		/* Open feedback output */
		openOutput();
	}

	return result;
}

void MIDIDevice::close()
{
	MMRESULT res;
	
	res = midiInClose(m_handle);
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

		/* Close feedback output */
		closeOutput();
	}
}

/*****************************************************************************
 * Device info
 *****************************************************************************/

QString MIDIDevice::infoText()
{
	QString info;
	info += QString("<B>%1</B>").arg(name());

	if (m_isOK == true)
	{
		info += QString("<P>");
		info += QString("Device is working correctly.");
		info += QString("</P>");
		info += QString("<P>");
		info += QString("<B>MIDI Channel: </B>%1<BR>")
				.arg(m_midiChannel + 1);
		info += QString("<B>Mode: </B>%1")
				.arg(modeToString(m_mode));
		info += QString("</P>");
	}
	else
	{
		info += QString("<P>");
		info += QString("MIDI Input not available.");
		info += QString("</P>");
	}
	
	return info;
}

QString MIDIDevice::name()
{
	return m_name;
}

void MIDIDevice::extractName()
{
	MIDIINCAPS inCaps;
	MMRESULT res;

	/* Extract name */
	m_name = QString("MIDI Input %1: ").arg(m_id + 1);
	res = midiInGetDevCaps(m_id, &inCaps, sizeof(MIDIINCAPS));
	if (res == MMSYSERR_BADDEVICEID)
	{
		m_name += QString("Bad device ID");
		qDebug() << "MIDI IN device" << m_id + 1 << "has bad device ID";
	}
	else if (res == MMSYSERR_INVALPARAM)
	{
		m_name += QString("Invalid parameters");
		qDebug() << "Invalid params for MIDI IN device" << m_id + 1;
	}
	else if (res == MMSYSERR_NODRIVER)
	{
		m_name += QString("No driver installed");
		qDebug() << "MIDI IN device" << m_id + 1 << "has no driver";
	}
	else if (res == MMSYSERR_NOMEM)
	{
		m_name += QString("Out of memory");
		qDebug() << "Out of memory while opening MIDI IN" << m_id + 1;
	}
	else
	{
		m_name += QString::fromWCharArray(inCaps.szPname);
		m_isOK = true;
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

/*****************************************************************************
 * Input data
 *****************************************************************************/

void MIDIDevice::customEvent(QEvent* event)
{
	if (event->type() == MIDIInputEvent::eventType)
	{
		MIDIInputEvent* e = static_cast<MIDIInputEvent*> (event);
		emit valueChanged(this, e->m_channel, e->m_value);
		event->accept();
	}
}

/*****************************************************************************
 * Feedback
 *****************************************************************************/

UINT MIDIDevice::feedBackId() const
{
	return m_feedBackId;
}

void MIDIDevice::setFeedBackId(UINT id)
{
	QSettings settings;
	bool reopen = false;
	QString key;

	if (m_feedBackHandle != NULL)
	{
		reopen = true;
		closeOutput();
	}

	m_feedBackId = id;

	/* Save the feedback line number */
	key = QString("/MIDIInput/Inputs/%1/FeedBackLine").arg(m_id);
	settings.setValue(key, id);

	if (m_feedBackId  != UINT_MAX && reopen == true)
		openOutput();
}

void MIDIDevice::openOutput()
{
	if (m_feedBackHandle == NULL && m_feedBackId != UINT_MAX)
	{
		MMRESULT res;
		res = midiOutOpen(&m_feedBackHandle, m_feedBackId, 0, 0, 0);
		if (res != MMSYSERR_NOERROR)
			m_feedBackHandle = NULL;
	}
}

void MIDIDevice::closeOutput()
{
	if (m_feedBackHandle != NULL)
	{
		MMRESULT res;
		res = midiOutClose(m_feedBackHandle);
		if (res == MMSYSERR_NOERROR)
			m_feedBackHandle = NULL;
	}
}

QStringList MIDIDevice::feedBackNames()
{
	QStringList list;
	MIDIOUTCAPS caps;
	MMRESULT res;
	QString name;
	UINT num;
	
	num = midiOutGetNumDevs();
	for (UINT id = 0; id < num; id++)
	{
		res = midiOutGetDevCaps(id, &caps, sizeof(MIDIOUTCAPS));	
		if (res == MMSYSERR_NOERROR)
		{
			name = QString("MIDI Output %1: %2").arg(id + 1)
				.arg(QString::fromWCharArray(caps.szPname));
			list << name;
		}
	}
	
	return list;
}

void MIDIDevice::feedBack(t_input_channel channel, t_input_value value)
{
	/* Attempt to open the same line for feedback output as well */
	if (m_feedBackId != UINT_MAX && m_feedBackHandle == NULL)
		openOutput();

	/* MIDI devices can have only 128 real channels */
	if (channel > 127)
		return;

	char scaled = static_cast <char> (SCALE(double(value),
						double(0),
						double(KInputValueMax),
						double(0),
						double(127)));

	union
	{
		DWORD dwData;
		BYTE bData[4];
	} msg;

	if (mode() == ControlChange)
	{
		/* Use control change numbers as DMX channels and
		   control values as DMX channel values */ 
		msg.bData[0] = MIDI_CONTROL_CHANGE | (BYTE) midiChannel();
		msg.bData[1] = (BYTE) channel;
		msg.bData[2] = (BYTE) scaled;
		msg.bData[3] = 0;

		/* Push the message out */
		midiOutShortMsg(m_feedBackHandle, msg.dwData);
	}
	else
	{
		/* Use note numbers as DMX channels and velocities as
		   DMX channel values. 0 is written as note off */
		if (value == 0)
		{
			msg.bData[0] = MIDI_NOTE_OFF | (BYTE) midiChannel();
			msg.bData[1] = (BYTE) channel;
			msg.bData[2] = 0;
			msg.bData[3] = 0;
		}
		else
		{
			msg.bData[0] = MIDI_NOTE_ON | (BYTE) midiChannel();
			msg.bData[1] = (BYTE) channel;
			msg.bData[2] = (BYTE) scaled;
			msg.bData[3] = 0;
		}

		/* Push the message out */
		midiOutShortMsg(m_feedBackHandle, msg.dwData);
	}
}
