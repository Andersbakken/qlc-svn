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

#include <qlist.h>
#include "function.h"

class Feeder;
class Scene;
class Event;
class QFile;
class QString;

class ChaserStep
{
 public:
  ChaserStep(Function* function = NULL)
    {
      m_function = function;
    }

  ChaserStep(ChaserStep* step)
    {
      m_function = step->function();
    }

  virtual ~ChaserStep()
    {
    }

  void setFunction(Function* f) { m_function = f; }
  Function* function() { return m_function; }

 private:
  Function* m_function;
};

class Chaser : public Function
{
  Q_OBJECT

 public:
  Chaser();
  Chaser(Chaser* ch, bool append = true);
  virtual ~Chaser();

  void copyFrom(Chaser* ch, bool append = true);

  void addStep(Function* function);
  void removeStep(int index = 0);

  void raiseStep(int index);
  void lowerStep(int index);

  QList <ChaserStep> *steps() { return &m_steps; }

  Event* getEvent(Feeder* feeder);
  void recalculateSpeed (Feeder *f);

  void saveToFile(QFile &file);

  void registerFirstStep();

  bool registerFunction(Feeder* feeder);
  bool unRegisterFunction(Feeder* feeder);

  void createContents(QList<QString> &list);

  QList <ChaserStep> m_steps;

 public slots:
  void slotFunctionUnRegistered(Function* function, Function* controller, DMXDevice* caller, unsigned long feederID);
  void slotMemberFunctionDestroyed(Function*);

 protected:
  bool m_running; // One chaser object can be running only once at a time
  bool m_OKforNextStep;

  unsigned long m_repeatTimes;
};

#endif
