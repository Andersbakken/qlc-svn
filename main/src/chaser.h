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
#include "classes.h"
#include "function.h"
#include "scene.h"
#include "sequencestructs.h"

class Chaser : public Function
{
  Q_OBJECT

 public:
  Chaser();
  ~Chaser();

  void addStep(Device* device, Function* function);
  void removeStep(int index = 0);

  int steps(void);
  ChaserStep* at(int index); 

  Event* getEvent(Feeder* feeder);

  void saveToFile(QFile &file);

  void registerFirstStep();

  void unRegister();

  void createContents(QList<QString> &list);

 public slots:
  void slotFunctionUnRegistered(Function* function, Function* controller, Device* caller, unsigned long feederID);

 protected:
  QList <ChaserStep> m_steps;

  bool m_running; // One chaser object can be running only once at a time
  bool m_OKforNextStep;

  unsigned long m_repeatTimes;
};

#endif
