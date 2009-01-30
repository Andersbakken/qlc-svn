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

#include <QActionGroup>
#include <QFontDialog>
#include <QScrollArea>
#include <QSpacerItem>
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

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Monitor::Monitor(QWidget* parent) : QWidget(parent)
{
	m_channelStyle = MonitorFixture::RelativeChannels;
	m_valueStyle = MonitorFixture::DMXValues;

	init();

	_app->outputMap()->collectValues(true);
}

Monitor::~Monitor()
{
	_app->outputMap()->collectValues(false);
}

void Monitor::init()
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
	slotDocumentChanged(_app->doc());
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
	action->setData(MonitorFixture::RelativeChannels);
	group->addAction(action);
	action->setChecked(true);

	action = menu->addAction(tr("Absolute DMX"));
	action->setCheckable(true);
	action->setData(MonitorFixture::DMXChannels);
	group->addAction(action);

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
	action->setData(MonitorFixture::DMXValues);
	group->addAction(action);
	action->setChecked(true);

	action = menu->addAction(tr("Percentage"));
	action->setCheckable(true);
	action->setData(MonitorFixture::PercentageValues);
	group->addAction(action);
}

void Monitor::slotChooseFont()
{
	bool ok = false;
	QFont f = QFontDialog::getFont(&ok, font(), this);
	if (ok == true)
		setFont(f);
}

void Monitor::slotChannelStyleTriggered(QAction* action)
{
	Q_ASSERT(action != NULL);
	action->setChecked(true);
	m_channelStyle = MonitorFixture::ChannelStyle(action->data().toInt());
	emit channelStyleChanged(m_channelStyle);
}

void Monitor::slotValueStyleTriggered(QAction* action)
{
	Q_ASSERT(action != NULL);
	action->setChecked(true);
	m_valueStyle = MonitorFixture::ValueStyle(action->data().toInt());
	emit valueStyleChanged(m_valueStyle);
}

/****************************************************************************
 * Fixture added/removed stuff
 ****************************************************************************/

void Monitor::createMonitorFixture(Fixture* fxi)
{
	MonitorFixture* mof;

	mof = new MonitorFixture(m_monitorWidget);
	mof->setFixture(fxi->id());
	mof->slotSetChannelStyle(m_channelStyle);
	mof->slotSetValueStyle(m_valueStyle);
	mof->show();

	/* Listen to value & channel style changes */
	connect(this, SIGNAL(valueStyleChanged(MonitorFixture::ValueStyle)),
		mof, SLOT(slotSetValueStyle(MonitorFixture::ValueStyle)));
	connect(this, SIGNAL(channelStyleChanged(MonitorFixture::ChannelStyle)),
		mof, SLOT(slotSetChannelStyle(MonitorFixture::ChannelStyle)));

	m_monitorLayout->addItem(new MonitorLayoutItem(mof));
	m_monitorFixtures.append(mof);
}

void Monitor::slotDocumentChanged(Doc* doc)
{
	Q_ASSERT(doc != NULL);

	/* Listen to fixture additions and changes from the new Doc */
	connect(doc, SIGNAL(fixtureAdded(t_fixture_id)),
		this, SLOT(slotFixtureAdded(t_fixture_id)));
	connect(doc, SIGNAL(fixtureChanged(t_fixture_id)),
		this, SLOT(slotFixtureChanged(t_fixture_id)));
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
 * Load/save settings
 ****************************************************************************/

void Monitor::loader(QDomDocument*, QDomElement*)
{
	// TODO
	//_app->createMonitor();
	//_app->monitor()->loadXML(doc, root);
}

bool Monitor::loadXML(QDomDocument*, QDomElement* root)
{
	bool vis = false;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;

	QDomElement tag;
	QDomNode node;
	QFont font;

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
		if (tag.tagName() == KXMLQLCWindowState)
			QLCFile::loadXMLWindowState(&tag, &x, &y, &w, &h, &vis);
		else if (tag.tagName() == KXMLQLCMonitorFont)
			font.fromString(tag.text());
		else
			qDebug() << "Unknown monitor tag:" << tag.tagName();

		node = node.nextSibling();
	}

	setFont(font);

	hide();
	setGeometry(x, y, w, h);
	if (vis == false)
		showMinimized();
	else
		showNormal();

	return true;
}

bool Monitor::saveXML(QDomDocument* doc, QDomElement* fxi_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(fxi_root != NULL);

	/* Fixture Console entry */
	root = doc->createElement(KXMLQLCMonitor);
	fxi_root->appendChild(root);

	/* Font */
	tag = doc->createElement(KXMLQLCMonitorFont);
	root.appendChild(tag);
	text = doc->createTextNode(font().toString());
	tag.appendChild(text);

	/* Save window state. parentWidget() should be used for all
	   widgets within the workspace. */
	return QLCFile::saveXMLWindowState(doc, &root, parentWidget());
}
