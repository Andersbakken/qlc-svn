/*
  Q Light Controller
  functioncollection.h
  
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

#ifndef FUNCTIONCOLLECTION_H
#define FUNCTIONCOLLECTION_H

#include "function.h"

class FunctionStep;

class FunctionCollection : public Function
{
 public:
  FunctionCollection(t_function_id id = 0);
  FunctionCollection(FunctionCollection* fc);
  void copyFrom(FunctionCollection* fc);
  virtual ~FunctionCollection();

  QPtrList <FunctionStep> *steps() { return &m_steps; }

  bool addItem(Function* function);
  bool removeItem(Function* function);
  bool removeItem(const t_function_id functionId);

  void saveToFile(QFile &file);
  void createContents(QPtrList <QString> &list);

  void speedChange();
  void stop();
  void freeRunTimeData();

  void functionStopped();

 protected:
  void init();
  void run();

 private:
  QPtrList <FunctionStep> m_steps;

  int m_childCount;
  QMutex m_childCountMutex;
};

#endif
