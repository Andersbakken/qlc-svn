/*
  Q Light Controller
  enttecdmxusb.cpp

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

#include <QStringList>
#include <QMessageBox>
#include <QDebug>

#include "enttecdmxusbwidget.h"
#include "enttecdmxusbopen.h"
#include "enttecdmxusbpro.h"
#include "enttecdmxusbout.h"

/****************************************************************************
 * Initialization
 ****************************************************************************/

void EnttecDMXUSBOut::init()
{
	rescanWidgets();
}

void EnttecDMXUSBOut::open(t_output output)
{
	if (output < m_widgets.size())
		m_widgets.at(output)->open();
}

void EnttecDMXUSBOut::close(t_output output)
{
	if (output < m_widgets.size())
		m_widgets.at(output)->close();
}

/****************************************************************************
 * Devices (ENTTEC calls them "widgets" and so shall we)
 ****************************************************************************/

bool EnttecDMXUSBOut::rescanWidgets()
{
	FT_DEVICE_LIST_INFO_NODE* devInfo = NULL;
	FT_STATUS status = FT_OK;
	DWORD num = 0;

	while (m_widgets.isEmpty() == false)
		delete m_widgets.takeFirst();

	/* Find out the number of FTDI devices present */
	status = FT_CreateDeviceInfoList(&num);
	if (status != FT_OK)
	{
		qWarning() << "CreateDeviceInfoList failed:" << status;
		return false;
	}
	else if (num <= 0)
	{
		qDebug() << "No devices found";
		return true;
	}

	// Allocate storage for list based on numDevices
	devInfo = new FT_DEVICE_LIST_INFO_NODE[num];

	// Get the device information list
	if (FT_GetDeviceInfoList(devInfo, &num) == FT_OK)
	{
		for (DWORD i = 0; i < num; i++)
		{
			/* Get the device description field so that it can be
			   used to determine the device type (Pro/Open) */
			QString desc(devInfo[i].Description);

			if (desc.contains("pro", Qt::CaseInsensitive) == true)
			{
				/* This is a DMX USB Pro widget */
				EnttecDMXUSBPro* w;
				w = new EnttecDMXUSBPro(this, devInfo[i], i);
				m_widgets.append(w);
			}
			else
			{
				EnttecDMXUSBOpen* w;
				w = new EnttecDMXUSBOpen(this, devInfo[i]);
				Q_ASSERT(w != NULL);
				w->setSerial(QString("%1").arg(i));
				m_widgets.append(w);
			}
		}
	}

	delete [] devInfo;

	return true;
}

QStringList EnttecDMXUSBOut::outputs()
{
	QStringList list;
	int i = 1;

	QListIterator <EnttecDMXUSBWidget*> it(m_widgets);
	while (it.hasNext() == true)
		list << QString("%1: %2").arg(i++).arg(it.next()->uniqueName());
	return list;
}

/****************************************************************************
 * Name
 ****************************************************************************/

QString EnttecDMXUSBOut::name()
{
	return QString("Enttec DMX USB Output");
}

/****************************************************************************
 * Configuration
 ****************************************************************************/

void EnttecDMXUSBOut::configure()
{
	int r = QMessageBox::question(NULL, name(),
				tr("Do you wish to re-scan your hardware?"),
				QMessageBox::Yes, QMessageBox::No);
	if (r == QMessageBox::Yes)
		rescanWidgets();
}

/****************************************************************************
 * Plugin status
 ****************************************************************************/

QString EnttecDMXUSBOut::infoText(t_output output)
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
		str += QString("devices manufactured by ENTTEC: ");
		str += QString("DMX USB Pro and Open DMX USB. See ");
		str += QString("<a href=\"http://www.enttec.com\">");
		str += QString("http://www.enttec.com</a> for more ");
		str += QString("information. ");
		str += QString("</P>");

#ifdef WIN32
		if (m_widgets.size() == 0)
		{
			str += QString("<P>");
			str += QString("<B>No devices available</B>. Make ");
			str += QString("sure you have your Enttec hardware ");
			str += QString("plugged in and the <I>D2XX</I> ");
			str += QString("drivers installed. Then click the ");
			str += QString("<B>configure</B> button. Note that ");
			str += QString("the VCP driver is not supported.");
			str += QString("</P>");
		}
#endif
	}
	else if (output < m_widgets.size())
	{
		str += QString("<H3>%1</H3>").arg(outputs()[output]);
		str += QString("<P>");
		str += QString("Device is operating correctly.");
		str += QString("</P>");
	}

	str += QString("</BODY>");
	str += QString("</HTML>");

	return str;
}

/*****************************************************************************
 * Value Read/Write
 *****************************************************************************/

void EnttecDMXUSBOut::writeChannel(t_output output, t_channel channel,
				   t_value value)
{
	Q_UNUSED(output);
	Q_UNUSED(channel);
	Q_UNUSED(value);
}

void EnttecDMXUSBOut::writeRange(t_output output, t_channel address,
				 t_value* values, t_channel num)
{
	Q_UNUSED(address);

	if (output < m_widgets.size())
	{
		QByteArray ba((const char*)values, num);
		m_widgets.at(output)->sendDMX(ba);
	}
}

void EnttecDMXUSBOut::readChannel(t_output output, t_channel channel,
				  t_value* value)
{
	Q_UNUSED(output);
	Q_UNUSED(channel);
	Q_UNUSED(value);
}

void EnttecDMXUSBOut::readRange(t_output output, t_channel address,
				t_value* values, t_channel num)
{
	Q_UNUSED(output);
	Q_UNUSED(address);
	Q_UNUSED(values);
	Q_UNUSED(num);
}


/****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(enttecdmxusbout, EnttecDMXUSBOut)

