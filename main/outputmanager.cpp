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
#include <QTreeWidget>
#include <QStringList>
#include <QHeaderView>
#include <QAction>
#include <QMenu>

#include "common/qlcoutplugin.h"
#include "outputpatcheditor.h"
#include "outputmanager.h"
#include "outputpatch.h"
#include "outputmap.h"
#include "app.h"

#define KColumnUniverse   0
#define KColumnPlugin     1
#define KColumnOutputName 2
#define KColumnOutput     3

extern App* _app;

OutputManager::OutputManager(QWidget* parent) : QWidget(parent)
{
	Q_ASSERT(_app->outputMap() != NULL);

	setupUi(this);

	m_tree->header()->setResizeMode(QHeaderView::ResizeToContents);

	connect(m_edit, SIGNAL(clicked()), this, SLOT(slotEditClicked()));
	connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotEditClicked()));

	update();
}

OutputManager::~OutputManager()
{
}

void OutputManager::update()
{
	m_tree->clear();
	for (int i = 0; i < KUniverseCount; i++)
	{
		QTreeWidgetItem* item;
		OutputPatch* outputPatch;
		QString str;

		outputPatch = _app->outputMap()->patch(i);
		Q_ASSERT(outputPatch != NULL);

		item = new QTreeWidgetItem(m_tree);
		item->setText(KColumnUniverse, str.setNum(i + 1));
		item->setText(KColumnPlugin, outputPatch->plugin()->name());
		item->setText(KColumnOutputName,
			      outputPatch->plugin()->outputs()
						[outputPatch->output()]);
		item->setText(KColumnOutput,
			      str.setNum(outputPatch->output() + 1));
	}
}

/*****************************************************************************
 * Tree widget
 *****************************************************************************/

void OutputManager::slotEditClicked()
{
	QTreeWidgetItem* item;
	int universe;
	QString str;
	int output;

	item = m_tree->currentItem();
	if (item == NULL)
		return;

	universe = item->text(KColumnUniverse).toInt() - 1;
	str = item->text(KColumnPlugin);
	output = item->text(KColumnOutput).remove("Output").toInt() - 1;

	OutputPatchEditor ope(this, universe, str, output);
	if (ope.exec() == QDialog::Accepted)
	{
		if (ope.output() != KOutputInvalid &&
		    ope.pluginName().isEmpty() == false)
		{
			item->setText(KColumnUniverse,
				      str.setNum(universe + 1));
			item->setText(KColumnPlugin, ope.pluginName());
			item->setText(KColumnOutputName, ope.outputName());
			item->setText(KColumnOutput,
				      str.setNum(ope.output() + 1));

			_app->outputMap()->setPatch(universe, ope.pluginName(),
						    ope.output());
		}
	}
}
