/*
  Q Light Controller
  VCLabel.h

  Copyright (C) Heikki Junnila
                Stefan Krumm
  
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

#ifndef VCLABEL_H
#define VCLABEL_H

#include <qlabel.h>
#include <qptrlist.h>
#include "vcwidgetbase.h"

class QLineEdit;
class QMouseEvent;
class QFile;
class QString;

class VCWidget;
class FloatingEdit;

#define VCLABEL_MENU_FONT        1110
#define VCLABEL_MENU_F_COLOR     1120
#define VCLABEL_MENU_B_COLOR     1130
#define VCLABEL_MENU_NO_BACKGR   1140

class VCLabel : public QLabel,
		public VCWidgetBase
{
  Q_OBJECT

 public:
  VCLabel(VCWidget* parent);
  ~VCLabel();

  void init();

  void saveToFile(QFile& file, unsigned int parentID);
  void createContents(QPtrList <QString> &list);

 private:
  int m_origX;
  int m_origY;

  FloatingEdit* m_renameEdit;

 private:
  bool moveThreshold(int x, int y);
  bool m_noBackground;
  void moveTo(int x, int y);
  void setTransparent();

 public slots:
  void slotRenameReturnPressed();

 private slots:
  void slotMenuCallback(int item);
  void slotBackgroundChanged();

 protected:
  void mousePressEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void mouseMoveEvent(QMouseEvent* e);
  void mouseDoubleClickEvent(QMouseEvent* e);
};

#endif

