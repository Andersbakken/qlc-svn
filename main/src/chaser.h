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
      if (function)
	{
	  m_functionId = function->id();
	}
      else
	{
	  m_functionId = 0;
	}
    }

  ChaserStep(ChaserStep* step)
    {
      ASSERT(step && step->function());
      m_function = step->function();
      m_functionId = step->functionId();
    }

  virtual ~ChaserStep()
    {
    }

  void setFunction(Function* f)
    {
      m_function = f;
      if (f)
	{
	  m_functionId = f->id();
	}
      else
	{
	  m_functionId = 0;
	}
    }

  Function* function()
    { 
      return m_function;
    }

  unsigned long functionId()
    { 
      return m_functionId;
    }

 private:
  Function* m_function;
  unsigned long m_functionId; // Used when deleting items because function might already be invalid
};

class Chaser : public Function
{
  Q_OBJECT

 public:
  Chaser(unsigned long id = 0);
  Chaser(Chaser* ch, bool append = true);
  virtual ~Chaser();

  void copyFrom(Chaser* ch, bool append = true);

  void addStep(Function* function);
  void removeStep(int index = 0);

  void raiseStep(unsigned int index);
  void lowerStep(unsigned int index);

  QPtrList <ChaserStep> *steps() { return &m_steps; }

  Event* getEvent(Feeder* feeder);
  void recalculateSpeed (Feeder *f);

  void saveToFile(QFile &file);

  void registerFirstStep();

  bool registerFunction(Feeder* feeder);
  bool unRegisterFunction(Feeder* feeder);

  void createContents(QList<QString> &list);

  QPtrList <ChaserStep> m_steps;

 public slots:
  void slotFunctionUnRegistered(Function* function, Function* controller, unsigned long feederID);
  void slotMemberFunctionDestroyed(unsigned long fid);

 protected:
  bool m_running; // One chaser object can be running only once at a time
  bool m_OKforNextStep;

  unsigned long m_repeatTimes;
};

#endif
