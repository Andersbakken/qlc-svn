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
#include <qpixmap.h>

#include "common/types.h"
#include "device.h"

class QFile;
class QString;
class QPaintEvent;
class QMouseEvent;
class XYChannelUnit;

class VCXYPad : public QFrame
{
  Q_OBJECT

 public:
  VCXYPad(QWidget* parent);
  virtual ~VCXYPad();

  void init();

  void saveToFile(QFile& file, t_vc_id parentID = 0);
  virtual void createContents(QPtrList <QString> &list);

  QPtrList<XYChannelUnit>* channelsX() { return &m_channelsX; }
  QPtrList<XYChannelUnit>* channelsY() { return &m_channelsY; }
  
 private slots:
  void slotModeChanged();

 signals:
  void backgroundChanged();

 protected:
  void invokeMenu(QPoint point);
  void parseWidgetMenu(int item);

  void mousePressEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void mouseMoveEvent(QMouseEvent* e);
  void paintEvent(QPaintEvent* e);
  void customEvent(QCustomEvent* e);

  void resizeTo(QPoint p);
  void moveTo(QPoint p);
  void outputDMX(int x, int y);

  void createChannelUnitFromString(QString string, bool isX);
 
 protected:
  int m_xpos;
  int m_ypos;

  QPoint m_mousePressPoint;
  bool m_resizeMode;
 
  QPoint m_currentXYPosition;
  QPixmap m_pixmap;

  QPtrList<XYChannelUnit> m_channelsX;
  QPtrList<XYChannelUnit> m_channelsY;
};

#endif
