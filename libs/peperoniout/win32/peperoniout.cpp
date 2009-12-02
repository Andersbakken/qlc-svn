/*
  Q Light Controller
  peperoniout.cpp

  Copyright (c)	Heikki Junnila

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

#include <QMessageBox>
#include <QStringList>
#include <windows.h>
#include <QDebug>
#include <QString>

#include "peperoni/usbdmx-dynamic.h"
#include "peperonidevice.h"
#include "peperoniout.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void PeperoniOut::init()
{
	/* Load usbdmx.dll */
	m_usbdmx = usbdmx_init();
	if (m_usbdmx == NULL)
	{
		qWarning() << "Loading USBDMX.DLL failed.";
	}
	else if (USBDMX_DLL_VERSION_CHECK(m_usbdmx) == FALSE)
	{
		/* verify USBDMX dll version */
		qWarning() << "USBDMX.DLL version does not match. Abort.";
		qWarning() << "Found" << m_usbdmx->version() << "but expected"
			   << USBDMX_DLL_VERSION;
	}
	else
	{
		qDebug() << "Using USBDMX.DLL version" << m_usbdmx->version();
		rescanDevices();
	}
}

void PeperoniOut::rescanDevices()
{
	USHORT id = 0;

	if (m_usbdmx == NULL)
		return;

	for (id = 0; id < 32; id++)
	{
		HANDLE handle = NULL;
		if (m_usbdmx->open(id, &handle) == TRUE)
		{
			/* We don't need the handle now. */
			m_usbdmx->close(handle);

			if (id >= m_devices.size())
			{
				PeperoniDevice* device;

				/* Device was opened successfully and it's
				   a new device. Append it to our list. */
				device = new PeperoniDevice(this, m_usbdmx, id);
				m_devices.append(device);
			}
			else
			{
				/* We already have a device with this id. Try
				   the next one. */
			}
		}
		else
		{
			/* This device ID doesn't exist and neither does any
			   consecutive id, so we can stop looking. */
			break;
		}
	}

	/* Remove those devices that aren't present. I.e. if our search
	   stopped into an ID that is equal to or less than the current number
	   of devices, one or more devices are no longer present. */
	while (id < m_devices.size())
		delete m_devices.takeLast();

	/* Because all devices have just plain and dull IDs, we can't know,
	   whether the user removed one XSwitch and plugged in a Rodin1,
	   that ends up getting the same ID. Therefore, force all known devices
	   to reload their info again. */
	QListIterator <PeperoniDevice*> it(m_devices);
	while (it.hasNext() == true)
		it.next()->rehash();
}

/*****************************************************************************
 * Plugin open/close
 *****************************************************************************/

void PeperoniOut::open(t_output output)
{
	if (m_usbdmx == NULL)
		return;

	if (output < m_devices.size())
		m_devices.at(output)->open();
}

void PeperoniOut::close(t_output output)
{
	if (m_usbdmx == NULL)
		return;

	if (output < m_devices.size())
		m_devices.at(output)->close();
}

QStringList PeperoniOut::outputs()
{
	QStringList list;
	int i = 1;

	QListIterator <PeperoniDevice*> it(m_devices);
	while (it.hasNext() == true)
		list << QString("%1: %2").arg(i++).arg(it.next()->name());

	return list;
}

/*****************************************************************************
 * Name
 *****************************************************************************/

QString PeperoniOut::name()
{
	return QString("Peperoni Output");
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

void PeperoniOut::configure()
{
	int r = QMessageBox::question(NULL, name(),
				tr("Do you wish to re-scan your hardware?"),
				QMessageBox::Yes, QMessageBox::No);
	if (r == QMessageBox::Yes)
		rescanDevices();
}

/*****************************************************************************
 * Plugin status
 *****************************************************************************/

QString PeperoniOut::infoText(t_output output)
{
	QString str;

	str += QString("<HTML>");
	str += QString("<HEAD>");
	str += QString("<TITLE>%1</TITLE>").arg(name());
	str += QString("</HEAD>");
	str += QString("<BODY>");

	if (m_usbdmx == NULL)
	{
		str += QString("<H3>%1</H3>").arg(name());
		str += QString("<P>");
		str += QString("The shared library usbdmx.dll could not be ");
		str += QString("found or is too old to be used with QLC. ");
		str += QString("You can request a driver package from ");
		str += QString("<address>www.peperoni-light.de</address> ");
		str += QString("for your Peperoni lighting products.");
		str += QString("</P>");
	}
	else if (output == KOutputInvalid)
	{
		str += QString("<H3>%1</H3>").arg(name());
		str += QString("<P>");
		str += QString("This plugin provides DMX output support for ");
		str += QString("devices manufactured by Peperoni Light: ");
		str += QString("Rodin 1, Rodin 2, Rodin T, X-Switch and ");
		str += QString("USBDMX21. See ");
		str += QString("<a href=\"http://www.peperoni-light.de\">");
		str += QString("http://www.peperoni-light.de</a> for more ");
		str += QString("information. ");
		str += QString("</P>");
	}
	else if (output < m_devices.size())
	{
		str += m_devices.at(output)->infoText();
	}

	str += QString("</BODY>");
	str += QString("</HTML>");

	return str;
}

/*****************************************************************************
 * Write
 *****************************************************************************/

void PeperoniOut::outputDMX(t_output output, const QByteArray& universe)
{
	if (output < m_devices.size())
		m_devices.at(output)->outputDMX(universe);
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(peperoniout, PeperoniOut)

