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
#include <QString>
#include <QDebug>
#include <usb.h>

#include "peperonidevice.h"
#include "peperoniout.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

PeperoniOut::~PeperoniOut()
{
}

void PeperoniOut::init()
{
	usb_init();
	rescanDevices();
}

void PeperoniOut::open(t_output output)
{
	if (output < m_devices.size())
		m_devices.at(output)->open();
}

void PeperoniOut::close(t_output output)
{
	if (output < m_devices.size())
		m_devices.at(output)->close();
}

/*****************************************************************************
 * Devices
 *****************************************************************************/

void PeperoniOut::rescanDevices()
{
	struct usb_device* dev;
	struct usb_bus* bus;

	/* Treat all devices as dead first, until we find them again. Those
	   that aren't found, get destroyed at the end of this function. */
	QList <PeperoniDevice*> destroyList(m_devices);

	usb_find_busses();
	usb_find_devices();

	/* Iterate thru all buses */
	for (bus = usb_get_busses(); bus != NULL; bus = bus->next)
	{
		/* Iterate thru all devices in each bus */
		for (dev = bus->devices; dev != NULL; dev = dev->next)
		{
			PeperoniDevice* pepdev = device(dev);
			if (pepdev != NULL)
			{
				/* We already have this device and it's still
				   there. Remove from the destroy list and
				   continue iterating. */
				destroyList.removeAll(pepdev);
				continue;
			}
			else if (PeperoniDevice::isPeperoniDevice(dev) == true)
			{
				/* This is a new device. Create and append. */
				pepdev = new PeperoniDevice(this, dev);
				m_devices.append(pepdev);
			}
		}
	}

	/* Destroy those devices that were no longer found. */
	while (destroyList.isEmpty() == false)
	{
		PeperoniDevice* pepdev = destroyList.takeFirst();
		m_devices.removeAll(pepdev);
		delete pepdev;
	}
}

PeperoniDevice* PeperoniOut::device(struct usb_device* usbdev)
{
	QListIterator <PeperoniDevice*> it(m_devices);
	while (it.hasNext() == true)
	{
		PeperoniDevice* dev = it.next();
		if (dev->device() == usbdev)
			return dev;
	}

	return NULL;
}

/*****************************************************************************
 * Outputs
 *****************************************************************************/

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

	if (output == KOutputInvalid)
	{
		str += QString("<H3>%1</H3>").arg(name());
		str += QString("<P>");
		str += QString("This plugin provides DMX output support for ");
		str += QString("Peperoni DMX devices. See ");
		str += QString("<a href=\"http://www.peperoni-light.de\">");
		str += QString("http://www.peperoni-light.de</a> for more ");
		str += QString("information.");
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
 * Value Read/Write
 *****************************************************************************/

void PeperoniOut::writeChannel(t_output output, t_channel channel,
			       t_value value)
{
	Q_UNUSED(output);
	Q_UNUSED(channel);
	Q_UNUSED(value);
}

void PeperoniOut::writeRange(t_output output, t_channel address,
			     t_value* values, t_channel num)
{
	Q_UNUSED(address);

	if (output < m_devices.size())
		m_devices.at(output)->writeRange((char*) values, num);
}

void PeperoniOut::readChannel(t_output output, t_channel channel,
			      t_value* value)
{
	Q_UNUSED(output);
	Q_UNUSED(channel);
	Q_UNUSED(value);
}

void PeperoniOut::readRange(t_output output, t_channel address, t_value* values,
			  t_channel num)
{
	Q_UNUSED(output);
	Q_UNUSED(address);
	Q_UNUSED(values);
	Q_UNUSED(num);
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(peperoniout, PeperoniOut)
