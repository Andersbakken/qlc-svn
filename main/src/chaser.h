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

#include <qptrlist.h>
#include "function.h"

class Scene;
class Event;
class QFile;
class QString;
class FunctionStep;

class Chaser : public Function
{
 public:
  Chaser(t_function_id id = 0);
  Chaser(Chaser* ch, bool append = false);
  virtual ~Chaser();

  void copyFrom(Chaser* ch, bool append = false);

  void addStep(Function* function);
  void removeStep(int index = 0);

  void raiseStep(unsigned int index);
  void lowerStep(unsigned int index);

  QPtrList <FunctionStep> *steps() { return &m_steps; }

  void saveToFile(QFile &file);
  void createContents(QPtrList <QString> &list);

  void busValueChanged(t_bus_id, t_bus_value);
  
  void freeRunTimeData();
  void childFinished();

 protected:
  void init();
  void run();

 protected:
  QPtrList <FunctionStep> m_steps;

  bool m_childRunning;
  QMutex m_childMutex;

  time_t m_holdTime;
};

#endif
