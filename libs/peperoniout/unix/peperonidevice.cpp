/*
  Q Light Controller
  peperonidevice.cpp

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

#include <sys/ioctl.h>
#include <QDebug>
#include <QFile>

#include "peperonidevice.h"
#include "usbdmx.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

PeperoniDevice::PeperoniDevice(QObject* parent, const QString& path)
	: QObject(parent)
{
	Q_ASSERT(path.isEmpty() == false);

	m_path = path;
	m_file.setFileName(m_path);

	extractName();
}

PeperoniDevice::~PeperoniDevice()
{
	close();
}

void PeperoniDevice::extractName()
{
	bool needToClose = false;

	if (m_file.isOpen() == false)
	{
		needToClose = true;
		open();
	}

	if (m_file.isOpen() == false)
	{
		needToClose = false;
		m_name = QString("Nothing");
	}
	else
	{
		int type = ::ioctl(m_file.handle(), DMX_TYPE_GET, NULL);
		switch (type)
		{
		case XSWITCH:
			m_name = QString("X-Switch");
			break;
		case RODIN1:
			m_name = QString("Rodin 1");
			break;
		case RODIN2:
			m_name = QString("Rodin 2");
			break;
		case USBDMX21:
			m_name = QString("USBDMX21");
			break;
		default:
			m_name = QString("Unknown");
			break;
		}
	}

	if (needToClose == true)
		close();
}

/****************************************************************************
 * Properties
 ****************************************************************************/

QString PeperoniDevice::name() const
{
	return m_name;
}

QString PeperoniDevice::path() const
{
	return m_path;
}

/****************************************************************************
 * Open & close
 ****************************************************************************/

void PeperoniDevice::open()
{
	m_file.unsetError();
	if (m_file.open(QIODevice::WriteOnly | QIODevice::Unbuffered) == true)
	{
		/* Set writing mode */
		::ioctl(m_file.handle(), DMX_MEM_MAP_SET, DMX_TX_MEM);

		/* Set DMX startcode */
		::ioctl(m_file.handle(), DMX_TX_STARTCODE_SET, 0);
	}
	else
	{
		qWarning() << QString("Unable to open %1: %2").arg(m_name)
				.arg(m_file.errorString());
	}
}

void PeperoniDevice::close()
{
	m_file.unsetError();
	m_file.close();

	if (m_file.error() != QFile::NoError)
		qWarning() << QString("Unable to close %1: %2").arg(m_name)
				.arg(m_file.errorString());
}

/****************************************************************************
 * Write
 ****************************************************************************/

void PeperoniDevice::writeRange(t_value* values, t_channel num)
{
	Q_UNUSED(num);

	m_file.seek(0);
	if (m_file.write((const char*) values, 512) == -1)
		qWarning() << "writeRange error:" << m_file.errorString();
}
