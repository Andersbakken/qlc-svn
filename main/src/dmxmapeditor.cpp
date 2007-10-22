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

#include "dmxmapeditor.h"
#include "dmxmap.h"

#define KColumnUniverse 0
#define KColumnPlugin   1
#define KColumnOutput   2

DMXMapEditor::DMXMapEditor(QWidget* parent, DMXMap* dmxMap)
	: UI_DMXMapEditor(parent, "DMX Map Editor", true),
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
}

void DMXMapEditor::slotListViewContextMenuRequested(QListViewItem* item,
						    const QPoint& point,
						    int column)
{
	QStringList list;
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

	str.sprintf("Route universe %s thru...",
		    (const char*) item->text(KColumnUniverse));
	pluginMenu->insertItem(str, INT_MAX);
	pluginMenu->setItemEnabled(INT_MAX, false);
	pluginMenu->insertSeparator();

	for (it = m_pluginList.begin(); it != m_pluginList.end(); ++it)
	{
		pluginName = *it;

		/* Find out, how many outputs the plugin has. Skip if it has
		   none (i.e. its probably an invalid plugin) */
		outputs = m_dmxMap->pluginOutputs(*it);
		if (outputs > 0)
		{
			/* Put the plugin's outputs into a sub menu and
			   insert the output menu to the top level menu */
			outputMenu = new QPopupMenu(pluginMenu);
			pluginMenu->insertItem(pluginName, outputMenu);
			for (i = 0; i < outputs; i++)
			{
				str.sprintf("Output %d", i + 1);
				outputMenu->insertItem(str, menuid + i);
			}

			menuid += 1000;
		}
	}

	pluginMenu->exec(point);

	delete pluginMenu;
}

/*****************************************************************************
 * OK & Cancel
 *****************************************************************************/

void DMXMapEditor::accept()
{
	UI_DMXMapEditor::accept();
}
