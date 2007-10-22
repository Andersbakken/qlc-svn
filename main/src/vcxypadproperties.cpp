/*
  Q Light Controller
  vcbuttonproperties.h

  Copyright (C) 2005, Stefan Krumm, Heikki Junnila

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

#include <assert.h>
#include <qlistview.h>
#include <qptrlist.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qpopupmenu.h>
#include <qcombobox.h>
#include <qpushbutton.h>

#include "common/qlcfixturedef.h"
#include "common/qlcchannel.h"
#include "common/filehandler.h"

#include "vcxypad.h"
#include "xychannelunit.h"
#include "vcxypadproperties.h"
#include "fixturelist.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

const int KColumnFixtureName    ( 0 );
const int KColumnChannelName   ( 1 );
const int KColumnLo            ( 2 );
const int KColumnHi            ( 3 );
const int KColumnReverse       ( 4 );
const int KColumnFixtureID      ( 5 );
const int KColumnChannelNumber ( 6 );

const int KComboItemReverse    ( 0 );
const int KComboItemNormal     ( 1 );

VCXYPadProperties::VCXYPadProperties(QWidget* parent, const char* name)
	: UI_VCXYPadProperties(parent, name, true)
{
	m_parent = static_cast<VCXYPad*> (parent);
}


VCXYPadProperties::~VCXYPadProperties()
{
}


void VCXYPadProperties::init()
{
	m_addX->setPixmap(QPixmap(QString(PIXMAPS) + QString("/edit_add.png")));
	m_addY->setPixmap(QPixmap(QString(PIXMAPS) + QString("/edit_add.png")));

	m_removeX->setPixmap(QPixmap(QString(PIXMAPS) + QString("/edit_remove.png")));
	m_removeY->setPixmap(QPixmap(QString(PIXMAPS) + QString("/edit_remove.png")));

	fillChannelList(m_listX, m_parent->channelsX());
	fillChannelList(m_listY, m_parent->channelsY());

	m_maxYSpin->setValue(255);
	m_maxXSpin->setValue(255);

	m_reverseXCombo->insertItem(KXMLQLCTrue, KComboItemReverse);
	m_reverseXCombo->insertItem(KXMLQLCFalse, KComboItemNormal);

	m_reverseYCombo->insertItem(KXMLQLCTrue, KComboItemReverse);
	m_reverseYCombo->insertItem(KXMLQLCFalse, KComboItemNormal);

	m_listX->setSelected(m_listX->firstChild(), true);
	slotSelectionXChanged(m_listX->firstChild());

	m_listY->setSelected(m_listY->firstChild(), true);
	slotSelectionYChanged(m_listY->firstChild());
}

/**
 * Fill a channel list with XYChannelUnit objects
 */
void VCXYPadProperties::fillChannelList(QListView *list,
					QPtrList<XYChannelUnit>* channels)
{
	QPtrListIterator<XYChannelUnit> it(*channels);
	XYChannelUnit *unit = NULL;

	while ( (unit = *it) != NULL)
	{
		++it;

		createChannelEntry(list,
				   unit->fixtureID(),
				   unit->channel(),
				   unit->lo(),
				   unit->hi(),
				   unit->reverse());
	}
}

/**
 * Create a channel entry to the given parent listview
 */
QListViewItem* VCXYPadProperties::createChannelEntry(QListView* parent,
						     t_fixture_id fixtureID,
						     t_channel channel,
						     t_value lo,
						     t_value hi,
						     bool reverse)
{
	Fixture* fxi = NULL;
	QLCChannel* ch = NULL;
	QListViewItem* item = NULL;
	QString s;

	fxi = _app->doc()->fixture(fixtureID);
	if (fxi == NULL)
	{
		return NULL;
	}

	item = new QListViewItem(parent);

	// Fixture name
	item->setText(KColumnFixtureName, fxi->name());

	// Channel name
	ch = fxi->channel(channel);
	if (ch != NULL)
	{
		s.sprintf("%.3d: ", channel + 1);
		s += ch->name();
		item->setText(KColumnChannelName, s);
	}
	else
	{
		delete item;
		return NULL;
	}

	// High limit
	s.sprintf("%.3d", hi);
	item->setText(KColumnHi, s);

	// Low limit
	s.sprintf("%.3d", lo);
	item->setText(KColumnLo, s);

	// Reverse
	if (reverse == true)
	{
		item->setText(KColumnReverse, KXMLQLCTrue);
	}
	else
	{
		item->setText(KColumnReverse, KXMLQLCFalse);
	}

	// Fixture ID
	s.setNum(fixtureID);
	item->setText(KColumnFixtureID, s);

	// Channel number
	s.sprintf("%.3d", channel);
	item->setText(KColumnChannelNumber, s);

	return item;
}

void VCXYPadProperties::slotAddY()
{
	slotAdd(m_listY);
}

void VCXYPadProperties::slotAddX()
{
	slotAdd(m_listX);
}

void VCXYPadProperties::slotRemoveX()
{
	delete(m_listX->currentItem());
}

void VCXYPadProperties::slotRemoveY()
{
	delete(m_listY->currentItem());
}

void VCXYPadProperties::slotAdd(QListView *list)
{
	FixtureList* fl = new FixtureList(this);
	fl->init();

	if (list == m_listX)
	{
		fl->setCaption("Add a channel to the list of horizontal axes");
	}
	else
	{
		fl->setCaption("Add a channel to the list of vertical axes");
	}

	if (fl->exec() == QDialog::Accepted)
	{
		t_fixture_id fxi_id = fl->selectedFixtureID();
		t_channel ch = fl->selectedChannel();

		if (fxi_id != KNoID && ch != KChannelInvalid)
		{
			createChannelEntry(list, fxi_id, ch,
					   KChannelValueMin,
					   KChannelValueMax,
					   false);
		}
	}

	delete fl;
}

void VCXYPadProperties::slotMaxXChanged(const QString& text)
{
	QListViewItem* item = m_listX->currentItem();
	if (item)
	{
		item->setText(KColumnHi, text);
	}
}

void VCXYPadProperties::slotMinXChanged(const QString& text)
{
	QListViewItem* item = m_listX->currentItem();
	if (item)
	{
		item->setText(KColumnLo, text);
	}

}

void VCXYPadProperties::slotMaxYChanged(const QString& text)
{
	QListViewItem* item = m_listY->currentItem();
	if (item)
	{
		item->setText(KColumnHi, text);
	}
}

void VCXYPadProperties::slotMinYChanged(const QString& text)
{
	QListViewItem* item = m_listY->currentItem();
	if (item)
	{
		item->setText(KColumnLo, text);
	}
}

void VCXYPadProperties::slotReverseXActivated(const QString& text)
{
	QListViewItem* item = m_listX->currentItem();
	if (item)
	{
		item->setText(KColumnReverse, text);
	}
}

void VCXYPadProperties::slotReverseYActivated(const QString& text)
{
	QListViewItem* item = m_listY->currentItem();
	if (item)
	{
		item->setText(KColumnReverse, text);
	}
}

void VCXYPadProperties::slotSelectionXChanged(QListViewItem* item)
{
	if (item)
	{
		m_minXSpin->setValue(item->text(KColumnLo).toInt());
		m_maxXSpin->setValue(item->text(KColumnHi).toInt());
		if (item->text(KColumnReverse) == KXMLQLCTrue)
		{
			m_reverseXCombo->setCurrentItem(KComboItemReverse);
		}
		else
		{
			m_reverseXCombo->setCurrentItem(KComboItemNormal);
		}
	}
}

void VCXYPadProperties::slotSelectionYChanged(QListViewItem* item)
{
	if (item)
	{
		m_minYSpin->setValue(item->text(KColumnLo).toInt());
		m_maxYSpin->setValue(item->text(KColumnHi).toInt());
		if (item->text(KColumnReverse) == KXMLQLCTrue)
		{
			m_reverseYCombo->setCurrentItem(KComboItemReverse);
		}
		else
		{
			m_reverseYCombo->setCurrentItem(KComboItemNormal);
		}
	}
}

/**
 * Set a value to a min/max/reverse column using a context menu
 */
void VCXYPadProperties::slotContextMenuRequested(QListViewItem* item,
						 const QPoint &point,
						 int column)
{
	int result;
	QString s;

	if (column == KColumnLo)
	{
		result = invokeDMXValueMenu(point);

		if (result <= KChannelValueMax &&
		result >= KChannelValueMin)
		{
			s.sprintf("%.3d", result);
			item->setText(KColumnLo, s);

			slotSelectionXChanged(m_listX->currentItem());
			slotSelectionYChanged(m_listY->currentItem());
		}
	}
	else if (column == KColumnHi)
	{
		result = invokeDMXValueMenu(point);

		if (result <= KChannelValueMax &&
		result >= KChannelValueMin)
		{
			s.sprintf("%.3d", result);
			item->setText(KColumnHi, s);

			slotSelectionXChanged(m_listX->currentItem());
			slotSelectionYChanged(m_listY->currentItem());
		}
	}
	else if (column == KColumnReverse)
	{
		QPopupMenu* menu = new QPopupMenu();
		menu->insertItem("Reverse", KNoID);
		menu->insertSeparator();
		menu->insertItem(KXMLQLCTrue, KComboItemReverse);
		menu->insertItem(KXMLQLCFalse, KComboItemNormal);

		result = menu->exec(point);
		if (result == KComboItemNormal)
		{
			item->setText(KColumnReverse, KXMLQLCFalse);
		}
		else if (result == KComboItemReverse)
		{
			item->setText(KColumnReverse, KXMLQLCTrue);
		}

		slotSelectionXChanged(m_listX->currentItem());
		slotSelectionYChanged(m_listY->currentItem());

		delete menu;
	}
}

/**
 * Display a DMX value menu, divided into submenus of 16 values
 */
int VCXYPadProperties::invokeDMXValueMenu(const QPoint &point)
{
	int result;

	QPopupMenu* menu = new QPopupMenu;
	QPtrList <QPopupMenu> deleteList;

	menu->insertItem("Value", KNoID);
	menu->insertSeparator();

	menu->insertItem(QString::number(KChannelValueMin), KChannelValueMin);
	menu->insertItem(QString::number(KChannelValueMax), KChannelValueMax);
	for (t_value i = 0; i != KChannelValueMax; i += 15)
	{
		QPopupMenu* sub = new QPopupMenu();
		deleteList.append(sub);

		QString top;
		top.sprintf("%d - %d", i+1, i + 15);

		for (t_value j = 1; j < 16; j++)
		{
			QString num;
			num.setNum(i + j);
			sub->insertItem(num, i + j);
		}

		menu->insertItem(top, sub);
	}

	result = menu->exec(point);

	while (deleteList.isEmpty() == false)
	{
		delete deleteList.take(0);
	}

	delete menu;

	return result;
}

void VCXYPadProperties::slotOKClicked()
{
	QPtrList<XYChannelUnit>* list;

	// Update the X list
	list = m_parent->channelsX();

	list->setAutoDelete(true);
	list->clear();
	list->setAutoDelete(false);

	QListViewItemIterator xit(m_listX);

	while (xit.current())
	{
		list->append(createChannelUnit(*xit));
		++xit;
	}

	// Update the Y list
	list = m_parent->channelsY();

	list->setAutoDelete(true);
	list->clear();
	list->setAutoDelete(false);

	QListViewItemIterator yit(m_listY);

	while (yit.current())
	{
		list->append(createChannelUnit(*yit));
		++yit;
	}
       accept();
}

/**
 * Create an XY channel unit from the given list item
 */
XYChannelUnit* VCXYPadProperties::createChannelUnit(QListViewItem* item)
{
	if (item == NULL)
	{
		return NULL;
	}
	else
	{
		return new XYChannelUnit(
			item->text(KColumnFixtureID).toInt(),
			item->text(KColumnChannelNumber).toInt(),
			item->text(KColumnLo).toInt(),
			item->text(KColumnHi).toInt(),
			item->text(KColumnReverse) == KXMLQLCTrue
		);
	}
}





