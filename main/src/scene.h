/*
  Q Light Controller
  scene.h

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

#ifndef SCENE_H
#define SCENE_H

#include "function.h"
#include <qlist.h>

enum SceneValueType
  {
    Set,  // Normal value
    Fade, // Fade value
    NoSet // Empty value
  };

typedef struct
{
  unsigned char value;
  SceneValueType type;

} SceneValue;

class Scene : public Function
{
  Q_OBJECT

 public:
  Scene(unsigned long id = 0);
  Scene(Scene* sc);
  virtual  ~Scene();

  void copyFrom(Scene* sc);

 public:
  Event* getEvent(Feeder* feeder);
  void recalculateSpeed(Feeder* feeder);

  bool allocate(unsigned short channels);
  bool set(unsigned short ch, unsigned char value, SceneValueType type);
  bool clear(unsigned short ch);
  SceneValue channelValue(unsigned short ch);

  QString valueTypeString(int ch);

  void saveToFile(QFile &file);

  bool registerFunction(Feeder* feeder);
  bool unRegisterFunction(Feeder* feeder);

  void createContents(QList<QString> &list);
  
 private:
  SceneValue m_values[512];
};

#endif
