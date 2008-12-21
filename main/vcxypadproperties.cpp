/*
  Q Light Controller
  vcxypadproperties.h

  Copyright (C) Stefan Krumm, Heikki Junnila

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
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QMenu>

#include "common/qlcfixturedef.h"
#include "common/qlcchannel.h"
#include "common/qlcfile.h"

#include "vcxypadproperties.h"
#include "xychannelunit.h"
#include "fixturelist.h"
#include "vcxypad.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

const int KColumnFixtureName   ( 0 );
const int KColumnChannelName   ( 1 );
const int KColumnLo            ( 2 );
const int KColumnHi            ( 3 );
const int KColumnReverse       ( 4 );
const int KColumnFixtureID     ( 5 );
const int KColumnChannelNumber ( 6 );

VCXYPadProperties::VCXYPadProperties(QWidget* parent, VCXYPad* xypad)
	: QDialog(parent)
{
	m_xypad = xypad;

	setupUi(this);

	connect(m_addX, SIGNAL(clicked()), this, SLOT(slotAddX()));
	connect(m_removeX, SIGNAL(clicked()), this, SLOT(slotRemoveX()));
	connect(m_addY, SIGNAL(clicked()), this, SLOT(slotAddY()));
	connect(m_removeY, SIGNAL(clicked()), this, SLOT(slotRemoveY()));

	/* Min X spin */
	m_minXSpin->setValue(0);
	connect(m_minXSpin, SIGNAL(valueChanged(const QString&)),
		this, SLOT(slotMinXChanged(const QString&)));

	/* Max X spin */
	m_maxXSpin->setValue(255);
	connect(m_maxXSpin, SIGNAL(valueChanged(const QString&)),
		this, SLOT(slotMaxXChanged(const QString&)));

	/* Min Y spin */
	m_minYSpin->setValue(0);
	connect(m_minYSpin, SIGNAL(valueChanged(const QString&)),
		this, SLOT(slotMinYChanged(const QString&)));

	/* Max Y spin */
	m_maxYSpin->setValue(255);
	connect(m_maxYSpin, SIGNAL(valueChanged(const QString&)),
		this, SLOT(slotMaxYChanged(const QString&)));

	/* X axis reverse */
	m_reverseX->setChecked(false);
	connect(m_reverseX, SIGNAL(toggled(bool)),
		this, SLOT(slotReverseXToggled(bool)));

	/* Y axis reverse */
	m_reverseY->setChecked(false);
	connect(m_reverseY, SIGNAL(toggled(bool)),
		this, SLOT(slotReverseYToggled(bool)));

	/* X list */
	connect(m_listX, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotSelectionXChanged()));
	fillChannelList(m_listX, m_xypad->channelsX());
	m_listX->setCurrentItem(m_listX->topLevelItem(0));

	/* Y list */
	connect(m_listY, SIGNAL(itemSelectionChanged()),
		this, SLOT(slotSelectionYChanged()));
	fillChannelList(m_listY, m_xypad->channelsY());
	m_listY->setCurrentItem(m_listY->topLevelItem(0));
}

VCXYPadProperties::~VCXYPadProperties()
{
}

/**
 * Fill a channel list with XYChannelUnit objects
 */
void VCXYPadProperties::fillChannelList(QTreeWidget *list,
					QList <XYChannelUnit*>* channels)
{
	QListIterator <XYChannelUnit*> it(*channels);
	while (it.hasNext() == true)
	{
		XYChannelUnit* xyc = it.next();
		createChannelEntry(list, xyc->fixtureID(), xyc->channel(),
				   xyc->lo(), xyc->hi(), xyc->reverse());
	}
}

/**
 * Create a channel entry to the given parent listview
 */
QTreeWidgetItem* VCXYPadProperties::createChannelEntry(QTreeWidget* parent,
						       t_fixture_id fixtureID,
						       t_channel channel,
						       t_value lo,
						       t_value hi,
						       bool reverse)
{
	QTreeWidgetItem* item;
	QLCChannel* ch;
	Fixture* fxi;
	QString s;

	fxi = _app->doc()->fixture(fixtureID);
	if (fxi == NULL)
		return NULL;

	item = new QTreeWidgetItem(parent);

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
	item->setText(KColumnHi, s.sprintf("%.3d", hi));

	// Low limit
	item->setText(KColumnLo, s.sprintf("%.3d", lo));

	// Reverse
	if (reverse == true)
		item->setText(KColumnReverse, "Reversed");
	else
		item->setText(KColumnReverse, "Normal");

	// Fixture ID
	item->setText(KColumnFixtureID, s.setNum(fixtureID));

	// Channel number
	item->setText(KColumnChannelNumber, s.sprintf("%.3d", channel));

	return item;
}

void VCXYPadProperties::addChannel(QTreeWidget* list)
{
	FixtureList fl(this);

	if (list == m_listX)
		fl.setWindowTitle("Add a channel to the list of horizontal axes");
	else
		fl.setWindowTitle("Add a channel to the list of vertical axes");

	if (fl.exec() == QDialog::Accepted)
	{
		t_fixture_id fxi_id = fl.selectedFixtureID();
		t_channel ch = fl.selectedChannel();

		if (fxi_id != KNoID && ch != KChannelInvalid)
			createChannelEntry(list, fxi_id, ch, KChannelValueMin,
					   KChannelValueMax, false);
	}
}

void VCXYPadProperties::slotAddY()
{
	addChannel(m_listY);
}

void VCXYPadProperties::slotAddX()
{
	addChannel(m_listX);
}

void VCXYPadProperties::slotRemoveX()
{
	delete(m_listX->currentItem());
}

void VCXYPadProperties::slotRemoveY()
{
	delete(m_listY->currentItem());
}

void VCXYPadProperties::slotMaxXChanged(const QString& text)
{
	QTreeWidgetItem* item = m_listX->currentItem();
	if (item != NULL)
		item->setText(KColumnHi, text);
}

void VCXYPadProperties::slotMinXChanged(const QString& text)
{
	QTreeWidgetItem* item = m_listX->currentItem();
	if (item != NULL)
		item->setText(KColumnLo, text);
}

void VCXYPadProperties::slotMaxYChanged(const QString& text)
{
	QTreeWidgetItem* item = m_listY->currentItem();
	if (item != NULL)
		item->setText(KColumnHi, text);
}

void VCXYPadProperties::slotMinYChanged(const QString& text)
{
	QTreeWidgetItem* item = m_listY->currentItem();
	if (item != NULL)
		item->setText(KColumnLo, text);
}

void VCXYPadProperties::slotReverseXToggled(bool state)
{
	QTreeWidgetItem* item = m_listX->currentItem();
	if (item != NULL)
		item->setText(KColumnReverse, (state) ? "Reversed" : "Normal");
}

void VCXYPadProperties::slotReverseYToggled(bool state)
{
	QTreeWidgetItem* item = m_listY->currentItem();
	if (item != NULL)
		item->setText(KColumnReverse, (state) ? "Reversed" : "Normal");
}

void VCXYPadProperties::slotSelectionXChanged()
{
	QTreeWidgetItem* item = m_listX->currentItem();
	if (item != NULL)
	{
		m_minXSpin->setValue(item->text(KColumnLo).toInt());
		m_maxXSpin->setValue(item->text(KColumnHi).toInt());

		if (item->text(KColumnReverse) == "Reversed")
			m_reverseX->setChecked(true);
		else
			m_reverseX->setChecked(false);
	}
}

void VCXYPadProperties::slotSelectionYChanged()
{
	QTreeWidgetItem* item = m_listY->currentItem();
	if (item != NULL)
	{
		m_minYSpin->setValue(item->text(KColumnLo).toInt());
		m_maxYSpin->setValue(item->text(KColumnHi).toInt());

		if (item->text(KColumnReverse) == "Reversed")
			m_reverseY->setChecked(true);
		else
			m_reverseY->setChecked(false);
	}
}

void VCXYPadProperties::accept()
{
	QList <XYChannelUnit*>* list;

	// Update the X list
	list = m_xypad->channelsX();
	while (list->isEmpty() == false)
		delete list->takeFirst();

	QTreeWidgetItemIterator xit(m_listX);
	while (*xit != NULL)
	{
		list->append(createChannelUnit(*xit));
		++xit;
	}

	// Update the Y list
	list = m_xypad->channelsY();
	while (list->isEmpty() == false)
		delete list->takeFirst();

	QTreeWidgetItemIterator yit(m_listY);
	while (*yit != NULL)
	{
		list->append(createChannelUnit(*yit));
		++yit;
	}
	
	QDialog::accept();
}

/**
 * Create an XY channel unit from the given list item
 */
XYChannelUnit* VCXYPadProperties::createChannelUnit(QTreeWidgetItem* item)
{
	if (item == NULL)
		return NULL;

	return new XYChannelUnit(
		item->text(KColumnFixtureID).toInt(),
		item->text(KColumnChannelNumber).toInt(),
		item->text(KColumnLo).toInt(),
		item->text(KColumnHi).toInt(),
		(item->text(KColumnReverse) == "Reversed") ? true : false);
}
