/*
  Q Light Controller
  vcdockslider.h

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

#ifndef VCDOCKSLIDER_H
#define VCDOCKSLIDER_H

#include <qptrlist.h>
#include <qstring.h>

#include "uic_vcdockslider.h"
#include "types.h"

class QMouseEvent;
class QPopupMenu;
class QFile;

class VCDockSlider : public UI_VCDockSlider
{
  Q_OBJECT
    
 public:
  VCDockSlider(QWidget* parent, bool isStatic = false,
	       const char* name = NULL);
  ~VCDockSlider();

  void init();
  
  enum Mode
    {
      Normal = 0,
      Speed  = 1,
      Master = 2
    };

  bool setBusID(t_bus_id id);

  void createContents(QPtrList <QString> &list);
  void saveToFile(QFile &file, t_vc_id parentID);

  Mode mode() { return m_mode; }
  QString modeString(Mode mode);

 private slots:
  void slotSliderValueChanged(int);
  void slotBusMenuActivated(int id);

  void slotModeChanged();

  void slotBusNameChanged(t_bus_id, const QString&);
  void slotBusValueChanged(t_bus_id, t_bus_value);

 protected:
  void updateBusMenu();

  void mousePressEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void mouseMoveEvent(QMouseEvent* e);
  void paintEvent(QPaintEvent* e);

  bool moveThreshold(int x, int y);
  void moveTo(int x, int y);

 private:
  t_vc_id m_id;
  Mode m_mode;
  t_bus_id m_busID;

  bool m_static;
  bool m_updateOnly;

  int m_origX;
  int m_origY;
  int m_xpos;
  int m_ypos;
  bool m_resizeMode;

  QPopupMenu* m_busMenu;
};

#endif

