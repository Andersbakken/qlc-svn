/*
  Q Light Controller
  dmxmapeditor.cpp
  
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

#include <qlayout.h>
#include <qcombobox.h>
#include <qstringlist.h>
#include <qlistview.h>
#include <qpopupmenu.h>

#include "common/outputplugin.h"

#include "dmxmap.h"
#include "dmxmapeditor.h"
#include "dmxpatcheditor.h"

#define KColumnUniverse 0
#define KColumnPlugin   1
#define KColumnOutput   2

DMXMapEditor::DMXMapEditor(QWidget* parent, DMXMap* dmxMap)
	: UI_DMXMapEditor(parent, "Output Mapper", true),
	  m_dmxMap(dmxMap)
{
}

DMXMapEditor::~DMXMapEditor()
{
}

void DMXMapEditor::init()
{
	QListViewItem* item = NULL;
	DMXPatch* dmxPatch = NULL;
	int i = 0;
	QString str;

	m_pluginList = m_dmxMap->pluginNames();

	/* Clear the mapping list first */
	m_listView->clear();

	for (i = 0; i < KUniverseCount; i++)
	{
		dmxPatch = m_dmxMap->patch(i);
		Q_ASSERT(dmxPatch != NULL);

		item = new QListViewItem(m_listView);
		str.setNum(i + 1);
		item->setText(KColumnUniverse, str);
		item->setText(KColumnPlugin, dmxPatch->plugin->name());
		str.setNum(dmxPatch->output + 1);
		item->setText(KColumnOutput, str);
	}
}

/*****************************************************************************
 *
 *****************************************************************************/

void DMXMapEditor::slotEditMappingButtonClicked()
{
	QListViewItem* item = NULL;
	int universe = -1;
	QString pluginName;
	int output = 0;

	item = m_listView->currentItem();
	if (item == NULL)
		return;

	universe = item->text(KColumnUniverse).toInt() - 1;
	pluginName = item->text(KColumnPlugin);
	output = item->text(KColumnOutput).remove("Output").toInt() - 1;

	DMXPatchEditor dpe(static_cast<QWidget*>(parent()), m_dmxMap, universe,
			   pluginName, output);
	if (dpe.exec() == QDialog::Accepted)
	{
		m_dmxMap->setPatch(universe, dpe.pluginName(), dpe.output());
		init();
	}
}

void DMXMapEditor::slotListViewContextMenuRequested(QListViewItem* item,
						    const QPoint& point,
						    int column)
{
	QStringList::Iterator it;
	QPopupMenu* pluginMenu = NULL;
	QPopupMenu* outputMenu = NULL;
	QString str;
	QString pluginName;
	int outputs = 0;
	int menuid = 0;
	int i = 0;
	int universe = 0;

	if (item == NULL)
		return;

	pluginMenu = new QPopupMenu();
	connect(pluginMenu, SIGNAL(activated(int)),
		this, SLOT(slotPluginMenuActivated(int)));

	str.sprintf("Route universe %s thru...",
		    (const char*) item->text(KColumnUniverse));
	pluginMenu->insertItem(str, INT_MAX);
	pluginMenu->setItemEnabled(INT_MAX, false);
	pluginMenu->insertSeparator();

	for (it = m_pluginList.begin(); it != m_pluginList.end(); ++it)
	{
		pluginName = *it;
		outputs = m_dmxMap->pluginOutputs(pluginName);
		if (outputs >= 0)
		{
			/* Put the plugin's outputs into a sub menu and
			   insert the output menu to the top level menu */
			outputMenu = new QPopupMenu(pluginMenu);
			for (i = 0; i < outputs && i < 100; i++)
			{
				str.sprintf("Output %d", i + 1);
				outputMenu->insertItem(str, menuid + i);
			}

			connect(outputMenu, SIGNAL(activated(int)),
				this, SLOT(slotPluginMenuActivated(int)));

			pluginMenu->insertItem(pluginName, outputMenu);

			menuid += 100;
		}
	}

	pluginMenu->exec(point);

	delete pluginMenu;
}

void DMXMapEditor::slotPluginMenuActivated(int item)
{
	QListViewItem* lvitem = NULL;
	QStringList::Iterator it;
	QString pluginName;
	int pluginIndex = 0;
	int output = 0;
	QString str;

	lvitem = m_listView->currentItem();
	Q_ASSERT(lvitem != NULL);

	/* Get only the particular hundredth (154/100=1.54, cast to int = 1) */
	pluginIndex = (int) (item / 100);

	/* The correct plugin can be found with the index number, since the
	   plugin menu was constructed from the same string list */
	pluginName = m_pluginList[pluginIndex];
	lvitem->setText(KColumnPlugin, pluginName);

	/* Get only the output number */
	output = item - (100 * pluginIndex);
	str.setNum(output + 1);
	lvitem->setText(KColumnOutput, str);
}

/*****************************************************************************
 * OK & Cancel
 *****************************************************************************/

void DMXMapEditor::accept()
{
	QListViewItemIterator it(m_listView);
	QListViewItem* item = NULL;
	int universe = 0;
	int output = 0;
	QString pluginName;

	while ((item = it.current()) != NULL)
	{
		universe = item->text(KColumnUniverse).toInt() - 1;
		pluginName = item->text(KColumnPlugin);
		output = item->text(KColumnOutput).toInt() - 1;

		m_dmxMap->setPatch(universe, pluginName, output);

		++it;
	}

	UI_DMXMapEditor::accept();
}
