/*
  Q Light Controller
  vcbutton.h
  
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

#ifndef VCBUTTON_H
#define VCBUTTON_H

#include <qpushbutton.h>
#include <qptrlist.h>

#include "vcwidgetbase.h"

class QLineEdit;
class QMouseEvent;
class QPaintEvent;
class QFile;
class QString;

class VCWidget;
class Function;
class FloatingEdit;

class VCButton : public QPushButton,
		 public VCWidgetBase
{
  Q_OBJECT

 public:
  VCButton(VCWidget* parent);
  ~VCButton();

  void init();
  void copyFrom(VCButton* button);

  Function* function() const;

  KeyBind* keyBind() { return m_keyBind; }
  QColor* bgColor() { return m_bgColor; }
  QPixmap* bgPixmap() const { return m_bgPixmap; }

  void saveToFile(QFile& file, unsigned int parentID);
  void createContents(QPtrList <QString> &list);

 private:
  int m_origX;
  int m_origY;
  
  QColor* m_bgColor;

  Function* m_function;

  bool m_resizeMode;

  FloatingEdit* m_renameEdit;

  void keyPress(QKeyEvent* e);
  void keyRelease(QKeyEvent* e);

 private:
  bool moveThreshold(int x, int y);
  void moveTo(int x, int y);

  void pressFunction();
  void releaseFunction();
  void attachFunction(Function* function);

 public slots:
  void slotRenameReturnPressed();
  void slotFunctionDestroyed();

 private slots:
  void slotMenuCallback(int item);
  void slotFlashReady();
  void slotModeChanged();

 protected:
  QPixmap* m_bgPixmap;
  QString m_bgPixmapFileName;
  void mousePressEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void mouseMoveEvent(QMouseEvent* e);
  void mouseDoubleClickEvent(QMouseEvent* e);
  void paintEvent(QPaintEvent* e);
  void customEvent(QCustomEvent* e);
};

#endif
