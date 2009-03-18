/*
  Q Light Controller
  busmanager.cpp

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
#include <QInputDialog>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QStringList>
#include <QHeaderView>
#include <QSettings>
#include <QMdiArea>
#include <QToolBar>
#include <QAction>
#include <QString>
#include <QIcon>

#include "busmanager.h"
#include "app.h"
#include "bus.h"

#define KColumnID   0
#define KColumnName 1

extern App* _app;

BusManager* BusManager::s_instance = NULL;

/****************************************************************************
 * Initialization
 ****************************************************************************/

BusManager::BusManager(QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f)
{
	Q_ASSERT(parent != NULL);

	/* Create a new layout for this widget */
	new QVBoxLayout(this);

	/* Toolbar */
	m_toolbar = new QToolBar(tr("Bus Manager"), this);
	m_toolbar->addAction(QIcon(":/edit.png"), tr("Edit bus name"),
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
	columns << tr("Bus ID") << tr("Name");
	m_tree->setHeaderLabels(columns);

	fillTree();
}

BusManager::~BusManager()
{
	QSettings settings;
	QRect rect;

#ifdef __APPLE__
	rect = this->rect();
#else
	rect = parentWidget()->rect();
#endif
	settings.setValue("busmanager/width", rect.width());
	settings.setValue("busmanager/height", rect.height());

	/* Reset singleton instance */
	s_instance = NULL;
}

void BusManager::create(QWidget* parent)
{
	QWidget* window;

	/* Must not create more than one instance */
	if (s_instance != NULL)
		return;

#ifdef __APPLE__
	/* Create a separate window for OSX */
	s_instance = new BusManager(parent, Qt::Window);
	window = s_instance;
#else
	/* Create an MDI window for X11 & Win32 */
	QMdiArea* area = qobject_cast<QMdiArea*> (_app->centralWidget());
	Q_ASSERT(area != NULL);
	s_instance = new BusManager(parent);
	window = area->addSubWindow(s_instance);
#endif

	/* Set some common properties for the window and show it */
	window->setAttribute(Qt::WA_DeleteOnClose);
	window->setWindowIcon(QIcon(":/bus.png"));
	window->setWindowTitle(tr("Bus Manager"));
	window->setContextMenuPolicy(Qt::CustomContextMenu);
	window->show();

	connect(_app, SIGNAL(modeChanged(App::Mode)),
		s_instance, SLOT(slotAppModeChanged(App::Mode)));

	QSettings settings;
	QVariant w = settings.value("busmanager/width");
	QVariant h = settings.value("busmanager/height");
	if (w.isValid() == true && h.isValid() == true)
		window->resize(w.toInt(), h.toInt());
	else
		window->resize(300, 400);
}

/****************************************************************************
 * App mode
 ****************************************************************************/

void BusManager::slotAppModeChanged(App::Mode mode)
{
	/* Destroy this when going to operate mode */
	if (mode == App::Operate)
#ifdef __APPLE__
		deleteLater(); /* Destroy this */
#else
		parent()->deleteLater(); /* Destroy mdi subwindow */
#endif
}

/****************************************************************************
 * Toolbar
 ****************************************************************************/

void BusManager::slotEditClicked()
{
	QTreeWidgetItem* item = m_tree->currentItem();

	if (item != NULL)
	{
		QString label;
		QString name;
		t_bus_id id;
		bool ok = false;

		id = item->text(KColumnID).toInt() - 1;
		label = tr("Bus #%1 name:").arg(id + 1);
		name = QInputDialog::getText(this, tr("Rename bus"), label,
					     QLineEdit::Normal, Bus::name(id),
					     &ok);
		if (ok == true)
		{
			Bus::setName(id, name);
			item->setText(KColumnName, name);
		}
	}
}

/****************************************************************************
 * Tree widget
 ****************************************************************************/

void BusManager::fillTree()
{
	/* Fill all buses to tree */
	for (t_bus_id id = KBusIDMin; id < KBusCount; id++)
	{
		QTreeWidgetItem* item;
		item = new QTreeWidgetItem(m_tree);
		item->setText(KColumnID, QString("%1").arg(id + 1));
		item->setText(KColumnName, Bus::name(id));
		item->setFlags(item->flags() | Qt::ItemIsEditable);
	}

	/* Set first bus selected */
	m_tree->setCurrentItem(m_tree->topLevelItem(0));

	/* Listen to in-place item renaming */
	connect(m_tree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
		this, SLOT(slotItemChanged(QTreeWidgetItem*,int)));
}

void BusManager::slotItemChanged(QTreeWidgetItem* item, int column)
{
	int index;

	Q_ASSERT(item != NULL);

	index = m_tree->indexOfTopLevelItem(item);
	if (column == KColumnID) /* Reject ID column edits */
		item->setText(KColumnID, QString("%1").arg(index + 1));
	else /* Change bus name */
		Bus::setName(index, item->text(KColumnName));
}
