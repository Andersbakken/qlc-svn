/*
  Q Light Controller
  inputmanager.cpp

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

#include <QTreeWidgetItem>
#include <QMdiSubWindow>
#include <QTreeWidget>
#include <QHeaderView>
#include <QStringList>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QSettings>
#include <QMdiArea>
#include <QToolBar>
#include <QAction>
#include <QTimer>
#include <QDebug>
#include <QIcon>

#include "qlcinplugin.h"

#include "inputpatcheditor.h"
#include "inputmanager.h"
#include "inputpatch.h"
#include "inputmap.h"
#include "apputil.h"
#include "app.h"

#define SETTINGS_GEOMETRY "inputmanager/geometry"

#define KColumnUniverse 0
#define KColumnPlugin   1
#define KColumnInput    2
#define KColumnProfile  3
#define KColumnEditor   4
#define KColumnInputNum 5

extern App* _app;

InputManager* InputManager::s_instance = NULL;

/****************************************************************************
 * Initialization
 ****************************************************************************/

InputManager::InputManager(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
{
    /* Create a new layout for this widget */
    new QVBoxLayout(this);

    /* Toolbar */
    m_toolbar = new QToolBar(tr("Input Manager"), this);
    m_toolbar->addAction(QIcon(":/edit.png"), tr("Edit Mapping"),
                         this, SLOT(slotEditClicked()));
    layout()->addWidget(m_toolbar);

    /* Tree */
    m_tree = new QTreeWidget(this);
    layout()->addWidget(m_tree);
    m_tree->setRootIsDecorated(false);
    m_tree->setItemsExpandable(false);
    m_tree->setSortingEnabled(false);
    m_tree->setAllColumnsShowFocus(true);
    m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);

    QStringList columns;
    columns << tr("Universe") << tr("Plugin") << tr("Input") << tr("Profile")
            << tr("Editor universe");
    m_tree->setHeaderLabels(columns);

    connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotEditClicked()));

    /* Timer that clears the input data icon after a while */
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimerTimeout()));

    /* Listen to input map's input data signals */
    connect(_app->inputMap(), SIGNAL(inputValueChanged(quint32,quint32,uchar)),
            this, SLOT(slotInputValueChanged(quint32,quint32,uchar)));

    /* Listen to document changes */
    connect(_app, SIGNAL(documentChanged(Doc*)),
            this, SLOT(slotDocumentChanged(Doc*)));
    /* Use the initial document */
    slotDocumentChanged(_app->doc());

    /* Listen to plugin configuration changes */
    connect(_app->inputMap(), SIGNAL(pluginConfigurationChanged(const QString&)),
            this, SLOT(slotPluginConfigurationChanged()));
}

InputManager::~InputManager()
{
    QSettings settings;
#ifdef __APPLE__
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
#else
    settings.setValue(SETTINGS_GEOMETRY, parentWidget()->saveGeometry());
#endif
    InputManager::s_instance = NULL;
}

void InputManager::create(QWidget* parent)
{
    QWidget* window;

    /* Must not create more than one instance */
    if (s_instance != NULL)
        return;

#ifdef __APPLE__
    /* Create a separate window for OSX */
    s_instance = new InputManager(parent, Qt::Window);
    window = s_instance;
#else
    /* Create an MDI window for X11 & Win32 */
    QMdiArea* area = qobject_cast<QMdiArea*> (_app->centralWidget());
    Q_ASSERT(area != NULL);
    s_instance = new InputManager(parent);
    window = area->addSubWindow(s_instance);
#endif

    /* Set some common properties for the window and show it */
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setWindowIcon(QIcon(":/input.png"));
    window->setWindowTitle(tr("Input Manager"));
    window->setContextMenuPolicy(Qt::CustomContextMenu);
    window->show();

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
    {
        window->restoreGeometry(var.toByteArray());
        AppUtil::ensureWidgetIsVisible(window);
    }
    else
    {
        QVariant w = settings.value("inputmanager/width");
        QVariant h = settings.value("inputmanager/height");
        if (w.isValid() == true && h.isValid() == true)
            window->resize(w.toInt(), h.toInt());
        else
            window->resize(700, 300);
    }
}

void InputManager::slotModeChanged(Doc::Mode mode)
{
    /* Close this when entering operate mode */
    if (mode == Doc::Operate)
#ifdef __APPLE__
        deleteLater();
#else
        parent()->deleteLater();
#endif
}

void InputManager::slotDocumentChanged(Doc* doc)
{
    connect(doc, SIGNAL(modeChanged(Doc::Mode)),
            this, SLOT(slotModeChanged(Doc::Mode)));
    updateTree();
}

/*****************************************************************************
 * Tree widget
 *****************************************************************************/

void InputManager::updateTree()
{
    m_tree->clear();
    for (quint32 i = 0; i < _app->inputMap()->universes(); i++)
    {
        InputPatch* inputPatch = _app->inputMap()->patch(i);
        Q_ASSERT(inputPatch != NULL);

        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
        updateItem(item, inputPatch, i);
    }
}

void InputManager::updateItem(QTreeWidgetItem* item, InputPatch* ip,
                              quint32 universe)
{
    Q_ASSERT(item != NULL);
    Q_ASSERT(ip != NULL);

    item->setText(KColumnUniverse, QString("%1").arg(universe + 1));
    item->setText(KColumnPlugin, ip->pluginName());
    item->setText(KColumnInput, ip->inputName());
    item->setText(KColumnProfile, ip->profileName());
    if (_app->inputMap()->editorUniverse() == universe)
    {
        item->setCheckState(KColumnEditor, Qt::Checked);
        item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
    }
    item->setText(KColumnInputNum, QString("%1").arg(ip->input() + 1));
}

void InputManager::slotPluginConfigurationChanged()
{
    updateTree();
}

void InputManager::slotInputValueChanged(quint32 universe, quint32 channel,
                                         uchar value)
{
    Q_UNUSED(channel);
    Q_UNUSED(value);

    QTreeWidgetItem* item = m_tree->topLevelItem(universe);
    if (item == NULL)
        return;

    /* Show an icon on a universe row that received input data */
    QIcon icon(":/input.png");
    item->setIcon(KColumnUniverse, icon);

    /* Restart the timer */
    m_timer->start(250);
}

void InputManager::slotTimerTimeout()
{
    QTreeWidgetItemIterator it(m_tree);
    while (*it != NULL)
    {
        (*it)->setIcon(KColumnUniverse, QIcon());
        ++it;
    }
}

/****************************************************************************
 * Toolbar
 ****************************************************************************/

void InputManager::slotEditClicked()
{
    QTreeWidgetItem* item = m_tree->currentItem();
    if (item == NULL)
        return;

    quint32 universe = item->text(KColumnUniverse).toInt() - 1;
    InputPatch* patch = _app->inputMap()->patch(universe);
    Q_ASSERT(patch != NULL);

    InputPatchEditor ipe(this, universe, patch);
    if (ipe.exec() == QDialog::Accepted)
        updateTree();
}
