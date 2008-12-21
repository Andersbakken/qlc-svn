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

#include <QPainter>
#include <QWidget>
#include <QMutex>

#include "common/qlctypes.h"

class QDomDocument;
class QDomElement;
class QPixmap;
class QAction;
class QColor;
class QBrush;
class QTimer;
class QRect;
class QFont;
class QMenu;
class QIcon;

class OutputMap;

#define KXMLQLCMonitor "Monitor"
#define KXMLQLCMonitorFont "Font"
#define KXMLQLCMonitorUpdateFrequency "UpdateFrequency"

class Monitor : public QWidget
{
	Q_OBJECT

public:
	Monitor(QWidget* parent);
	~Monitor();

private:
	Q_DISABLE_COPY(Monitor)

protected:
	void init();

	/*********************************************************************
	 * Content
	 *********************************************************************/
protected:
	void setUniverse(t_channel universe);
	void setFrequency(int freq);

protected slots:
	void slotTimeOut();

protected:
	void connectTimer();

protected:
	QTimer* m_timer;
	int m_updateFrequency;

	/** The universe number under inspection */
	t_channel m_universe;

	/** Old value buffer */
	t_value* m_oldValues;

	/** New value buffer */
	t_value* m_newValues;

	/** Value buffer mutex */
	QMutex m_valueMutex;

	/*********************************************************************
	 * Menu
	 *********************************************************************/
protected:
	void initActions();
	void initMenu();
	void slotMenuTriggered(QAction* action);

protected slots:
	void slotUniverseTriggered(QAction* action);
	void slotFont();
	void slot16Hz();
	void slot32Hz();
	void slot64Hz();

protected:
	QList <QAction*> m_universeActions;
	QAction* m_fontAction;
	QAction* m_16HzAction;
	QAction* m_32HzAction;
	QAction* m_64HzAction;

	/*********************************************************************
	 * Painting
	 *********************************************************************/
protected:
	void paintEvent(QPaintEvent* e);

	/** Fixture label painting */
	void paintFixtureLabelAll(QRegion region, int x_offset, int y_offset,
				  int unitW, int unitH, int unitsX);
	void paintFixtureLabel(int x, int y, int w, int h, QString label);

	/** Channel label painting */
	void paintChannelLabelAll(QRegion region, int x_offset, int y_offset,
				  int unitW, int unitH, int unitsX);
	void paintChannelLabel(int x, int y, int w, int h, QString s);

	/** Channel value painting */
	void paintChannelValueAll(QRegion region, int x_offset, int y_offset,
				  int unitW, int unitH, int unitsX,
				  bool onlyDelta);
	void paintChannelValue(int x, int y, int w, int h, QString s);

protected:
	/** The smallest visible channel number */
	t_channel m_visibleMin;

	/** The biggest visible channel number */
	t_channel m_visibleMax;

	/** Master painter object that draws stuff on the widget */
	QPainter m_painter;

	/** Font used */
	QFont m_font;

	/*********************************************************************
	 * Save & Load
	 *********************************************************************/
public:
	static void loader(QDomDocument* doc, QDomElement* root);
	bool loadXML(QDomDocument* doc, QDomElement* root);
	bool saveXML(QDomDocument* doc, QDomElement* fxi_root);
};

#endif
