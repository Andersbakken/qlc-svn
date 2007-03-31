/*
  Q Light Controller
  chaser.h
  
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

#ifndef CHASER_H
#define CHASER_H

#include <qvaluelist.h>
#include "function.h"

class Event;
class QFile;
class QString;
class QDomDocument;

class Chaser : public Function
{
 public:
	Chaser();
	virtual ~Chaser();

	void copyFrom(Chaser* ch, bool append = false);

	bool setDevice(t_device_id) { return false; }

	void addStep(t_function_id);
	void removeStep(int index = 0);

	bool raiseStep(unsigned int index);
	bool lowerStep(unsigned int index);

	void setRunOrder(RunOrder ro);
	RunOrder runOrder() { return m_runOrder; }

	void setDirection(Direction dir);
	Direction direction() { return m_direction; }

	QValueList <t_function_id> *steps() { return &m_steps; }

	void saveToFile(QFile &file);
	void createContents(QPtrList <QString> &list);

	void saveXML(QDomDocument* doc);

	void busValueChanged(t_bus_id, t_bus_value);

	void arm();
	void disarm();
	void cleanup();
	void stop();
	void childFinished();

 protected:
	void init();
	void run();

	bool startMemberAt(int);
	void stopMemberAt(int);
	void hold();

 protected:
	QValueList <t_function_id> m_steps;

	RunOrder m_runOrder;
	Direction m_direction;

	bool m_childRunning;

	t_bus_value m_holdTime;
	t_bus_value m_holdStart;
	t_bus_value m_timeCode;

	Direction m_runTimeDirection;
	int m_runTimePosition;
};

#endif
