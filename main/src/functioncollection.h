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

#include "classes.h"
#include "function.h"

typedef struct
{
  Device* callerDevice;
  Function* feederFunction;
} CollectionItem;

class FunctionCollection : public Function
{
  Q_OBJECT

 public:
  FunctionCollection();
  ~FunctionCollection();

  QList <CollectionItem> items() const { return m_items; }

  Event* getEvent(Feeder* feeder);
  void recalculateSpeed (Feeder *feeder);

  int m_registerCount;

  void addItem(Device* device, Function* function);
  bool removeItem(Device* device, Function* function);
  bool removeItem(QString deviceString, QString functionString);

  void saveToFile(QFile &file);

  void increaseRegisterCount();

  void unRegisterFunction();
  void registerFunction(Feeder* feeder);

  void createContents(QList<QString> &list);

 private:
  QList <CollectionItem> m_items;

  void decreaseRegisterCount();

 private slots:
  void slotFunctionUnRegistered(Function* feeder, Function* controller, Device* caller, unsigned long feederID);

};

#endif
