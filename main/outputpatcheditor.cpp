/*
  Q Light Controller
  outputpatcheditor.cpp

  Copyright (C) Heikki Junnila

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
#include <QToolButton>
#include <QGroupBox>
#include <QVariant>
#include <QDebug>
#include <QIcon>

#include "outputpatcheditor.h"
#include "outputmap.h"
#include "app.h"

#define KColumnName   0
#define KColumnOutput 1

extern App* _app;

OutputPatchEditor::OutputPatchEditor(QWidget* parent, int universe,
				     const QString& pluginName,
				     t_output output) : QDialog(parent)
{
	/* OutputMap */
	Q_ASSERT(_app->outputMap() != NULL);

	/* Setup UI controls */
	setupUi(this);
	connect(m_tree, SIGNAL(currentItemChanged(QTreeWidgetItem*,
						  QTreeWidgetItem*)),
		this, SLOT(slotCurrentItemChanged(QTreeWidgetItem*)));
	connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(accept()));

	/* Universe */
	Q_ASSERT(universe < _app->outputMap()->universes());
	m_universe = universe;
	setWindowTitle(tr("Mapping properties for output universe %1")
			.arg(universe + 1));

	/* Selected plugin & output */
	m_pluginName = pluginName;
	m_output = output;

	fillTree();
}

OutputPatchEditor::~OutputPatchEditor()
{
}

void OutputPatchEditor::fillTree()
{
	QTreeWidgetItem* pitem;
	QTreeWidgetItem* oitem;
	QString pluginName;
	int i;
	
	m_tree->clear();
	
	/* Go thru available plugins and put them as the tree's root nodes. */
	QStringListIterator pit(_app->outputMap()->pluginNames());
	while (pit.hasNext() == true)
	{
		i = 0;

		pluginName = pit.next();
		pitem = new QTreeWidgetItem(m_tree);
		pitem->setText(KColumnName, pluginName);
		pitem->setText(KColumnOutput,
			       QString("%1").arg(KOutputInvalid));

		/* Go thru available outputs provided by each plugin and put
		   them as their parent plugin's leaf nodes. */
		QStringListIterator iit(_app->outputMap()->pluginOutputs(
						pluginName));
		while (iit.hasNext() == true)
		{
			oitem = new QTreeWidgetItem(pitem);
			oitem->setText(KColumnName, iit.next());
			oitem->setText(KColumnOutput, QString("%1").arg(i));
			
			/* Select that plugin's line that is currently mapped */
			if (m_pluginName == pluginName && m_output == i)
				m_tree->setCurrentItem(oitem);

			i++;
		}
		
		/* If no outputs were appended to the plugin node, put a
		   "Nothing" node there. */
		if (i == 0)
		{
			oitem = new QTreeWidgetItem(pitem);
			oitem->setText(KColumnName, KOutputNone);
			oitem->setText(KColumnOutput,
				       QString("%1").arg(KOutputInvalid));
			oitem->setFlags(oitem->flags() & ~Qt::ItemIsEnabled);
			oitem->setFlags(oitem->flags() & ~Qt::ItemIsSelectable);
		}
	}
}

void OutputPatchEditor::updateOutputInfo()
{
	QLCOutPlugin* plugin;

	plugin = _app->outputMap()->plugin(m_pluginName);
	if (plugin == NULL)
	{
		/* No plugin selected */
	}
	else
	{
		qDebug() << plugin->infoText(m_output);
	}
}

void OutputPatchEditor::slotCurrentItemChanged(QTreeWidgetItem* item)
{
	if (item == NULL)
	{
		m_output = KOutputInvalid;
		m_outputName = QString::null;
		m_pluginName = QString::null;
	}
	else
	{
		m_output = item->text(KColumnOutput).toInt();
		if (m_output != KOutputInvalid)
		{
			m_outputName = item->text(KColumnName);
			m_pluginName = item->parent()->text(KColumnName);
		}
		else
		{
			m_outputName = QString::null;
			m_pluginName = QString::null;
		}
	}

	updateOutputInfo();
}
