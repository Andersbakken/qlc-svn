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

class CollectionItem
{
 public:
  CollectionItem()
    {
      m_device = NULL;
      m_function = NULL;
      m_registered = false;
    }

  CollectionItem(CollectionItem* item)
    {
      m_device = item->m_device;
      m_function = item->m_function;
      m_registered = item->m_registered;
    }

  virtual ~CollectionItem()
    {
    }

  void setDevice(DMXDevice* d) { m_device = d; }
  DMXDevice* device() { return m_device; }

  void setFunction(Function* f) { m_function = f; }
  Function* function() { return m_function; }

  void setRegistered(bool reg) { m_registered = reg; }
  bool registered() { return m_registered; }

 private:
  DMXDevice* m_device;
  Function* m_function;
  bool m_registered;

};

class FunctionCollection : public Function
{
  Q_OBJECT

 public:
  FunctionCollection(unsigned long id = 0);
  FunctionCollection(FunctionCollection* fc);
  virtual ~FunctionCollection();

  QList <CollectionItem> *items() { return &m_items; }

  Event* getEvent(Feeder* feeder);
  void recalculateSpeed (Feeder *feeder);

  void addItem(DMXDevice* device, Function* function);
  bool removeItem(DMXDevice* device, Function* function);
  bool removeItem(const unsigned long functionId);

  void saveToFile(QFile &file);

  bool registerFunction(Feeder* feeder);
  bool unRegisterFunction(Feeder* feeder);

  void createContents(QList<QString> &list);

 private:
  QList <CollectionItem> m_items;

  void decreaseRegisterCount();
  void increaseRegisterCount();

  int m_registerCount;

 private slots:
  void slotFunctionUnRegistered(Function* feeder, Function* controller, DMXDevice* caller, unsigned long feederID);
  void slotMemberFunctionDestroyed(Function* f);
};

#endif
