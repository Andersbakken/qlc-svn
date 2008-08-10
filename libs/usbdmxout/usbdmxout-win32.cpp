/*
  Q Light Controller
  usbdmxout-win32.cpp
  
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

#include <windows.h>

#include <QApplication>
#include <QMessageBox>
#include <QPalette>
#include <QDebug>
#include <QString>
#include <QColor>

#include "configureusbdmxout.h"
#include "usbdmxout-win32.h"
#include "usbdmx-dynamic.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void USBDMXOut::init()
{
	for (int i = 0; i < MAX_USBDMX_DEVICES; i++)
	{
		/* Initialize device handles */
		m_devices[i] = 0;

		/* Initialize value buffers */
		for (t_channel ch = 0; ch < 512; ch++)
			m_values[i][ch] = 0;

	}
	
	/* Nobody is using the device yet */
	m_refCount = 0;
	
	/* Load usbdmx.dll */
	usbdmx = usbdmx_init();
	if (usbdmx == NULL)
		qWarning() << "Loading USBDMX.DLL failed.";

	/* verify USBDMX dll version */
	if (USBDMX_DLL_VERSION_CHECK(usbdmx) == FALSE)
	{
		qWarning() << "USBDMX.DLL version does not match. Abort.";
		qWarning() << "Found" << usbdmx->version << "but expected"
			   << USBDMX_DLL_VERSION;
	}
	else
	{
		qDebug() << "Using USBDMX.DLL version" << usbdmx->version();
	}
}

/*****************************************************************************
 * Plugin open/close
 *****************************************************************************/

int USBDMXOut::open()
{
	if (usbdmx == NULL)
		return -1;

	/* Count the number of times open() has been called so that the devices
	   are opened only once. This is basically reference counting. */
	m_refCount++;
	if (m_refCount > 1)
		return 0;

	for (int i = 0; i < MAX_USBDMX_DEVICES; i++)
	{
		HANDLE handle;
		
		/* Open the device */
		if (usbdmx->open(i, &handle) == TRUE)
		{
			USHORT version;
			
			m_devices[i] = handle;
			
			usbdmx->device_version(m_devices[i], &version);
			if (usbdmx->is_xswitch(m_devices[i]))
				m_names[i] = QString("X-Switch V%1")
						.arg(version);
			else if (usbdmx->is_rodin1(m_devices[i]))
				m_names[i] = QString("Rodin1 V%1")
						.arg(version);
			else if (usbdmx->is_rodin2(m_devices[i]))
				m_names[i] = QString("Rodin2 V%1")
						.arg(version);
			else if (usbdmx->is_rodint(m_devices[i]))
				m_names[i] = QString("RodinT V%1")
						.arg(version);
			else if (usbdmx->is_usbdmx21(m_devices[i]))
				m_names[i] = QString("USBDMX21 V%1")
						.arg(version);

		}
		else
		{
			m_devices[i] = 0;
		}
	}

	return 0;
}

int USBDMXOut::close()
{
	if (usbdmx == NULL)
		return -1;

	/* Count the number of times close() has been called so that the devices
	   are closed only after the last user closes this plugin. This is
	   basically reference counting. */
	m_refCount--;
	if (m_refCount > 0)
		return 0;
	Q_ASSERT(m_refCount == 0);

	for (int i = 0; i < MAX_USBDMX_DEVICES; i++)
	{
		/* Close the interface if it exists */
		if (m_devices[i] != 0)
			usbdmx->close(m_devices[i]);
		m_devices[i] = 0;
	}

	return 0;
}

int USBDMXOut::outputs()
{
	return MAX_USBDMX_DEVICES;
}

/*****************************************************************************
 * Name
 *****************************************************************************/

QString USBDMXOut::name()
{
	return QString("USB DMX Output");
}

/*****************************************************************************
 * Configuration
 *****************************************************************************/

int USBDMXOut::configure()
{
	int r;

	open();

	ConfigureUSBDMXOut conf(NULL, this);
	r = conf.exec();

	close();

	return r;
}

/*****************************************************************************
 * Plugin status
 *****************************************************************************/

QString USBDMXOut::infoText()
{
	QString info;
	QString s;

	/* HTML page Title */
	info += QString("<HTML>");
	info += QString("<HEAD>");
	info += QString("<TITLE>Plugin Info</TITLE>");
	info += QString("</HEAD>");
	info += QString("<BODY>");

	/* Plugin title */
	info += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
	info += QString("<TR>");
	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().color(QPalette::Highlight).name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().color(QPalette::HighlightedText).name();
	info += QString("\" SIZE=\"5\">");
	info += name();
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");
	info += QString("</TABLE>");

	/*********************************************************************
	 * Outputs
	 *********************************************************************/

	/* Title */
	info += QString("<TABLE COLS=\"3\" WIDTH=\"100%\">");
	info += QString("<TR>");
	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().color(QPalette::Highlight).name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().color(QPalette::HighlightedText).name();
	info += QString("\">");
	info += QString("Output");
	info += QString("</FONT>");
	info += QString("</TD>");

	info += QString("<TD BGCOLOR=\"");
	info += QApplication::palette().color(QPalette::Highlight).name();
	info += QString("\">");
	info += QString("<FONT COLOR=\"");
	info += QApplication::palette().color(QPalette::HighlightedText).name();
	info += QString("\">");
	info += QString("Device name");
	info += QString("</FONT>");
	info += QString("</TD>");
	info += QString("</TR>");

	open();

	bool atLeastOne = false;
	
	/* Output lines */
	for (int i = 0; i < MAX_USBDMX_DEVICES; i++)
	{
		if (m_devices[i] != NULL)
		{
			atLeastOne = true;

			info += QString("<TR>");
			info += QString("<TD>%1</TD>").arg(i + 1);
			info += QString("<TD>%1</TD>").arg(m_names[i]);
			info += QString("</TR>");
		}
	}

	close();

	if (atLeastOne == false)
	{
		info += QString("<TR>");
		info += QString("<TD COLSPAN=\"2\">No devices available</TD>");
		info += QString("</TR>");
	}
	
	info += QString("</TABLE>");

	info += QString("</BODY>");
	info += QString("</HTML>");

	return info;
}

/*****************************************************************************
 * Value Read/Write
 *****************************************************************************/

int USBDMXOut::writeChannel(t_channel channel, t_value value)
{
	int iFaceNo = int(channel / 512);
	int channelNo = channel % 512;
	int r;
	
	m_mutex.lock();
	
	m_values[iFaceNo][channelNo] = value;
	r = bulkWrite(iFaceNo);
	
	m_mutex.unlock();
	
	return r;
}

int USBDMXOut::writeRange(t_channel address, t_value* values, t_channel num)
{
	int iFaceNo = int(address / 512);
	int channelNo = address % 512;
	int r;
	
	Q_ASSERT(values != NULL);

	m_mutex.lock();
	memcpy(m_values[iFaceNo] + channelNo, values, num * sizeof(t_value));
	r = bulkWrite(iFaceNo);
	m_mutex.unlock();

	return r;
}

int USBDMXOut::readChannel(t_channel channel, t_value &value)
{
	int iFaceNo = int(channel / 512);
	int channelNo = channel % 512;

	m_mutex.lock();
	value = m_values[iFaceNo][channelNo];
	m_mutex.unlock();

	return 0;
}

int USBDMXOut::readRange(t_channel address, t_value* values, t_channel num)
{
	int iFaceNo = int(address / 512);
	int channelNo = address % 512;

	Q_ASSERT(values != NULL);

	m_mutex.lock();
	memcpy(values, m_values[iFaceNo] + channelNo, num * sizeof(t_value));
	m_mutex.unlock();

	return 0;
}

int USBDMXOut::bulkWrite(int iFaceNo)
{
	if (m_devices[iFaceNo] != 0)
	{
		if (usbdmx->tx_set(m_devices[iFaceNo], m_values[iFaceNo], 512))
			return 0;
		else
			return -1;
	}
	else
	{
		return -1;
	}
}

/*****************************************************************************
 * Plugin export
 ****************************************************************************/

Q_EXPORT_PLUGIN2(usbdmxout, USBDMXOut)
