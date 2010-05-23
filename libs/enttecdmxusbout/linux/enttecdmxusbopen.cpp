/*
  Q Light Controller
  enttecdmxusbopen.cpp

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

#include <QDebug>
#include "enttecdmxusbopen.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

EnttecDMXUSBOpen::EnttecDMXUSBOpen(QObject* parent, Ftdi::Context context)
	: QThread(parent)
	, m_context(context)
	, m_serial(QString::fromStdString(context.serial()))
	, m_name(QString::fromStdString(context.description()))
	, m_running(false)
	, m_universe(QByteArray(512, 0))
{
}

EnttecDMXUSBOpen::~EnttecDMXUSBOpen()
{
	close();
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool EnttecDMXUSBOpen::open()
{
	if (isOpen() == false)
	{
		int r;

		r = m_context.open();
		if (r < 0)
		{
			qWarning() << "Unable to open" << uniqueName()
				   << ":" << r;
			return false;
		}

		r = m_context.reset();
		if (r < 0)
		{
			qWarning() << "Unable to reset" << uniqueName()
				   << ":" << r;
			return close();
		}

		r = m_context.set_line_property(BITS_8, STOP_BIT_2, NONE);
		if (r < 0)
		{
			qWarning() << "Unable to set 8N2 serial properties to"
				   << uniqueName() << ":" << r;
			return close();
		}

		r = m_context.set_baud_rate(250000);
		if (r < 0)
		{
			qWarning() << "Unable to set 250kbps baudrate for"
				   << uniqueName() << ":" << r;
			return close();
		}

		r = m_context.set_rts(false);
		if (r < 0)
		{
			qWarning() << "Unable to set RTS line to 0 for"
				   << uniqueName() << ":" << r;
			return close();
		}

		if (isRunning() == false)
			start();

		return true;
	}
	else
	{
		/* Already open */
		return true;
	}
}

bool EnttecDMXUSBOpen::close()
{
	if (isOpen() == true)
	{
		/* Stop the writer thread */
		if (isRunning() == true)
			stop();

		int r = m_context.close();
		if (r < 0)
		{
			qWarning() << "Unable to close" << uniqueName()
				   << ":" << r;
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		return true;
	}
}

bool EnttecDMXUSBOpen::isOpen()
{
	return m_context.is_open();
}

/****************************************************************************
 * Name & Serial
 ****************************************************************************/

QString EnttecDMXUSBOpen::name() const
{
	return m_name;
}

QString EnttecDMXUSBOpen::serial() const
{
	return m_serial;
}

QString EnttecDMXUSBOpen::uniqueName() const
{
	return QString("%1 (S/N: %2)").arg(name()).arg(serial());
}

/****************************************************************************
 * DMX Operations
 ****************************************************************************/

bool EnttecDMXUSBOpen::sendDMX(const QByteArray& universe)
{
	Q_ASSERT(universe.size() == 512);

	/* Can't send DMX unless the widget is open */
	if (isOpen() == false || isRunning() == false)
	{
		return false;
	}
	else
	{
		m_universe = universe;
		return true;
	}
}

/****************************************************************************
 * Thread
 ****************************************************************************/

void EnttecDMXUSBOpen::stop()
{
	if (isRunning() == true)
	{
		m_running = false;
		wait();
	}
}

void EnttecDMXUSBOpen::run()
{
	int r = 0;
	unsigned char startByte = 0;
	struct timespec tv;

	m_context.flush(Ftdi::Context::Output);

	m_running = true;
	while (m_running == true)
	{
		if (isOpen() == false)
			open();

		r = m_context.set_line_property(BITS_8, STOP_BIT_2, NONE, BREAK_ON);
		if (r < 0)
		{
			qWarning() << "Unable to toggle BREAK_ON for"
				   << uniqueName() << ":" << r;
			goto framesleep;
		}

		tv.tv_sec = 0;
		tv.tv_nsec = 88000;
		nanosleep(&tv, NULL);

		r = m_context.set_line_property(BITS_8, STOP_BIT_2, NONE, BREAK_OFF);
		if (r < 0)
		{
			qWarning() << "Unable to toggle BREAK_OFF for"
				   << uniqueName() << ":" << r;
			goto framesleep;
		}

		tv.tv_sec = 0;
		tv.tv_nsec = 8000;
		nanosleep(&tv, NULL);

		r = m_context.write(&startByte, 1);
		if (r < 0)
		{
			qWarning() << "Unable to write start byte to"
				   << uniqueName() << ":" << r;
			goto framesleep;
		}

		r = m_context.write((unsigned char*) m_universe.data(),
				    m_universe.size());
		if (r < 0)
		{
			qWarning() << "Unable to write DMX data to"
				   << uniqueName() << ":" << r;
			goto framesleep;
		}

framesleep:
		tv.tv_sec = 0;
		tv.tv_nsec = 22754000;
		nanosleep(&tv, NULL);
	}
}

