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
class QPaintEvent;
class QFile;
class QString;

class VCWidget;
class FloatingEdit;

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

  bool moveThreshold(int x, int y);
  bool m_background;
  bool m_resizeMode;

 private:
  void moveTo(int x, int y);
  void setTransparent(bool);

 public slots:
  void slotRenameReturnPressed();

 private slots:
  void slotMenuCallback(int item);
  void slotModeChanged();

 protected:
  void mousePressEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void mouseMoveEvent(QMouseEvent* e);
  void mouseDoubleClickEvent(QMouseEvent* e);
  void paintEvent(QPaintEvent* e);
};

#endif

