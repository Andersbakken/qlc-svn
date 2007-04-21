/*
  Q Light Controller
  logicalchannel.h
  
  Copyright (C) 2000, 2001, 2002 Heikki Junnila
  
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

#ifndef LOGICALCHANNEL_H
#define LOGICALCHANNEL_H

#include <qstring.h>
#include <qptrlist.h>
#include "types.h"

class Capability;
class QFile;

class LogicalChannel
{
 public:
	LogicalChannel();
	LogicalChannel(LogicalChannel* lc);
	~LogicalChannel();

	t_channel channel() const { return m_channel; }
	QString name() const { return m_name; }

	QPtrList <Capability> *capabilities() { return &m_capabilities; }
	Capability* searchCapability(t_value value);
	Capability* searchCapability(QString name);

	void setChannel(const t_channel &channel) { m_channel = channel; }
	void setName(const QString &name) { m_name = QString(name); }

	void saveToFile(QFile &file);

 private:
	t_channel m_channel;
	QString m_name;

	QPtrList <Capability> m_capabilities;
};

#endif
