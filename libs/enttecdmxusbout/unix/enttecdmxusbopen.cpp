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
	, m_universe(QByteArray(512, 0))
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
		int r;

		r = ftdi_usb_open_desc(&m_context, EnttecDMXUSBWidget::VID,
						   EnttecDMXUSBWidget::PID,
						   name().toAscii(),
						   serial().toAscii());
		if (r < 0)
		{
			qWarning() << "Unable to open" << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
			return false;
		}

		r = ftdi_usb_reset(&m_context);
		if (r < 0)
		{
			qWarning() << "Unable to reset" << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
			return close();
		}

		r = ftdi_set_line_property(&m_context, BITS_8, STOP_BIT_2, NONE);
		if (r < 0)
		{
			qWarning() << "Unable to set 8N2 serial properties to"
				   << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
			return close();
		}

		r = ftdi_set_baudrate(&m_context, 250000);
		if (r < 0)
		{
			qWarning() << "Unable to set 250kbps baudrate for"
				   << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
			return close();
		}

		r = ftdi_setrts(&m_context, 0);
		if (r < 0)
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

		int r = ftdi_usb_close(&m_context);
		if (r < 0)
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
	int r = 0;
	unsigned char startByte = 0;

	ftdi_usb_purge_buffers(&m_context);

	m_running = true;
	while (m_running == true)
	{
		if (isOpen() == false)
			continue;

		r = ftdi_set_line_property2(&m_context, BITS_8, STOP_BIT_2, NONE, BREAK_ON);
		if (r < 0)
		{
			qWarning() << "Unable to toggle BREAK_ON for"
				   << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
			goto framesleep;
		}

		sleep(88);

		r = ftdi_set_line_property2(&m_context, BITS_8, STOP_BIT_2, NONE, BREAK_OFF);
		if (r < 0)
		{
			qWarning() << "Unable to toggle BREAK_OFF for"
				   << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
			goto framesleep;
		}

		sleep(8);

		r = ftdi_write_data(&m_context, &startByte, 1);
		if (r < 0)
		{
			qWarning() << "Unable to write start byte to"
				   << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
			goto framesleep;
		}

		r = ftdi_write_data(&m_context, (unsigned char*) m_universe.data(),
				    m_universe.size());
		if (r < 0)
		{
			qWarning() << "Unable to write DMX data to"
				   << uniqueName()
				   << ":" << ftdi_get_error_string(&m_context);
			goto framesleep;
		}

framesleep:
		sleep(22754);
	}
}
