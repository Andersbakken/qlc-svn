/*
  Q Light Controller
  monitor.h

  Copyright (c) Heikki Junnila

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

#include "common/types.h"

class QColor;
class QBrush;
class QPainter;
class QRect;
class QPixmap;
class QApplication;
class QFont;
class QTimer;
class QRegion;
class QMenuBar;
class QPopupMenu;
class QDomDocument;
class QDomElement;

#define KXMLQLCMonitor "Monitor"

class Monitor : public QWidget
{
	Q_OBJECT

public:
	Monitor(QWidget* parent);
	~Monitor();

	void init();

	void setUniverse(t_channel universe);

	static bool loader(QDomDocument* doc, QDomElement* root);
	bool loadXML(QDomDocument* doc, QDomElement* root);
	bool saveXML(QDomDocument* doc, QDomElement* fxi_root);

protected slots:
	void slotTimeOut();
	void slotMenuCallback(int);

protected:
	void connectTimer();
	void initMenu();

	void paintEvent(QPaintEvent* e);
	void closeEvent(QCloseEvent* e);

	// Fixture label painting
	void paintFixtureLabelAll(QRegion region, int x_offset, int y_offset,
				  int unitW, int unitH, int unitsX);
	void paintFixtureLabel(int x, int y, int w, int h, QString label);

	// Channel label painting
	void paintChannelLabelAll(QRegion region, int x_offset, int y_offset,
				  int unitW, int unitH, int unitsX);
	void paintChannelLabel(int x, int y, int w, int h, QString s);

	// Channel value painting
	void paintChannelValueAll(QRegion region, int x_offset, int y_offset,
				  int unitW, int unitH, int unitsX,
				  bool onlyDelta);
	void paintChannelValue(int x, int y, int w, int h, QString s);

signals:
	void closed();

protected:
	t_channel m_universe;

	t_value* m_oldValues;
	t_value* m_newValues;
	QMutex m_valueMutex;
  
	QTimer* m_timer;
	int m_updateFrequency;

	QPainter m_painter;
	QFont m_font;

	QMenuBar* m_menuBar;
	QPopupMenu* m_universeMenu;
	QPopupMenu* m_displayMenu;
	QPopupMenu* m_speedMenu;

};

#endif
