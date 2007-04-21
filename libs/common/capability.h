/*
  Q Light Controller
  capability.h
  
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

#ifndef CAPABILITY_H
#define CAPABILITY_H

#include <qptrlist.h>

#include "function.h"
#include "types.h"

class QString;
class QFile;

class Capability
{
 public:
	Capability();
	Capability(Capability* cap);
	virtual ~Capability();
  
	t_value lo() { return m_lo; }
	t_value hi() { return m_hi; }
	QString name() { return m_name; }

	void setLo(const t_value &value) { m_lo = value; }
	void setHi(const t_value &value) { m_hi = value; }
	void setName(const QString &name) { m_name = QString(name); }
  
	void createInfo(QPtrList <QString> &list);

	void saveToFile(QFile &file);

 private:
	t_value m_lo;
	t_value m_hi;

	QString m_name;
};

#endif
