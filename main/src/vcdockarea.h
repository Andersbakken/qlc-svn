/*
  Q Light Controller
  vcdockarea.h
  
  Copyright (C) Heikki Junnila
  
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

#ifndef VCDOCKAREA_H
#define VCDOCKAREA_H

#include <qframe.h>

class QVBoxLayout;
class QToolButton;

class VCDockSlider;

class VCDockArea : public QFrame
{
  Q_OBJECT
    
    public:
  VCDockArea(QWidget* parent, const char* name = NULL);
  ~VCDockArea();

  void init();
  void show();
  void hide();

 signals:
  void areaHidden(bool);

 public slots:
  void slotHideButtonClicked();

 protected:
  QVBoxLayout* m_layout;

  VCDockSlider* m_defaultFadeSlider;
  VCDockSlider* m_defaultHoldSlider;
  VCDockSlider* m_masterSlider;

  QToolButton* m_hideButton;
};

#endif
