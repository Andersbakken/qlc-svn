/*
  Q Light Controller
  hiddevice.cpp

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

#include <qobject.h>
#include <qstring.h>
#include <qfile.h>

#include "hiddevice.h"
#include "hidinput.h"

HIDDevice::HIDDevice(HIDInput* parent, const QString& path) 
	: QObject(parent, "HIDDevice")
{
	Q_ASSERT(path != QString::null);
	m_file.setName(path);
}

HIDDevice::~HIDDevice()
{
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

bool HIDDevice::open()
{
	bool result = false;

	result = m_file.open(IO_Raw | IO_ReadWrite);
	if (result == false)
	{
		qWarning("HIDDevice: Unable to open %s in read/write mode. " \
			 "Trying read-only.", (const char*) m_file.name());
		
		result = m_file.open(IO_Raw | IO_ReadOnly);
		if (result == false)
		{
			qWarning("HIDDevice: Unable to open %s read-only. " \
				 "Giving up.", (const char*) m_file.name());
		}
	}

	return result;
}

void HIDDevice::close()
{
	m_file.close();
}

QString HIDDevice::path() const
{
	return m_file.name();
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void HIDDevice::feedBack(t_input_channel channel, t_input_value value)
{
}
