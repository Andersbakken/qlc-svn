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
#include <QByteArray>
#include <QMdiArea>
#include <QToolBar>
#include <QAction>
#include <QFont>
#include <QIcon>
#include <QtXml>

#include "monitorfixture.h"
#include "monitorlayout.h"
#include "universearray.h"
#include "outputmap.h"
#include "monitor.h"
#include "apputil.h"
#include "app.h"
#include "doc.h"

#include "qlctypes.h"
#include "qlcfile.h"

#define SETTINGS_GEOMETRY "monitor/geometry"
#define SETTINGS_FONT "monitor/font"
#define SETTINGS_VALUESTYLE "monitor/valuestyle"
#define SETTINGS_CHANNELSTYLE "monitor/channelstyle"

extern App* _app;

Monitor* Monitor::s_instance = NULL;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Monitor::Monitor(QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f)
{
    /* Master layout for toolbar and scroll area */
    new QVBoxLayout(this);

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

    /* Load global settings */
    loadSettings();

    /* Create toolbar */
    initToolBar();

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

    saveSettings();

    /* Reset the singleton instance */
    Monitor::s_instance = NULL;
}

void Monitor::loadSettings()
{
    QSettings settings;
    QVariant var;

    // Load font
    var = settings.value(SETTINGS_FONT);
    if (var.isValid() == true)
    {
        QFont fn;
        fn.fromString(var.toString());
        if (fn != _app->font())
            m_monitorWidget->setFont(fn);
    }

    // Load channel style
    var = settings.value(SETTINGS_CHANNELSTYLE);
    if (var.isValid() == true)
        m_channelStyle = ChannelStyle(var.toInt());
    else
        m_channelStyle = DMXChannels;

    // Load value style
    var = settings.value(SETTINGS_VALUESTYLE);
    if (var.isValid() == true)
        m_valueStyle = ValueStyle(var.toInt());
    else
        m_valueStyle = DMXValues;
}

void Monitor::saveSettings()
{
    QSettings settings;
#ifdef __APPLE__
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
#else
    settings.setValue(SETTINGS_GEOMETRY, parentWidget()->saveGeometry());
#endif
    settings.setValue(SETTINGS_FONT, m_monitorWidget->font().toString());
    settings.setValue(SETTINGS_VALUESTYLE, valueStyle());
    settings.setValue(SETTINGS_CHANNELSTYLE, channelStyle());
}

void Monitor::create(QWidget* parent)
{
    QWidget* window;

    /* Must not create more than one instance */
    if (s_instance != NULL)
        return;

#ifdef __APPLE__
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

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        window->restoreGeometry(var.toByteArray());
    AppUtil::ensureWidgetIsVisible(window);
    window->show();
}

/****************************************************************************
 * Menu
 ****************************************************************************/

void Monitor::initToolBar()
{
    QActionGroup* group;
    QAction* action;
    QToolBar* toolBar = new QToolBar(this);

    /* Menu bar */
    Q_ASSERT(layout() != NULL);
    layout()->setMenuBar(toolBar);

    /* Font */
    toolBar->addAction(QIcon(":/fonts.png"), tr("Font"),
                       this, SLOT(slotChooseFont()));

    toolBar->addSeparator();

    /* Channel style */
    group = new QActionGroup(this);
    group->setExclusive(true);

    action = toolBar->addAction(tr("DMX Channels"));
    action->setToolTip(tr("Show absolute DMX channel numbers"));
    action->setCheckable(true);
    action->setData(DMXChannels);
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(slotChannelStyleTriggered()));
    toolBar->addAction(action);
    group->addAction(action);
    if (channelStyle() == DMXChannels)
        action->setChecked(true);

    action = toolBar->addAction(tr("Relative Channels"));
    action->setToolTip(tr("Show channel numbers relative to fixture"));
    action->setCheckable(true);
    action->setData(RelativeChannels);
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(slotChannelStyleTriggered()));
    toolBar->addAction(action);
    group->addAction(action);
    if (channelStyle() == RelativeChannels)
        action->setChecked(true);

    toolBar->addSeparator();

    /* Value display style */
    group = new QActionGroup(this);
    group->setExclusive(true);

    action = toolBar->addAction(tr("DMX Values"));
    action->setToolTip(tr("Show DMX values 0-255"));
    action->setCheckable(true);
    action->setData(DMXValues);
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(slotValueStyleTriggered()));
    toolBar->addAction(action);
    group->addAction(action);
    action->setChecked(true);
    if (valueStyle() == DMXValues)
        action->setChecked(true);

    action = toolBar->addAction(tr("Percent Values"));
    action->setToolTip(tr("Show percentage values 0-100%"));
    action->setCheckable(true);
    action->setData(PercentageValues);
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(slotValueStyleTriggered()));
    toolBar->addAction(action);
    group->addAction(action);
    if (valueStyle() == PercentageValues)
        action->setChecked(true);
}

void Monitor::slotChooseFont()
{
    bool ok = false;
    QFont f = QFontDialog::getFont(&ok, m_monitorWidget->font(), this);
    if (ok == true)
        m_monitorWidget->setFont(f);
}

void Monitor::slotChannelStyleTriggered()
{
    QAction* action = qobject_cast<QAction*> (QObject::sender());
    Q_ASSERT(action != NULL);

    action->setChecked(true);
    m_channelStyle = ChannelStyle(action->data().toInt());
    emit channelStyleChanged(channelStyle());
}

void Monitor::slotValueStyleTriggered()
{
    QAction* action = qobject_cast<QAction*> (QObject::sender());
    Q_ASSERT(action != NULL);

    action->setChecked(true);
    m_valueStyle = ValueStyle(action->data().toInt());
    emit valueStyleChanged(valueStyle());
}

/****************************************************************************
 * Fixture added/removed stuff
 ****************************************************************************/

void Monitor::updateFixtureLabelStyles()
{
    QListIterator <MonitorFixture*> it(m_monitorFixtures);
    while (it.hasNext() == true)
        it.next()->updateLabelStyles();
}

void Monitor::createMonitorFixture(Fixture* fxi)
{
    MonitorFixture* mof = new MonitorFixture(m_monitorWidget);
    mof->setFixture(fxi->id());
    mof->slotChannelStyleChanged(channelStyle());
    mof->slotValueStyleChanged(valueStyle());
    mof->show();

    /* Make mof listen to value & channel style changes */
    connect(this, SIGNAL(valueStyleChanged(Monitor::ValueStyle)),
            mof, SLOT(slotValueStyleChanged(Monitor::ValueStyle)));
    connect(this, SIGNAL(channelStyleChanged(Monitor::ChannelStyle)),
            mof, SLOT(slotChannelStyleChanged(Monitor::ChannelStyle)));

    m_monitorLayout->addItem(new MonitorLayoutItem(mof));
    m_monitorFixtures.append(mof);
}

void Monitor::slotDocumentChanged(Doc* doc)
{
    Q_UNUSED(doc);

    /* Since the new document specifies, whether the monitor should be
       visible or not, we can just destroy this instance. */
#ifdef __APPLE__
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

    const UniverseArray* universes = _app->outputMap()->peekUniverses();
    QList <MonitorFixture*> list = findChildren <MonitorFixture*>();
    QListIterator <MonitorFixture*> it(list);
    while (it.hasNext() == true)
        it.next()->updateValues(universes->postGMValues());
}
