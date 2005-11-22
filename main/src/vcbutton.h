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
#include "types.h"

class QLineEdit;
class QMouseEvent;
class QPaintEvent;
class QFile;
class QString;
class QColor;
class QPixmap;
class QPoint;

class FloatingEdit;
class KeyBind;

class VCButton : public QPushButton
{
  Q_OBJECT

 public:
  VCButton(QWidget* parent);
  ~VCButton();

  void init();
  void copyFrom(VCButton* button);

  KeyBind* keyBind() { return m_keyBind; }
  void setKeyBind(const KeyBind* kb);

  void saveToFile(QFile& file, unsigned int parentID);
  void createContents(QPtrList <QString> &list);

  void attachFunction(t_function_id id);
  t_function_id functionID() const { return m_functionID; }

  void setExclusive(bool exclusive = true);
  bool isExclusive() { return m_isExclusive; }

 private:
  void invokeMenu(QPoint);
  void parseWidgetMenu(int);
  bool moveThreshold(int x, int y);
  void resizeTo(QPoint p);
  void moveTo(QPoint p);

 public slots:
  void setCaption(const QString& text);

  void pressFunction();
  void releaseFunction();

 private slots:
  void slotFlashReady();
  void slotModeChanged();

 protected:
  void mousePressEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void mouseMoveEvent(QMouseEvent* e);
  void mouseDoubleClickEvent(QMouseEvent* e);
  void paintEvent(QPaintEvent* e);
  void customEvent(QCustomEvent* e);
  bool m_isExclusive;

 private:
  t_function_id m_functionID;
  KeyBind* m_keyBind;

  QPoint m_mousePressPoint;
  bool m_resizeMode;
};

#endif
