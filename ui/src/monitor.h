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

#include <QWidget>
#include <QHash>
#include <QList>

#include "vcwidgetproperties.h"
#include "qlctypes.h"

class MonitorProperties;
class MonitorFixture;
class MonitorLayout;
class QDomDocument;
class QDomElement;
class QScrollArea;
class OutputMap;
class QAction;
class Fixture;
class Monitor;
class Doc;

#define KXMLQLCMonitor "Monitor"
#define KXMLQLCMonitorFont "Font"

#define KXMLQLCMonitorChannelStyle "ChannelStyle"
#define KXMLQLCMonitorChannelStyleDMX "DMX"
#define KXMLQLCMonitorChannelStyleRelative "Relative"

#define KXMLQLCMonitorValueStyle "ValueStyle"
#define KXMLQLCMonitorValueStyleDMX "DMX"
#define KXMLQLCMonitorValueStylePercentage "Percentage"

/*****************************************************************************
 * Monitor Properties
 *****************************************************************************/

class MonitorProperties : public VCWidgetProperties
{
	/** Monitor can modify protected members */
	friend class Monitor;

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	MonitorProperties();
	virtual ~MonitorProperties();

	/*********************************************************************
	 * Font
	 *********************************************************************/
public:
	/** Get the font used for monitor fixture labels */
	QString font() const { return m_font; }

protected:
	QString m_font;

	/*********************************************************************
	 * Channel & Value styles
	 *********************************************************************/
public:
	enum ChannelStyle { DMXChannels, RelativeChannels };
	enum ValueStyle { DMXValues, PercentageValues };

	/** Get the style used to draw DMX values in monitor fixtures */
	ValueStyle valueStyle() const { return m_valueStyle; }

	/** Get the style used to draw channel numbers in monitor fixtures */
	ChannelStyle channelStyle() const { return m_channelStyle; }

protected:
	static QString channelStyleToString(ChannelStyle style);
	static ChannelStyle stringToChannelStyle(const QString& str);

	static QString valueStyleToString(ValueStyle style);
	static ValueStyle stringToValueStyle(const QString& str);

protected:
	ValueStyle m_valueStyle;
	ChannelStyle m_channelStyle;

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	/** Store latest widget properties */
	void store(Monitor* monitor);

	/** Load contents from the given XML document */
	bool loadXML(const QDomElement* root);

	/** Save contents to the given XML document */
	bool saveXML(QDomDocument* doc, QDomElement* root);
};

/*****************************************************************************
 * Monitor
 *****************************************************************************/

class Monitor : public QWidget
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/** Get the monitor singleton instance. Can be NULL. */
	static Monitor* instance() { return s_instance; }

	/** Create a Monitor with parent. Fails if s_instance is not NULL. */
	static void create(QWidget* parent);

	/** Normal public destructor */
	~Monitor();

protected:
	/** Protected constructor to prevent multiple instances. */
	Monitor(QWidget* parent, Qt::WindowFlags f = 0);

private:
	Q_DISABLE_COPY(Monitor)

protected:
	/** The singleton Monitor instance */
	static Monitor* s_instance;

	/*********************************************************************
	 * Properties
	 *********************************************************************/
public:
	/** Get properties for reading. */
	static const MonitorProperties& properties() { return s_properties; }

protected:
	/** Monitor properties */
	static MonitorProperties s_properties;

	/*********************************************************************
	 * Menu
	 *********************************************************************/
protected:
	/** Create tool bar */
	void initToolBar();

protected slots:
	/** Menu action slot for font selection */
	void slotChooseFont();

	/** Menu action slot for channel style selection */
	void slotChannelStyleTriggered();

	/** Menu action slot for value style selection */
	void slotValueStyleTriggered();

	/********************************************************************
	 * Monitor Fixtures
	 ********************************************************************/
public:
	/** Update monitor fixture labels */
	void updateFixtureLabelStyles();

protected:
	/** Create a new MonitorFixture* and append it to the layout */
	void createMonitorFixture(Fixture* fxi);

protected slots:
	/** Slot for toplevel document changes (to rehash contents) */
	void slotDocumentChanged(Doc* doc);

	/** Slot for fixture additions (to append the new fixture to layout) */
	void slotFixtureAdded(t_fixture_id fxi_id);

	/** Slot for fixture removals (to remove the fixture from layout) */
	void slotFixtureChanged(t_fixture_id fxi_id);

signals:
	void channelStyleChanged(MonitorProperties::ChannelStyle style);
	void valueStyleChanged(MonitorProperties::ValueStyle style);

protected:
	QScrollArea* m_scrollArea;
	QWidget* m_monitorWidget;
	MonitorLayout* m_monitorLayout;

	QList <MonitorFixture*> m_monitorFixtures;

	/*********************************************************************
	 * Timer
	 *********************************************************************/
protected:
	/** Timer that fires every 1/32th of a second and gets new values */
	void timerEvent(QTimerEvent* e);

protected:
	/** Timer ID */
	int m_timer;

	/*********************************************************************
	 * Save & Load
	 *********************************************************************/
public:
	static bool loadXML(const QDomElement* root);
	static bool saveXML(QDomDocument* doc, QDomElement* root);
};

#endif
