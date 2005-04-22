/*
  Q Light Controller
  vcxypadproperties.h
  
  Copyright (C) 2005, Stefan Krumm, Heikki Junnila
  
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

#ifndef VCXYPADROPERTIES_H
#define VCXYPADROPERTIES_H

#include "types.h"
#include "vcxypad.h"
#include "uic_vcxypadproperties.h"


const int Kdmx        (7);
const int Kdevice     (0);
const int Kchannel    (1);
const int Kcapability (2);
const int Kmin        (3);
const int Kmax        (4);
const int Kreverse    (5);
const int Kid         (6);


class VCXYPadProperties : public UI_VCXYPadProperties
{
  Q_OBJECT

 public:
  VCXYPadProperties(QWidget* parent = 0, const char* name = 0);
  ~VCXYPadProperties();

  void init();
  void fillIt(QListView *list, QPtrList<XYChannelUnit> channels);
  
  //QPtrList<XYChannelUnit>* channelsX(){return m_channelsX;}
 // void setPointerListX(QPtrList<XYChannelUnit> xl)
 //                             { m_channelsX = xl;}
  //QPtrList<XYChannelUnit>* m_channelsX;
  
 public slots:
    virtual void slotAddX();
    virtual void slotEditX();
    virtual void slotRemoveX();
    virtual void slotAddY();
    virtual void slotRemoveY();
    virtual void slotEditY();
    virtual void slotEdit(QListView *list);
    virtual void slotAdd(QListView *list);
 protected:
  VCXYPad* m_parent;

};

#endif
