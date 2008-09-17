/*
  Q Light Controller
  win32-mididevice.cpp

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
#include <QObject>
#include <QString>
#include <QDebug>

#include "win32-mididevice.h"
#include "win32-midiinput.h"
#include "midiinputevent.h"

MIDIDevice::MIDIDevice(MIDIInput* parent, UINT id) : QObject(parent)
{
	MMRESULT res;
	MIDIINCAPS caps;
	
	res = midiInGetDevCaps(id, &caps, sizeof(MIDIINCAPS));

	m_id = id;
	m_name = QString("MIDI Input %1").arg(id + 1);
	m_handle = NULL;
	
	if (res == MMSYSERR_BADDEVICEID)
	{
		m_name += QString(": Bad device ID");
		qDebug() << "MIDI IN device" << id + 1 << " has bad device ID";
	}
	else if (res == MMSYSERR_INVALPARAM)
	{
		m_name += QString(": Invalid parameters");
		qDebug() << "Invalid params for MIDI IN device" << id + 1;
	}
	else if (res == MMSYSERR_NODRIVER)
	{
		m_name += QString(": No driver installed");
		qDebug() << "MIDI IN device" << id + 1 << "has no driver";
	}
	else if (res == MMSYSERR_NOMEM)
	{
		m_name += QString(": Out of memory");
		qDebug() << "Out of memory while opening MIDI IN" << id + 1;
	}
	else
	{
		QString str = QString::fromWCharArray(caps.szPname);
		m_name += QString(": %2").arg(str);

		qDebug() << m_name;
	}
}

MIDIDevice::~MIDIDevice()
{
	close();
}

/*****************************************************************************
 * File operations
 *****************************************************************************/
 
static void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD_PTR dwInstance, 
				DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
	MIDIDevice* self;
	t_input_channel channel = 0;
	t_input_value value = 0;

	self = (MIDIDevice*) dwInstance;
	Q_ASSERT(self != NULL);

	if (wMsg == MIM_DATA)
	{
		BYTE status = dwParam1 & 0xFF;
		BYTE data1 = (dwParam1 & 0xFF00) >> 8;
		BYTE data2 = (dwParam1 & 0xFF0000) >> 16;
		MIDIInputEvent* event;

		channel = static_cast<t_input_channel> (data1);
		value = static_cast<t_input_value> (data2);

		event = new MIDIInputEvent(self, self->line(), channel, value,
					   true);
		QApplication::postEvent(self, event);
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
	}
}

/*****************************************************************************
 * Device info
 *****************************************************************************/

QString MIDIDevice::infoText()
{
	QString info;
	QString str;

	info += QString("<TR>");

	/* File name */
	info += QString("<TD>");
	info += QString("%1").arg(m_id);
	info += QString("</TD>");

	/* Name */
	info += QString("<TD>");
	info += m_name;
	info += QString("</TD>");

	/* Channels */
	info += QString("<TD ALIGN=\"CENTER\">");
	info += QString("N/A");
	info += QString("</TD>");

	info += QString("</TR>");

	return info;
}

QString MIDIDevice::name()
{
	return m_name;
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
 
void MIDIDevice::feedBack(t_input_channel /*channel*/,
			      t_input_value /*value*/)
{
}

