/*
  Q Light Controller
  monitor.cpp

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

#include <QMdiSubWindow>
#include <QActionGroup>
#include <QFontDialog>
#include <QScrollArea>
#include <QSpacerItem>
#include <QMdiArea>
#include <QMenuBar>
#include <QAction>
#include <QFont>
#include <QMenu>
#include <QIcon>
#include <QtXml>

#include "monitorfixture.h"
#include "monitorlayout.h"
#include "outputmap.h"
#include "monitor.h"
#include "app.h"
#include "doc.h"

#include "common/qlctypes.h"
#include "common/qlcfile.h"

extern App* _app;

MonitorProperties Monitor::s_properties = MonitorProperties();
Monitor* Monitor::s_instance = NULL;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Monitor::Monitor(QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f)
{
	/* Master layout for menu and scroll area */
	new QVBoxLayout(this);

	/* Create menu bar */
	initMenu();

	/* Scroll area that contains the monitor widget */
	m_scrollArea = new QScrollArea(this);
	m_scrollArea->setWidgetResizable(true);
	layout()->addWidget(m_scrollArea);

	/* Monitor widget that contains all MonitorFixtures */
	m_monitorWidget = new QWidget(m_scrollArea);
	m_monitorWidget->setBackgroundRole(QPalette::Dark);
	m_monitorWidget->setFont(s_properties.font());
	m_monitorLayout = new MonitorLayout(m_monitorWidget);
	m_monitorLayout->setSpacing(1);
	m_monitorLayout->setMargin(1);

	/* Create a bunch of MonitorFixtures for each fixture */
	for (t_fixture_id i = 0; i < KFixtureArraySize; i++)
	{
		Fixture* fxi = _app->doc()->fixture(i);
		if (fxi != NULL)
			createMonitorFixture(fxi);
	}

	/* Show the master container widgets */
	m_scrollArea->setWidget(m_monitorWidget);
	m_monitorWidget->show();
	m_scrollArea->show();

	/* Listen to Document changes */
	connect(_app, SIGNAL(documentChanged(Doc*)),
		this, SLOT(slotDocumentChanged(Doc*)));

	/* Listen to fixture additions and changes from Doc */
	connect(_app->doc(), SIGNAL(fixtureAdded(t_fixture_id)),
		this, SLOT(slotFixtureAdded(t_fixture_id)));
	connect(_app->doc(), SIGNAL(fixtureChanged(t_fixture_id)),
		this, SLOT(slotFixtureChanged(t_fixture_id)));

	m_timer = startTimer(1000 / 50);
	QWidget::show();
}

Monitor::~Monitor()
{
	killTimer(m_timer);
	m_timer = 0;

	/* Store properties */
	s_properties.store(s_instance);

	/* Reset the singleton instance */
	Monitor::s_instance = NULL;
}

void Monitor::create(QWidget* parent)
{
	QWidget* window;

	/* Must not create more than one instance */
	if (s_instance != NULL)
		return;

#ifdef _APPLE_
	/* Create a separate window for OSX */
	s_instance = new Monitor(parent, Qt::Window);
	window = s_instance;
#else
	/* Create an MDI window for X11 & Win32 */
	QMdiArea* area = qobject_cast<QMdiArea*> (_app->centralWidget());
	Q_ASSERT(area != NULL);
	s_instance = new Monitor(parent);
	window = area->addSubWindow(s_instance);
#endif

	/* Set some common properties for the window and show it */
	window->setAttribute(Qt::WA_DeleteOnClose);
	window->setWindowIcon(QIcon(":/monitor.png"));
	window->setWindowTitle(tr("Fixture Monitor"));
	window->setContextMenuPolicy(Qt::CustomContextMenu);
	window->setWindowState(s_properties.state());
	window->setGeometry(s_properties.x(), s_properties.y(),
			    s_properties.width(), s_properties.height());
	window->show();
}

/****************************************************************************
 * Menu
 ****************************************************************************/

void Monitor::initMenu()
{
	QActionGroup* group;
	QAction* action;
	QMenu* displayMenu;
	QMenu* menu;

	/* Menu bar */
	Q_ASSERT(layout() != NULL);
	layout()->setMenuBar(new QMenuBar(this));

	/* Display menu */
	displayMenu = new QMenu(layout()->menuBar());
	qobject_cast <QMenuBar*> (layout()->menuBar())->addMenu(displayMenu);
	displayMenu->setTitle(tr("Display"));
	displayMenu->addAction(QIcon(":/fonts.png"), tr("Font"),
			       this, SLOT(slotChooseFont()));
	displayMenu->addSeparator();

	/* Channel display style */
	menu = new QMenu(displayMenu);
	displayMenu->addMenu(menu);
	menu->setTitle("Channel style");
	connect(menu, SIGNAL(triggered(QAction*)),
		this, SLOT(slotChannelStyleTriggered(QAction*)));

	/* Style group and actions */
	group = new QActionGroup(this);
	group->setExclusive(true);

	action = menu->addAction(tr("Relative to fixture"));
	action->setCheckable(true);
	action->setData(MonitorProperties::RelativeChannels);
	group->addAction(action);
	if (s_properties.channelStyle() == MonitorProperties::RelativeChannels)
		action->setChecked(true);

	action = menu->addAction(tr("Absolute DMX"));
	action->setCheckable(true);
	action->setData(MonitorProperties::DMXChannels);
	group->addAction(action);
	if (s_properties.channelStyle() == MonitorProperties::DMXChannels)
		action->setChecked(true);

	/* Value display style */
	menu = new QMenu(displayMenu);
	displayMenu->addMenu(menu);
	menu->setTitle("Value style");
	connect(menu, SIGNAL(triggered(QAction*)),
		this, SLOT(slotValueStyleTriggered(QAction*)));

	/* Style group and actions */
	group = new QActionGroup(this);
	group->setExclusive(true);

	action = menu->addAction(tr("Absolute DMX"));
	action->setCheckable(true);
	action->setData(MonitorProperties::DMXValues);
	group->addAction(action);
	action->setChecked(true);
	if (s_properties.valueStyle() == MonitorProperties::DMXValues)
		action->setChecked(true);

	action = menu->addAction(tr("Percentage"));
	action->setCheckable(true);
	action->setData(MonitorProperties::PercentageValues);
	group->addAction(action);
	if (s_properties.valueStyle() == MonitorProperties::PercentageValues)
		action->setChecked(true);
}

void Monitor::slotChooseFont()
{
	bool ok = false;
	QFont f = QFontDialog::getFont(&ok, font(), this);
	if (ok == true)
	{
		s_properties.m_font = f.toString();
		m_monitorWidget->setFont(f);
	}
}

void Monitor::slotChannelStyleTriggered(QAction* action)
{
	Q_ASSERT(action != NULL);
	action->setChecked(true);
	s_properties.m_channelStyle =
		MonitorProperties::ChannelStyle(action->data().toInt());
	emit channelStyleChanged(s_properties.m_channelStyle);
}

void Monitor::slotValueStyleTriggered(QAction* action)
{
	Q_ASSERT(action != NULL);
	action->setChecked(true);
	s_properties.m_valueStyle =
		MonitorProperties::ValueStyle(action->data().toInt());
	emit valueStyleChanged(s_properties.m_valueStyle);
}

/****************************************************************************
 * Fixture added/removed stuff
 ****************************************************************************/

void Monitor::createMonitorFixture(Fixture* fxi)
{
	MonitorFixture* mof = new MonitorFixture(m_monitorWidget);
	mof->setFixture(fxi->id());
	mof->show();

	/* Make mof listen to value & channel style changes */
	connect(this, SIGNAL(valueStyleChanged(MonitorProperties::ValueStyle)),
		mof, SLOT(slotValueStyleChanged(MonitorProperties::ValueStyle)));
	connect(this, SIGNAL(channelStyleChanged(MonitorProperties::ChannelStyle)),
		mof, SLOT(slotChannelStyleChanged(MonitorProperties::ChannelStyle)));

	m_monitorLayout->addItem(new MonitorLayoutItem(mof));
	m_monitorFixtures.append(mof);
}

void Monitor::slotDocumentChanged(Doc* doc)
{
	Q_UNUSED(doc);

	/* Since the new document specifies, whether the monitor should be
	   visible or not, we can just destroy this instance. */
#ifdef _APPLE_
	deleteLater();
#else
	parentWidget()->deleteLater();
#endif
}

void Monitor::slotFixtureAdded(t_fixture_id fxi_id)
{
	Fixture* fxi = _app->doc()->fixture(fxi_id);
	if (fxi != NULL)
		createMonitorFixture(fxi);
}

void Monitor::slotFixtureChanged(t_fixture_id fxi_id)
{
	Q_UNUSED(fxi_id);

	m_monitorLayout->sort();
	m_monitorWidget->updateGeometry();
}

/****************************************************************************
 * Timer
 ****************************************************************************/

void Monitor::timerEvent(QTimerEvent* e)
{
	Q_UNUSED(e);

	QList <MonitorFixture*> list = findChildren <MonitorFixture*>();
	QListIterator <MonitorFixture*> it(list);
	while (it.hasNext() == true)
		it.next()->updateValues();
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool Monitor::loadXML(QDomDocument* doc, QDomElement* root)
{
	Q_UNUSED(doc);

	if (root->tagName() != KXMLQLCMonitor)
	{
		qWarning("Monitor node not found!");
		return false;
	}

	s_properties.loadXML(doc, root);
	if (s_properties.visible() == true)
		create(_app);

	return true;
}

bool Monitor::saveXML(QDomDocument* doc, QDomElement* root)
{
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	/* Store latest properties */
	if (s_instance != NULL)
		s_properties.store(s_instance);

	/* Save the properties to file */
	s_properties.saveXML(doc, root);

	return false;
}

/****************************************************************************
 * Monitor Properties
 ****************************************************************************/

MonitorProperties::MonitorProperties() : QLCWidgetProperties()
{
	m_width = 640;
	m_height = 480;

	m_channelStyle = DMXChannels;
	m_valueStyle = DMXValues;
}

MonitorProperties::~MonitorProperties()
{
}

/****************************************************************************
 * Channel & Value styles
 ****************************************************************************/

QString MonitorProperties::channelStyleToString(ChannelStyle style)
{
	switch (style)
	{
	default:
	case DMXChannels:
		return KXMLQLCMonitorChannelStyleDMX;
	case RelativeChannels:
		return KXMLQLCMonitorChannelStyleRelative;
	}
}

MonitorProperties::ChannelStyle MonitorProperties::stringToChannelStyle(
							const QString& str)
{
	if (str == QString(KXMLQLCMonitorChannelStyleRelative))
		return RelativeChannels;
	else
		return DMXChannels;
}

QString MonitorProperties::valueStyleToString(ValueStyle style)
{
	switch (style)
	{
	default:
	case DMXValues:
		return KXMLQLCMonitorValueStyleDMX;
	case PercentageValues:
		return KXMLQLCMonitorValueStylePercentage;
	}
}

MonitorProperties::ValueStyle MonitorProperties::stringToValueStyle(
							const QString& str)
{
	if (str == QString(KXMLQLCMonitorValueStylePercentage))
		return PercentageValues;
	else
		return DMXValues;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

void MonitorProperties::store(Monitor* monitor)
{
#ifdef _APPLE_
	QLCWidgetProperties::store(monitor);
#else
	QLCWidgetProperties::store(monitor->parentWidget());
#endif
}

bool MonitorProperties::loadXML(QDomDocument* doc, QDomElement* root)
{
	QDomElement tag;
	QDomNode node;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCMonitor)
	{
		qWarning("Monitor node not found!");
		return false;
	}

	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();

		if (tag.tagName() == KXMLQLCMonitorFont)
			m_font = tag.text();
		else if (tag.tagName() == KXMLQLCMonitorValueStyle)
			m_valueStyle = stringToValueStyle(tag.text());
		else if (tag.tagName() == KXMLQLCMonitorChannelStyle)
			m_channelStyle = stringToChannelStyle(tag.text());
		else if (tag.tagName() == KXMLQLCWidgetProperties)
			QLCWidgetProperties::loadXML(doc, &tag);
		else
			qDebug() << "Unknown monitor tag:" << tag.tagName();

		node = node.nextSibling();
	}

	return true;
}

bool MonitorProperties::saveXML(QDomDocument* doc, QDomElement* root)
{
	QDomElement prop_root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	/* Monitor entry */
	prop_root = doc->createElement(KXMLQLCMonitor);
	root->appendChild(prop_root);

	/* Font */
	tag = doc->createElement(KXMLQLCMonitorFont);
	prop_root.appendChild(tag);
	text = doc->createTextNode(m_font);
	tag.appendChild(text);

	/* Value style */
	tag = doc->createElement(KXMLQLCMonitorValueStyle);
	prop_root.appendChild(tag);
	text = doc->createTextNode(valueStyleToString(m_valueStyle));
	tag.appendChild(text);

	/* Channel style */
	tag = doc->createElement(KXMLQLCMonitorChannelStyle);
	prop_root.appendChild(tag);
	text = doc->createTextNode(channelStyleToString(m_channelStyle));
	tag.appendChild(text);

	/* Window state */
	return QLCWidgetProperties::saveXML(doc, &prop_root);
}
