/*
  Q Light Controller
  vcxypad.h
  
  Copyright (C) 2005 Heikki Junnila, Stefan Krumm
  
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

#ifndef VCXYPAD_H
#define VCXYPAD_H

#include <qframe.h>
#include <qptrlist.h>

#include "types.h"

class QFile;
class QString;
class QPaintEvent;
class QMouseEvent;

class VCXYPad : public QFrame
{
  Q_OBJECT

 public:
  VCXYPad(QWidget* parent);
  virtual ~VCXYPad();

  enum ButtonBehaviour
    {
      Normal = 0,
      Exclusive = 1
    };
  
  void init();

  t_vc_id id() const { return m_id; }

  void saveFramesToFile(QFile& file, t_vc_id parentID = 0);
  void saveChildrenToFile(QFile& file);
  virtual void createContents(QPtrList <QString> &list);

  void setBottomFrame(bool set = true);
  bool isBottomFrame() { return m_bottomFrame; }

  void setButtonBehaviour(ButtonBehaviour);
  ButtonBehaviour buttonBehaviour() { return m_buttonBehaviour; }

 private slots:
  void slotModeChanged();

 signals:
  void backgroundChanged();

 protected:
  void setID(t_vc_id id);
  void invokeMenu(QPoint point);
  void parseWidgetMenu(int item);

  void mousePressEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void mouseMoveEvent(QMouseEvent* e);
  void paintEvent(QPaintEvent* e);
  void customEvent(QCustomEvent* e);

  void resizeTo(QPoint p);
  void moveTo(QPoint p);

 protected:
  int m_origX;
  int m_origY;
  int m_xpos;
  int m_ypos;

  t_vc_id m_id;

  bool m_resizeMode;
  bool m_bottomFrame;

  ButtonBehaviour m_buttonBehaviour;

 private:
  static t_vc_id s_nextVCID;

 public:
  static void ResetID() { s_nextVCID = KVCIDMin; }
};

#endif
