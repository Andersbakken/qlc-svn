/*
  Q Light Controller
  advancedsceneeditor.cpp

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

#include <qlistbox.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qbuttongroup.h>
#include <qtooltip.h>
#include <qinputdialog.h>
#include <qapplication.h>
#include <qfont.h>
#include <qheader.h>
#include <qpopupmenu.h>
#include <qmessagebox.h>
#include <limits.h>
#include <assert.h>
#include <math.h>

#include "common/qlcfixturedef.h"
#include "common/qlcchannel.h"
#include "common/qlccapability.h"

#include "advancedsceneeditor.h"
#include "app.h"
#include "doc.h"
#include "function.h"
#include "fixture.h"
#include "scene.h"
#include "editscenevalue.h"

extern App* _app;

const int KColumnNumber  ( 0 );
const int KColumnChannel ( 1 );
const int KColumnCapability  ( 2 );
const int KColumnValue   ( 3 );
const int KColumnType    ( 4 );

AdvancedSceneEditor::AdvancedSceneEditor(QWidget* parent, Scene* scene)
	: UI_AdvancedSceneEditor(parent, "Advanced Scene Editor", true)
{
	Q_ASSERT(scene != NULL);
	m_original = scene;

	m_scene = new Scene();
	m_scene->copyFrom(scene, scene->fixture());

	m_sceneNameEdit->setText(m_scene->name());
	m_sceneNameEdit->setSelection(0, m_scene->name().length());
	initListView();
}

AdvancedSceneEditor::~AdvancedSceneEditor()
{
	Q_ASSERT(m_scene != NULL);
	delete m_scene;
	m_scene = NULL;
}

/*****************************************************************************
 * Context Menu
 *****************************************************************************/

void AdvancedSceneEditor::slotContextMenu(QListViewItem* item,
					  const QPoint &pos,
					  int col)
{
	if (m_scene == NULL)
		return;

	switch (col)
	{
	case KColumnNumber:
		break;

	case KColumnChannel:
		break;

	case KColumnCapability:
		invokeCapabilityMenu(pos);
		break;

	case KColumnValue:
		invokeValueMenu(pos);
		break;

	case KColumnType:
		invokeTypeMenu(pos);
		break;

	default:
		break;
	}
}

/*****************************************************************************
 * Capability Menu
 *****************************************************************************/

void AdvancedSceneEditor::invokeCapabilityMenu(const QPoint &pos)
{
	QLCCapability* cap = NULL;
	QLCChannel* channel = NULL;
	Fixture* fxi = NULL;
	QPopupMenu* menu = NULL;
	QPopupMenu* chmenu = NULL;
	QPopupMenu* capmenu = NULL;
	t_value i = 0;
	QString s;
	ChannelList::iterator it;

	fxi = _app->doc()->fixture(m_scene->fixture());
	Q_ASSERT(fxi != NULL);

	menu = new QPopupMenu();
	connect(menu, SIGNAL(activated(int)),
		this, SLOT(slotCapabilityMenuActivated(int)));

	/* Put all selected channels into the top-level menu */
	for (it = m_selection.begin(); it != m_selection.end(); ++it)
	{
		channel = fxi->channel(*it);
		Q_ASSERT(channel != NULL);

		chmenu = new QPopupMenu(menu);
		menu->insertItem(channel->name(), chmenu);
		connect(chmenu, SIGNAL(activated(int)),
			this, SLOT(slotCapabilityMenuActivated(int)));

		QPtrListIterator <QLCCapability>
			capit(*channel->capabilities());

		/* Put each channel's capabilities into a 2nd level menu */
		while (*capit != NULL)
		{
			capmenu = new QPopupMenu(chmenu);
			chmenu->insertItem((*capit)->name(), capmenu);
			connect(capmenu, SIGNAL(activated(int)),
				this, SLOT(slotCapabilityMenuActivated(int)));
			
			/* Put each capability's values into a 3rd level menu */
			for (i = (*capit)->min(); i < (*capit)->max(); i++)
			{
				s.sprintf("%.3d", i);
				capmenu->insertItem(s, i);
			}
			
			/* The maximum value has to be written manually because
			   255 would cause an endless loop in the for-clause */
			s.sprintf("%.3d", (*capit)->max());
			capmenu->insertItem(s, i);

			++capit;
		}
	}

	menu->exec(pos);
	delete menu;
}

void AdvancedSceneEditor::slotCapabilityMenuActivated(int value)
{
	QLCCapability* c = NULL;
	ChannelList::iterator it;

	if (value < 0 || value > 255)
		return;

	for (it = m_selection.begin(); it != m_selection.end(); ++it)
		m_scene->set(*it, value, m_scene->channelValue(*it).type);

	updateSelectedItems();
}

/*****************************************************************************
 * Value Menu
 *****************************************************************************/

void AdvancedSceneEditor::invokeValueMenu(const QPoint &pos)
{
	QPopupMenu* menu = NULL;
	QPopupMenu* sub = NULL;
	QString top;
	QString num;

	/* Insert common min & max values to the top of the menu */
	menu = new QPopupMenu();
	menu->insertItem("Off", 0);
	menu->insertItem("Full", 255);
	menu->insertSeparator();
	connect(menu, SIGNAL(activated(int)),
		this, SLOT(slotValueMenuActivated(int)));

	/* Insert all values in sub-menus containing 15 values each */
	for (t_value i = 0; i != 255; i += 15)
	{
		sub = new QPopupMenu(menu);
		for (t_value j = 1; j < 16; j++)
		{
			num.setNum(i + j);
			sub->insertItem(num, i + j);
		}

		top.sprintf("%d - %d", i+1, i + 15);
		menu->insertItem(top, sub);
		connect(sub, SIGNAL(activated(int)),
			this, SLOT(slotValueMenuActivated(int)));
	}

	menu->exec(pos);
	delete menu;
}

void AdvancedSceneEditor::slotValueMenuActivated(int value)
{
	if (value < 0 || value > 255)
	{
		return;
	}
	else
	{
		/* Set the selected value to all selected scene channels */
		ChannelList::iterator it;
		for (it = m_selection.begin(); it != m_selection.end(); ++it)
		{
			m_scene->set(*it, value,
				     m_scene->valueType(*it));
		}

		/* Update scene contents to list view */
		updateSelectedItems();
	}
}

/*****************************************************************************
 * Value Type Menu
 *****************************************************************************/

void AdvancedSceneEditor::invokeTypeMenu(const QPoint &pos)
{
	QPopupMenu* menu = new QPopupMenu;
	menu->setCheckable(false);

	menu->insertItem("Fade", Scene::Fade);
	menu->insertItem("Set", Scene::Set);
	menu->insertItem("NoSet", Scene::NoSet);

	connect(menu, SIGNAL(activated(int)),
		this, SLOT(slotTypeMenuActivated(int)));

	menu->exec(pos);
	disconnect(menu);

	delete menu;
}

void AdvancedSceneEditor::slotTypeMenuActivated(int type)
{
	if (type == Scene::Fade || type == Scene::Set || type == Scene::NoSet)
	{
		/* Update the selected type to all selected scene channels */
		ChannelList::iterator it;
		for (it = m_selection.begin(); it != m_selection.end(); ++it)
		{
			m_scene->set(*it,
				     m_scene->channelValue(*it).value,
				     static_cast<Scene::ValueType> (type));
		}

		/* Update scene contents to list view */
		updateSelectedItems();
	}
}

/*****************************************************************************
 * List view
 *****************************************************************************/

void AdvancedSceneEditor::initListView()
{
	m_listView->clear();

	/* Create channel items into listview and update their contents */
	for (t_channel ch = 0; ch < m_scene->channels(); ch++)
		updateChannelItem(new QListViewItem(m_listView), ch);
}

void AdvancedSceneEditor::updateSelectedItems()
{
	QListViewItemIterator lvit(m_listView);
	ChannelList::iterator chit;
	t_channel ch = 0;

	/* Update the contents of all selected items */
	while ((*lvit) != NULL)
	{
		ch = (*lvit)->text(KColumnNumber).toInt() - 1;
		chit = m_selection.find(ch);

		if (chit != m_selection.end())
			updateChannelItem((*lvit), ch);

		++lvit;
	}
}

void AdvancedSceneEditor::updateChannelItem(QListViewItem* item, t_channel ch)
{
	QLCChannel* channel = NULL;
	QLCCapability* cap = NULL;
	Fixture* fxi = NULL;
	QString s;

	Q_ASSERT(item != NULL);

	/* Get the fixture object of the scene */
	fxi = _app->doc()->fixture(m_scene->fixture());
	Q_ASSERT(fxi != NULL);

	/* Get channel number and set it to the item */
	s.sprintf("%.3d", ch + 1);
	item->setText(KColumnNumber, s);

	/* Get a channel object from the fixture and set its name to the item */
	channel = fxi->channel(ch);
	Q_ASSERT(channel != NULL);
	item->setText(KColumnChannel, channel->name());

	/* Get a capability object that matches the channel value and set the
	   capability's name to the item */
	cap = channel->searchCapability(m_scene->channelValue(ch).value);
	if (cap == NULL)
		s = QString("Unknown");
	else
		s = cap->name();
	item->setText(KColumnCapability, s);

	/* Get channel value from the scene and set its value to the item */
	s.sprintf("%.3d", m_scene->channelValue(ch).value);
	item->setText(KColumnValue, s);

	/* Get the channel's value type from the scene and set it to the item */
	item->setText(KColumnType, m_scene->valueTypeString(ch));
}

void AdvancedSceneEditor::slotSelectionChanged()
{
	QListViewItem* item = NULL;
	t_channel ch = 0;

	for (item = m_listView->firstChild(); item != NULL;
	     item = item->nextSibling())
	{
		ch = item->text(KColumnNumber).toInt() - 1;

		/* Remove such channel numbers that are not selected in the
		   listview and append those that are selected (but not yet
		   found from the selection list) */
		if (item->isSelected() == false)
			m_selection.remove(ch);
		else if (m_selection.find(ch) == m_selection.end())
			m_selection.append(ch);
	}
}

void AdvancedSceneEditor::slotDoubleClicked(QListViewItem* item)
{
	slotSelectionChanged();
	slotEditValueClicked();
}

/*****************************************************************************
 * Control buttons
 *****************************************************************************/

void AdvancedSceneEditor::slotEditValueClicked()
{
	QMessageBox::information(this, "TODO",
				 "Use right-mouse over different columns to\n"\
				 "edit scene values for now...");
}

void AdvancedSceneEditor::slotOKClicked()
{
	m_scene->setName(m_sceneNameEdit->text());

	m_original->copyFrom(m_scene, m_original->fixture());
	_app->doc()->setModified();

	accept();
}

void AdvancedSceneEditor::slotCancelClicked()
{
	reject();
}
