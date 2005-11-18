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

class QLineEdit;
class QMouseEvent;
class QPaintEvent;
class QFile;
class QString;

class VCFrame;
class FloatingEdit;

class VCLabel : public QLabel
{
  Q_OBJECT

 public:
  VCLabel(QWidget* parent);
  ~VCLabel();

  void init();

  void saveToFile(QFile& file, unsigned int parentID);
  void createContents(QPtrList <QString> &list);

 public slots:
  void setCaption(const QString&);

 private:
  void invokeMenu(QPoint);
  void parseWidgetMenu(int);
  void resizeTo(QPoint);
  void moveTo(QPoint);

 private slots:
  void slotModeChanged();

 protected:
  void mousePressEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent*);
  void mouseDoubleClickEvent(QMouseEvent*);
  void paintEvent(QPaintEvent*);
  void customEvent(QCustomEvent*);

 private:
  QPoint m_mousePressPoint;
  bool m_resizeMode;
};

#endif

