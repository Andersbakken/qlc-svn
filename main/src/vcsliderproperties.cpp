/*
  Q Light Controller
  vcsliderproperties.cpp

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

#include <qinputdialog.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlistview.h>
#include <qpushbutton.h>

#include "vcsliderproperties.h"
#include "vcslider.h"
#include "app.h"
#include "doc.h"
#include "fixture.h"
#include "common/qlcchannel.h"
#include "common/qlccapability.h"

extern App* _app;

#define KColumnName 0
#define KColumnType 1
#define KColumnRange 2
#define KColumnID 3

VCSliderProperties::VCSliderProperties(QWidget* parent, VCSlider* slider) :
	UI_VCSliderProperties(parent, "SliderProperties")
{
	m_slider = slider;
}

VCSliderProperties::~VCSliderProperties()
{
}

void VCSliderProperties::init()
{
	Q_ASSERT(m_slider != NULL);

	/* Generic page */
	m_nameEdit->setText(m_slider->caption());
	m_modeGroup->setButton(m_slider->sliderMode());

	/* Bus page */
	fillBusCombo();
	m_busLowLimitSpin->setValue(m_slider->busLowLimit());
	m_busHighLimitSpin->setValue(m_slider->busHighLimit());

	/* Level page */
	m_levelList->addColumn("Name");
	m_levelList->addColumn("Type");
	m_levelList->addColumn("Range");
	m_levelList->setSorting(KColumnRange);
	m_levelList->setRootIsDecorated(true);
	m_levelList->setResizeMode(QListView::LastColumn);
	levelUpdateFixtures();
	levelUpdateChannelSelections();
}

/*****************************************************************************
 * Bus page
 *****************************************************************************/

void VCSliderProperties::fillBusCombo()
{
	QString s;

	m_busCombo->clear();

	for (t_bus_id i = 0; i < KBusCount; i++)
	{
		s.sprintf("%.2d:", i+1);
		s += Bus::name(i);
		m_busCombo->insertItem(s);
	}

	m_busCombo->setCurrentItem(m_slider->bus());
}

void VCSliderProperties::slotBusLowLimitSpinChanged(int value)
{
	if (value >= m_busHighLimitSpin->value())
	{
		m_busHighLimitSpin->setValue(value + 1);
	}
}

void VCSliderProperties::slotBusHighLimitSpinChanged(int value)
{
	if (value <= m_busLowLimitSpin->value())
	{
		m_busLowLimitSpin->setValue(value - 1);
	}
}

/*****************************************************************************
 * Level page
 *****************************************************************************/

void VCSliderProperties::levelUpdateFixtures()
{
	t_fixture_id id = 0;
	Fixture* fxi = NULL;

	for (id = 0; id < KFixtureArraySize; id++)
	{
		fxi = _app->doc()->fixture(id);
		if (fxi == NULL)
			continue;

		levelUpdateFixtureNode(id);
	}
}

void VCSliderProperties::levelUpdateFixtureNode(t_fixture_id id)
{
	Fixture* fxi = NULL;
	QCheckListItem* item = NULL;
	QString str;

	fxi = _app->doc()->fixture(id);
	Q_ASSERT(fxi != NULL);

	item = levelFixtureNode(id);
	if (item == NULL)
		item = new QCheckListItem(m_levelList, fxi->name(),
					  QCheckListItem::CheckBoxController);

	item->setText(KColumnType, fxi->type());
	str.setNum(id);
	item->setText(KColumnID, str);

	levelUpdateChannels(item, fxi);
}

QCheckListItem* VCSliderProperties::levelFixtureNode(t_fixture_id id)
{
	QCheckListItem* item = NULL;

	for (item = static_cast<QCheckListItem*> (m_levelList->firstChild());
	     item != NULL;
	     item = static_cast<QCheckListItem*> (item->nextSibling()))
	{
		if (item->text(KColumnID).toInt() == id)
			return item;
	}

	return NULL;
}

void VCSliderProperties::levelUpdateChannels(QCheckListItem* parent,
					     Fixture* fxi)
{
	t_channel channels = 0;
	t_channel ch = 0;

	Q_ASSERT(parent != NULL);
	Q_ASSERT(fxi != NULL);

	channels = fxi->channels();
	for (ch = 0; ch < channels; ch++)
		levelUpdateChannelNode(parent, fxi, ch);
}

void VCSliderProperties::levelUpdateChannelNode(QCheckListItem* parent,
						Fixture* fxi,
						t_channel ch)
{
	QCheckListItem* item = NULL;
	QLCChannel* channel = NULL;
	QString str;

	Q_ASSERT(parent != NULL);

	channel = fxi->channel(ch);
	Q_ASSERT(channel != NULL);

	item = levelChannelNode(parent, ch);
	if (item == NULL)
		item = new QCheckListItem(parent, channel->name(),
					  QCheckListItem::CheckBox);

	item->setText(KColumnType, channel->group());
	str.setNum(ch);
	item->setText(KColumnID, str);

	levelUpdateCapabilities(item, channel);
}

QCheckListItem* VCSliderProperties::levelChannelNode(QCheckListItem* parent,
						     t_channel ch)
{
	QCheckListItem* item = NULL;

	for (item = static_cast<QCheckListItem*> (parent->firstChild());
	     item != NULL;
	     item = static_cast<QCheckListItem*> (item->nextSibling()))
	{
		if (item->text(KColumnID).toInt() == ch)
			return item;
	}

	return NULL;
}

void VCSliderProperties::levelUpdateCapabilities(QCheckListItem* parent,
						 QLCChannel* channel)
{
	QLCCapability* cap = NULL;

	Q_ASSERT(parent != NULL);
	Q_ASSERT(channel != NULL);

	QPtrListIterator<QLCCapability> it(*channel->capabilities());

	while ( (cap = it.current()) != NULL )
	{
		levelUpdateCapabilityNode(parent, cap);
		++it;
	}
}

void VCSliderProperties::levelUpdateCapabilityNode(QCheckListItem* parent,
						   QLCCapability* cap)
{
	QListViewItem* item = NULL;
	QString str;

	Q_ASSERT(parent != NULL);
	Q_ASSERT(cap != NULL);

	item = new QListViewItem(parent);
	item->setText(KColumnName, cap->name());
	str.sprintf("%.3d - %.3d", cap->min(), cap->max());
	item->setText(KColumnRange, str);
}

void VCSliderProperties::levelUpdateChannelSelections()
{
	QValueListIterator<int> it;
	QValueList<int> list = m_slider->levelChannels();

	for (it = list.begin(); it != list.end(); ++it)
	{
		/* TODO */
	}
}

void VCSliderProperties::levelSelectChannelsByGroup(QString group)
{
	QCheckListItem* fxiNode = NULL;
	QCheckListItem* chNode = NULL;

	for (fxiNode = static_cast<QCheckListItem*> (m_levelList->firstChild());
	     fxiNode != NULL;
	     fxiNode = static_cast<QCheckListItem*> (fxiNode->nextSibling()))
	{
		for (chNode = static_cast<QCheckListItem*> (fxiNode->firstChild());
		     chNode != NULL;
		     chNode = static_cast<QCheckListItem*> (chNode->nextSibling()))
		{
			if (chNode->text(KColumnType) == group)
				chNode->setOn(true);
			else
				chNode->setOn(false);
		}
	}
}

void VCSliderProperties::slotLevelLowSpinChanged(int value)
{
	if (value >= m_levelHighLimitSpin->value())
		m_levelHighLimitSpin->setValue(value + 1);
}

void VCSliderProperties::slotLevelHighSpinChanged(int value)
{
	if (value <= m_levelLowLimitSpin->value())
		m_levelLowLimitSpin->setValue(value - 1);
}

void VCSliderProperties::slotLevelCapabilityButtonClicked()
{
	QListViewItem* item = NULL;
	QStringList list;

	item = m_levelList->currentItem();
	if (item == NULL || item->depth() != 2)
		return;

	list = QStringList::split("-", item->text(KColumnRange));
	Q_ASSERT(list.size() == 2);

	m_levelLowLimitSpin->setValue(list[0].toInt());
	m_levelHighLimitSpin->setValue(list[1].toInt());
}

void VCSliderProperties::slotLevelListClicked(QListViewItem* item)
{
	if (item == NULL)
		return;

	/* Enable the capability button if a capability has been selected */
	if (item->depth() == 2)
		m_levelCapabilityButton->setEnabled(true);
	else
		m_levelCapabilityButton->setEnabled(false);
}

void VCSliderProperties::slotLevelAllClicked()
{
	QCheckListItem* fxi_item = NULL;

	/* Set all fixture items selected, their children should get selected
	   as well because the fixture items are Controller items. */
	for (fxi_item = static_cast<QCheckListItem*> (m_levelList->firstChild());
	     fxi_item != NULL;
	     fxi_item = static_cast<QCheckListItem*> (fxi_item->nextSibling()))
	{
		fxi_item->setOn(true);
	}
}

void VCSliderProperties::slotLevelNoneClicked()
{
	QCheckListItem* fxi_item = NULL;

	/* Set all fixture items unselected, their children should get unselected
	   as well because the fixture items are Controller items. */
	for (fxi_item = static_cast<QCheckListItem*> (m_levelList->firstChild());
	     fxi_item != NULL;
	     fxi_item = static_cast<QCheckListItem*> (fxi_item->nextSibling()))
	{
		fxi_item->setOn(false);
	}
}

void VCSliderProperties::slotLevelInvertClicked()
{
	QCheckListItem* fxi_item = NULL;
	QCheckListItem* ch_item = NULL;

	/* Go thru only channel items. Fixture items get (partially) selected 
	   according to their children's state */
	for (fxi_item = static_cast<QCheckListItem*> (m_levelList->firstChild());
	     fxi_item != NULL;
	     fxi_item = static_cast<QCheckListItem*> (fxi_item->nextSibling()))
	{
		for (ch_item = static_cast<QCheckListItem*> (fxi_item->firstChild());
		     ch_item != NULL;
		     ch_item = static_cast<QCheckListItem*> (ch_item->nextSibling()))
		{
			ch_item->setOn(!ch_item->isOn());
		}
	}
}

void VCSliderProperties::slotLevelByGroupButtonClicked()
{
	bool ok = false;
	QString group;

	group = QInputDialog::getItem("Select channels by group",
				      "Select a channel group",
				      QLCChannel::groupList(), 0,
				      false, &ok, _app);

	if (ok == true)
		levelSelectChannelsByGroup(group);
}

/*****************************************************************************
 * OK & Cancel
 *****************************************************************************/

void VCSliderProperties::storeLevelChannels()
{
	QCheckListItem* fxi_item = NULL;
	QCheckListItem* ch_item = NULL;
	t_fixture_id fxi_id = KNoID;
	t_channel ch_num = 0;
	
	m_slider->clearLevelChannels();

	for (fxi_item = static_cast<QCheckListItem*> (m_levelList->firstChild());
	     fxi_item != NULL;
	     fxi_item = static_cast<QCheckListItem*> (fxi_item->nextSibling()))
	{
		fxi_id = fxi_item->text(KColumnID).toInt();

		for (ch_item = static_cast<QCheckListItem*> (fxi_item->firstChild());
		     ch_item != NULL;
		     ch_item = static_cast<QCheckListItem*> (ch_item->nextSibling()))
		{
			if (ch_item->isOn() == true)
			{
				ch_num = ch_item->text(KColumnID).toInt();
				m_slider->addLevelChannel(fxi_id, ch_num);
			}
		}
	}
}

void VCSliderProperties::accept()
{
	/* Bus page */
	m_slider->setBus(m_busCombo->currentItem());
	m_slider->setBusLowLimit(m_busLowLimitSpin->value());
	m_slider->setBusHighLimit(m_busHighLimitSpin->value());

	/* Level page */
	m_slider->setLevelLowLimit(m_levelLowLimitSpin->value());
	m_slider->setLevelHighLimit(m_levelHighLimitSpin->value());
	storeLevelChannels();

	/* Set general page stuff last so that name & mode don't get
	   overridden by bus/value/submaster setters */
	m_slider->setCaption(m_nameEdit->text());
	m_slider->setSliderMode(
		(VCSlider::SliderMode) m_modeGroup->selectedId());

	UI_VCSliderProperties::accept();
}
