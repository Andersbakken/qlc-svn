/*
  Q Light Controller
  monitor.h

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

#ifndef MONITOR_H
#define MONITOR_H

#include <qframe.h>
#include <qptrlist.h>
#include <qmutex.h>
#include <qpainter.h>

#include "types.h"

class QColor;
class QBrush;
class QPainter;
class QRect;
class QPixmap;
class QApplication;
class QFont;
class QTimer;

class Monitor : public QWidget
{
	Q_OBJECT

public:
	Monitor(QWidget* parent);
	~Monitor();

	void init();

protected slots:
	void slotTimeOut();
	void slotMenuCallback(int);

protected:
	void connectTimer();

	void paintEvent(QPaintEvent* e);
	void mousePressEvent(QMouseEvent* e);
	void closeEvent(QCloseEvent* e);

	void paint(QPaintEvent* e);
	void paintAll(QPaintEvent* e);

	void loadGeometry();
	void saveGeometry();

signals:
	void closed();

protected:
	t_channel m_fromChannel;
	t_channel m_toChannel;
	t_channel m_units;

	t_value* m_oldValues;
	t_value* m_newValues;
	QMutex m_valueMutex;
  
	QTimer* m_timer;
	int m_updateFrequency;

	QPainter m_painter;
	QFont m_font;
};

#endif
