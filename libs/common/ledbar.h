/*
  Q Light Controller
  ledbar.h
  
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

#ifndef LEDBAR_H
#define LEDBAR_H

#include <qframe.h>

class QColor;
class QBrush;
class QPainter;
class QRect;
class QPixmap;
class QApplication;
class QFont;

class LedBar : public QFrame
{
  Q_OBJECT
 
 public:
  LedBar(QWidget *parent, QApplication* qapp, const char *name = 0);
  ~LedBar();

  static int width();
  static int height();

  void setRange(int min, int max);
  void setText(QString text);
  void setFont(QFont font);
  void setToolTip(QString text = QString::null);

 public slots:
  void slotSetValue(int value);

 protected:
  void paintEvent(QPaintEvent* e);
  void paint(void);

 private:
  int m_min;
  int m_max;
  int m_value;
  QString m_text;

  QColor m_color;
  QBrush m_brush;
  QFrame* m_drawFrame;

  QApplication* m_qapp;
  QFont m_font;
};

#endif
