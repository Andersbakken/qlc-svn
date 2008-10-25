/*
  Q Light Controller
  unix-mididevice.cpp

  Copyright (c) Heikki Junnila
		Stefan Krumm

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
#include <QObject>
#include <QString>
#include <QDebug>
#include <QFile>

#include "unix-mididevice.h"
#include "unix-midiinput.h"
#include "midiinputevent.h"

MIDIDevice::MIDIDevice(MIDIInput* parent, t_input line, const QString& path)
	: QObject(parent)
{
	Q_ASSERT(path.length() > 0);
	m_file.setFileName(path);
	m_line = line;
	m_name = QString("MIDI Input %1").arg(line + 1);
}

MIDIDevice::~MIDIDevice()
{
	static_cast<MIDIInput*> (parent())->removePollDevice(this);
	close();
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

bool MIDIDevice::open()
{
	bool result = false;

	result = m_file.open(QIODevice::Unbuffered | QIODevice::ReadWrite);
	if (result == false)
	{
		result = m_file.open(QIODevice::Unbuffered |
				     QIODevice::ReadOnly);
		if (result == false)
		{
			qWarning() << "Unable to open" << m_file.fileName()
				   << m_file.errorString();
		}
		else
		{
			qDebug() << "Opened" << m_file.fileName()
				 << "in read only mode";		
		}
	}

	return result;
}

void MIDIDevice::close()
{
	m_file.close();
}

QString MIDIDevice::path() const
{
	return m_file.fileName();
}

int MIDIDevice::handle() const
{
	return m_file.handle();
}

bool MIDIDevice::readEvent()
{
	int r;
	unsigned char buf[10] = { 255, 255, 255, 255, 255,
				  255, 255, 255, 255, 255 };

	r = ::read(handle(), buf, 1);
	if (r == 0)
	{
		/* Unable to read from the MIDI device, although something
		   should be there. Deem it dead. */
		::perror("read");
		return false;
	}
	else if ((buf[0] & 176) == 176)
	{
		MIDIInputEvent* e;
		t_input_channel channel;
		t_input_value value;

		r = ::read(handle(), &buf[1], 2);
		if (r == 2)
		{
			/* channel = buf[0] ^ 176; */
			channel = buf[1];
			value = buf[2];
			
			/* Post the event to the global event loop so that we
			   can switch context away from the poller thread and
			   into the main application thread. This is caught in
			   MIDIInput::customEvent(). */
			e = new MIDIInputEvent(this, m_line, channel, value);
			QApplication::postEvent(parent(), e);
		}
	}

	/* Return true, because the event might have just been something that
	   we don't recognise/care about. */
	return true;
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
	info += m_file.fileName();
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

void MIDIDevice::feedBack(t_input_channel /*channel*/,
			      t_input_value /*value*/)
{
}

