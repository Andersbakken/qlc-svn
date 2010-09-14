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

#include <linux/input.h>
#include <errno.h>

#include <QString>

#include "hiddevice.h"
#include "hidinput.h"

HIDDevice::HIDDevice(HIDInput* parent, quint32 line, const QString& path) 
	: QObject(parent)
{
	Q_ASSERT(path.length() > 0);
	m_file.setFileName(path);
	m_line = line;
}

HIDDevice::~HIDDevice()
{
	close();
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

bool HIDDevice::open()
{
	return false;
}

void HIDDevice::close()
{
}

QString HIDDevice::path() const
{
	return QString::null;
}

int HIDDevice::handle() const
{
	return m_file.handle();
}

/*****************************************************************************
 * Device info
 *****************************************************************************/

QString HIDDevice::infoText()
{
	return QString::null;
}

QString HIDDevice::name()
{
	return m_name;
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void HIDDevice::feedBack(quint32 /*channel*/, uchar /*value*/)
{
}
