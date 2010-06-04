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
#define USE_NANOSLEEP
#ifdef USE_NANOSLEEP
#	include <time.h>
#else
#	include <unistd.h>
#endif

#include "enttecdmxusbopen.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

EnttecDMXUSBOpen::EnttecDMXUSBOpen(QObject* parent, const QString& name,
				   const QString& serial)
	: QThread(parent)
	, m_name(name)
	, m_serial(serial)
	, m_running(false)
	, m_universe(QByteArray(513, 0))
{
	ftdi_init(&m_context);
}

EnttecDMXUSBOpen::~EnttecDMXUSBOpen()
{
	close();
	ftdi_deinit(&m_context);
}

/****************************************************************************
 * Open & Close
 ****************************************************************************/

bool EnttecDMXUSBOpen::open()
{
	if (isOpen() == false)
	{
		if (ftdi_usb_open_desc(&m_context, EnttecDMXUSBWidget::VID,
						   EnttecDMXUSBWidget::PID,
						   name().toAscii(),
						   serial().toAscii()) < 0)
		{
			qWarning() << "Unable to open" << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
			return false;
		}

		if (ftdi_usb_reset(&m_context) < 0)
		{
			qWarning() << "Unable to reset" << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
			return close();
		}

		if (ftdi_set_line_property(&m_context, BITS_8, STOP_BIT_2, NONE) < 0)
		{
			qWarning() << "Unable to set 8N2 serial properties to"
				   << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
			return close();
		}

		if (ftdi_set_baudrate(&m_context, 250000) < 0)
		{
			qWarning() << "Unable to set 250kbps baudrate for"
				   << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
			return close();
		}

		if (ftdi_setrts(&m_context, 0) < 0)
		{
			qWarning() << "Unable to set RTS line to 0 for"
				   << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
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

		if (ftdi_usb_close(&m_context) < 0)
		{
			qWarning() << "Unable to close" << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
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
	if (m_context.usb_dev == NULL)
		return false;
	else
		return true;
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
	m_mutex.lock();
	m_universe = m_universe.replace(1, universe.size(), universe);
	m_mutex.unlock();
	return true;
}

/****************************************************************************
 * Thread
 ****************************************************************************/

void EnttecDMXUSBOpen::sleep(quint32 ms)
{
#ifdef USE_NANOSLEEP
	struct timespec tv;
	tv.tv_sec = 0;
	tv.tv_nsec = (ms * 1000);
	nanosleep(&tv, NULL);
#else
	usleep(ms);
#endif
}

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
	ftdi_usb_purge_buffers(&m_context);

	m_running = true;
	while (m_running == true)
	{
		if (isOpen() == false)
			continue;

		if (ftdi_set_line_property2(&m_context, BITS_8, STOP_BIT_2,
					    NONE, BREAK_ON) < 0)
		{
			qWarning() << "Unable to toggle BREAK_ON for"
				   << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
			goto framesleep;
		}

		sleep(88);

		if (ftdi_set_line_property2(&m_context, BITS_8, STOP_BIT_2,
					    NONE, BREAK_OFF) < 0)
		{
			qWarning() << "Unable to toggle BREAK_OFF for"
				   << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
			goto framesleep;
		}

		sleep(8);

		m_mutex.lock();
		if (ftdi_write_data(&m_context, (unsigned char*) m_universe.data(),
				    m_universe.size()) < 0)
		{
			qWarning() << "Unable to write DMX data to"
				   << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
		}
		m_mutex.unlock();

framesleep:
		sleep(22754);
	}
}
