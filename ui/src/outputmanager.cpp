/*
  Q Light Controller
  outputmanager.cpp

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
#include <QStringList>
#include <QHeaderView>
#include <QSettings>
#include <QMdiArea>
#include <QToolBar>
#include <QAction>
#include <QMenu>

#include "qlcoutplugin.h"
#include "outputpatcheditor.h"
#include "outputmanager.h"
#include "outputpatch.h"
#include "outputmap.h"
#include "apputil.h"
#include "app.h"

#define SETTINGS_GEOMETRY "outputmanager/geometry"

#define KColumnUniverse   0
#define KColumnPlugin     1
#define KColumnOutputName 2
#define KColumnOutput     3

extern App* _app;

OutputManager* OutputManager::s_instance = NULL;

/****************************************************************************
 * Initialization
 ****************************************************************************/

OutputManager::OutputManager(QWidget* parent, Qt::WindowFlags flags)
        : QWidget(parent, flags)
{
    /* Create a new layout for this widget */
    new QVBoxLayout(this);

    /* Toolbar */
    m_toolbar = new QToolBar(tr("Output Manager"), this);
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
    columns << tr("Universe") << tr("Plugin") << tr("Output");
    m_tree->setHeaderLabels(columns);

    connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotEditClicked()));

    /* Listen to document changes */
    connect(_app, SIGNAL(documentChanged(Doc*)),
            this, SLOT(slotDocumentChanged(Doc*)));
    /* Use the initial document */
    slotDocumentChanged(_app->doc());

    connect(_app->outputMap(), SIGNAL(pluginConfigurationChanged(const QString&)),
            this, SLOT(slotPluginConfigurationChanged()));
}

OutputManager::~OutputManager()
{
    QSettings settings;
#ifdef __APPLE__
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
#else
    settings.setValue(SETTINGS_GEOMETRY, parentWidget()->saveGeometry());
#endif
    OutputManager::s_instance = NULL;
}

void OutputManager::create(QWidget* parent)
{
    QWidget* window;

    /* Must not create more than one instance */
    if (s_instance != NULL)
        return;

#ifdef __APPLE__
    /* Create a separate window for OSX */
    s_instance = new OutputManager(parent, Qt::Window);
    window = s_instance;
#else
    /* Create an MDI window for X11 & Win32 */
    QMdiArea* area = qobject_cast<QMdiArea*> (_app->centralWidget());
    Q_ASSERT(area != NULL);
    s_instance = new OutputManager(parent);
    window = area->addSubWindow(s_instance);
#endif

    /* Set some common properties for the window and show it */
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setWindowIcon(QIcon(":/output.png"));
    window->setWindowTitle(tr("Output Manager"));
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
        QVariant w = settings.value("outputmanager/width");
        QVariant h = settings.value("outputmanager/height");
        if (w.isValid() == true && h.isValid() == true)
            window->resize(w.toInt(), h.toInt());
        else
            window->resize(700, 300);
    }
}

void OutputManager::slotModeChanged(Doc::Mode mode)
{
    /* Close this when entering operate mode */
    if (mode == Doc::Operate)
#ifdef __APPLE__
        deleteLater();
#else
        parent()->deleteLater();
#endif
}

void OutputManager::slotDocumentChanged(Doc* doc)
{
    connect(doc, SIGNAL(modeChanged(Doc::Mode)),
            this, SLOT(slotModeChanged(Doc::Mode)));
    updateTree();
}

/*****************************************************************************
 * Tree widget
 *****************************************************************************/

void OutputManager::updateTree()
{
    m_tree->clear();
    for (quint32 uni = 0; uni < KUniverseCount; uni++)
    {
        OutputPatch* op = _app->outputMap()->patch(uni);
        updateItem(new QTreeWidgetItem(m_tree), op, uni);
    }
}

void OutputManager::updateItem(QTreeWidgetItem* item, OutputPatch* op,
                               int universe)
{
    Q_ASSERT(item != NULL);
    Q_ASSERT(op != NULL);

    item->setText(KColumnUniverse, QString("%1").arg(universe + 1));
    item->setText(KColumnPlugin, op->pluginName());
    item->setText(KColumnOutputName, op->outputName());
    item->setText(KColumnOutput, QString("%1").arg(op->output() + 1));
}

void OutputManager::slotPluginConfigurationChanged()
{
    updateTree();
}

/****************************************************************************
 * Toolbar
 ****************************************************************************/

void OutputManager::slotEditClicked()
{
    QTreeWidgetItem* item;
    OutputPatch* patch;
    int universe;

    item = m_tree->currentItem();
    if (item == NULL)
        return;

    universe = item->text(KColumnUniverse).toInt() - 1;
    patch = _app->outputMap()->patch(universe);
    Q_ASSERT(patch != NULL);

    OutputPatchEditor ope(this, universe, patch);
    if (ope.exec() == QDialog::Accepted)
        updateItem(item, patch, universe);
}
