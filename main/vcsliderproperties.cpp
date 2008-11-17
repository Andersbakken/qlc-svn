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

#include <QTreeWidgetItem>
#include <QInputDialog>
#include <QRadioButton>
#include <QTreeWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>

#include "common/qlccapability.h"
#include "common/qlcchannel.h"

#include "vcsliderproperties.h"
#include "vcslider.h"
#include "inputmap.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

#define KColumnName  0
#define KColumnType  1
#define KColumnRange 2
#define KColumnID    3

VCSliderProperties::VCSliderProperties(QWidget* parent, VCSlider* slider)
	: QDialog(parent)
{
	Q_ASSERT(slider != NULL);
	m_slider = slider;

	setupUi(this);

	/* General page connections */
	connect(m_modeBusRadio, SIGNAL(clicked()),
		this, SLOT(slotModeBusClicked()));
	connect(m_modeLevelRadio, SIGNAL(clicked()),
		this, SLOT(slotModeLevelClicked()));
	connect(m_modeSubmasterRadio, SIGNAL(clicked()),
		this, SLOT(slotModeSubmasterClicked()));
	connect(m_valueExactRadio, SIGNAL(clicked()),
		this, SLOT(slotValueExactClicked()));
	connect(m_valuePercentageRadio, SIGNAL(clicked()),
		this, SLOT(slotValuePercentageClicked()));

	/* Level page connections */
	connect(m_levelLowLimitSpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotLevelLowSpinChanged(int)));
	connect(m_levelHighLimitSpin, SIGNAL(valueChanged(int)),
		this, SLOT(slotLevelHighSpinChanged(int)));
	connect(m_levelCapabilityButton, SIGNAL(clicked()),
		this, SLOT(slotLevelCapabilityButtonClicked()));
	connect(m_levelList, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotLevelListClicked(QTreeWidgetItem*)));
	connect(m_levelAllButton, SIGNAL(clicked()),
		this, SLOT(slotLevelAllClicked()));
	connect(m_levelNoneButton, SIGNAL(clicked()),
		this, SLOT(slotLevelNoneClicked()));
	connect(m_levelInvertButton, SIGNAL(clicked()),
		this, SLOT(slotLevelInvertClicked()));
	connect(m_levelByGroupButton, SIGNAL(clicked()),
		this, SLOT(slotLevelByGroupClicked()));

	/*********************************************************************
	 * General page
	 *********************************************************************/

	/* Name */
	m_nameEdit->setText(m_slider->caption());

	/* Slider mode */
	switch (m_slider->sliderMode())
	{
	default:
	case VCSlider::Bus:
		m_modeBusRadio->setChecked(true);
		slotModeBusClicked();
		break;
	case VCSlider::Level:
		m_modeLevelRadio->setChecked(true);
		slotModeLevelClicked();
		break;
	case VCSlider::Submaster:
		m_modeSubmasterRadio->setChecked(true);
		slotModeLevelClicked();
		break;
	}

	/* Value display style */
	switch (m_slider->valueDisplayStyle())
	{
	default:
	case VCSlider::ExactValue:
		m_valueExactRadio->setChecked(true);
		break;
	case VCSlider::PercentageValue:
		m_valuePercentageRadio->setChecked(true);
		break;
	}

	/* Under construction... */
	m_externalInputGroup->setEnabled(false);

	/*********************************************************************
	 * Bus page
	 *********************************************************************/

	/* Bus combo contents */
	fillBusCombo();

	/* Bus value limit spins */
	m_busLowLimitSpin->setValue(m_slider->busLowLimit());
	m_busHighLimitSpin->setValue(m_slider->busHighLimit());

	/*********************************************************************
	 * Level page
	 *********************************************************************/

	/* Level limit spins */
	m_levelLowLimitSpin->setValue(m_slider->levelLowLimit());
	m_levelHighLimitSpin->setValue(m_slider->levelHighLimit());

	/* Tree widget columns */
	QStringList labels;
	labels << "Name" << "Type" << "Range";
	m_levelList->setHeaderLabels(labels);
	m_levelList->header()->setResizeMode(QHeaderView::ResizeToContents);

	/* Tree widget contents */
	levelUpdateFixtures();
	levelUpdateChannelSelections();
}

VCSliderProperties::~VCSliderProperties()
{
}

/*****************************************************************************
 * General page
 *****************************************************************************/

void VCSliderProperties::slotModeBusClicked()
{
	m_nameEdit->setEnabled(false);
	slotBusComboActivated(m_busCombo->currentIndex());

	m_busValueRangeGroup->setEnabled(true);
	m_busGroup->setEnabled(true);
	
	m_levelValueRangeGroup->setEnabled(false);
	m_levelList->setEnabled(false);
	m_levelAllButton->setEnabled(false);
	m_levelNoneButton->setEnabled(false);
	m_levelInvertButton->setEnabled(false);
	m_levelByGroupButton->setEnabled(false);
}

void VCSliderProperties::slotModeLevelClicked()
{
	m_nameEdit->setEnabled(true);
	
	m_busValueRangeGroup->setEnabled(false);
	m_busGroup->setEnabled(false);
	
	m_levelValueRangeGroup->setEnabled(true);
	m_levelList->setEnabled(true);
	m_levelAllButton->setEnabled(true);
	m_levelNoneButton->setEnabled(true);
	m_levelInvertButton->setEnabled(true);
	m_levelByGroupButton->setEnabled(true);
}

void VCSliderProperties::slotModeSubmasterClicked()
{
}

void VCSliderProperties::slotValueExactClicked()
{
}

void VCSliderProperties::slotValuePercentageClicked()
{
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
		m_busCombo->addItem(s);
	}

	m_busCombo->setCurrentIndex(m_slider->bus());
}

void VCSliderProperties::slotBusComboActivated(int item)
{
	m_nameEdit->setText(Bus::name(item));
}

void VCSliderProperties::slotBusLowLimitSpinChanged(int value)
{
	/* Don't allow the low limit to get higher than the high limit */
	if (value >= m_busHighLimitSpin->value())
		m_busHighLimitSpin->setValue(value + 1);
}

void VCSliderProperties::slotBusHighLimitSpinChanged(int value)
{
	/* Don't allow the high limit to get lower than the low limit */
	if (value <= m_busLowLimitSpin->value())
		m_busLowLimitSpin->setValue(value - 1);
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
	QTreeWidgetItem* item;
	Fixture* fxi;
	QString str;

	fxi = _app->doc()->fixture(id);
	Q_ASSERT(fxi != NULL);

	item = levelFixtureNode(id);
	if (item == NULL)
	{
		item = new QTreeWidgetItem(m_levelList);
		item->setText(KColumnID, str.setNum(id));
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable
			       | Qt::ItemIsTristate);
	}

	item->setText(KColumnName, fxi->name());
	item->setText(KColumnType, fxi->type());
	
	levelUpdateChannels(item, fxi);
}

QTreeWidgetItem* VCSliderProperties::levelFixtureNode(t_fixture_id id)
{
	QTreeWidgetItem* item;
	int i;

	for (i = 0; i < m_levelList->topLevelItemCount(); i++)
	{
		item = m_levelList->topLevelItem(i);
		if (item->text(KColumnID).toInt() == id)
			return item;
	}

	return NULL;
}

void VCSliderProperties::levelUpdateChannels(QTreeWidgetItem* parent,
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

void VCSliderProperties::levelUpdateChannelNode(QTreeWidgetItem* parent,
						Fixture* fxi, t_channel ch)
{
	QTreeWidgetItem* item;
	QLCChannel* channel;
	QString str;

	Q_ASSERT(parent != NULL);

	channel = fxi->channel(ch);
	Q_ASSERT(channel != NULL);

	item = levelChannelNode(parent, ch);
	if (item == NULL)
	{
		item = new QTreeWidgetItem(parent);
		item->setText(KColumnID, str.setNum(ch));
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(KColumnName, Qt::Unchecked);
	}

	item->setText(KColumnName, channel->name());
	item->setText(KColumnType, channel->group());

	levelUpdateCapabilities(item, channel);
}

QTreeWidgetItem* VCSliderProperties::levelChannelNode(QTreeWidgetItem* parent,
						      t_channel ch)
{
	QTreeWidgetItem* item;
	int i;

	Q_ASSERT(parent != NULL);

	for (i = 0; i < parent->childCount(); i++)
	{
		item = parent->child(i);
		if (item->text(KColumnID).toInt() == ch)
			return item;
	}

	return NULL;
}

void VCSliderProperties::levelUpdateCapabilities(QTreeWidgetItem* parent,
						 QLCChannel* channel)
{
	Q_ASSERT(parent != NULL);
	Q_ASSERT(channel != NULL);

	QListIterator <QLCCapability*> it(*channel->capabilities());
	while (it.hasNext() == true)
		levelUpdateCapabilityNode(parent, it.next());
}

void VCSliderProperties::levelUpdateCapabilityNode(QTreeWidgetItem* parent,
						   QLCCapability* cap)
{
	QTreeWidgetItem* item;
	QString str;

	Q_ASSERT(parent != NULL);
	Q_ASSERT(cap != NULL);

	item = new QTreeWidgetItem(parent);
	item->setText(KColumnName, cap->name());
	item->setText(KColumnRange, str.sprintf("%.3d - %.3d",
						cap->min(), cap->max()));
	item->setFlags(item->flags() & (~Qt::ItemIsUserCheckable));
}

void VCSliderProperties::levelUpdateChannelSelections()
{
	/* Check all items that are present in the slider's list of
	   controlled channels. We don't need to set other items off, 
	   because this function is run only during init when everything
	   is off. */
	QListIterator <int> it(m_slider->m_levelChannels);
	while (it.hasNext() == true)
	{
		QTreeWidgetItem* fxiNode;
		QTreeWidgetItem* chNode;

		t_fixture_id fxi_id = KNoID;
		t_channel ch = 0;

		VCSlider::splitCombinedValue(it.next(), &fxi_id, &ch);

		fxiNode = levelFixtureNode(fxi_id);
		Q_ASSERT(fxiNode != NULL);

		chNode = levelChannelNode(fxiNode, ch);
		Q_ASSERT(chNode != NULL);

		chNode->setCheckState(KColumnName, Qt::Checked);
	}
}

void VCSliderProperties::levelSelectChannelsByGroup(QString group)
{
	QTreeWidgetItem* fxi_item;
	QTreeWidgetItem* ch_item;
	int i;
	int j;

	/* Go thru only channel items. Fixture items get (partially) selected 
	   according to their children's state */
	for (i = 0; i < m_levelList->topLevelItemCount(); i++)
	{
		fxi_item = m_levelList->topLevelItem(i);
		Q_ASSERT(fxi_item != NULL);

		for (j = 0; j < fxi_item->childCount(); j++)
		{
			ch_item = fxi_item->child(j);
			Q_ASSERT(ch_item != NULL);

			if (ch_item->text(KColumnType) == group)
				ch_item->setCheckState(KColumnName,
						       Qt::Checked);
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
	QTreeWidgetItem* item;
	QStringList list;

	item = m_levelList->currentItem();
	if (item == NULL || item->parent() == NULL ||
	    item->parent()->parent() == NULL)
		return;

	list = item->text(KColumnRange).split("-");
	Q_ASSERT(list.size() == 2);

	m_levelLowLimitSpin->setValue(list[0].toInt());
	m_levelHighLimitSpin->setValue(list[1].toInt());
}

void VCSliderProperties::slotLevelListClicked(QTreeWidgetItem* item)
{
	/* Enable the capability button if a capability has been selected */
	if (item != NULL && item->parent() != NULL &&
	    item->parent()->parent() != NULL)
	{
		m_levelCapabilityButton->setEnabled(true);
	}
	else
	{
		m_levelCapabilityButton->setEnabled(false);
	}
}

void VCSliderProperties::slotLevelAllClicked()
{
	QTreeWidgetItem* fxi_item;
	int i;

	/* Set all fixture items selected, their children should get selected
	   as well because the fixture items are Controller items. */
	for (i = 0; i < m_levelList->topLevelItemCount(); i++)
	{
		fxi_item = m_levelList->topLevelItem(i);
		Q_ASSERT(fxi_item != NULL);

		fxi_item->setCheckState(KColumnName, Qt::Checked);
	}
}

void VCSliderProperties::slotLevelNoneClicked()
{
	QTreeWidgetItem* fxi_item;
	int i;

	/* Set all fixture items unselected, their children should get unselected
	   as well because the fixture items are Controller items. */
	for (i = 0; i < m_levelList->topLevelItemCount(); i++)
	{
		fxi_item = m_levelList->topLevelItem(i);
		Q_ASSERT(fxi_item != NULL);

		fxi_item->setCheckState(KColumnName, Qt::Unchecked);
	}
}

void VCSliderProperties::slotLevelInvertClicked()
{
	QTreeWidgetItem* fxi_item;
	QTreeWidgetItem* ch_item;
	int i;
	int j;

	/* Go thru only channel items. Fixture items get (partially) selected 
	   according to their children's state */
	for (i = 0; i < m_levelList->topLevelItemCount(); i++)
	{
		fxi_item = m_levelList->topLevelItem(i);
		Q_ASSERT(fxi_item != NULL);

		for (j = 0; j < fxi_item->childCount(); j++)
		{
			ch_item = fxi_item->child(j);
			Q_ASSERT(ch_item != NULL);

			if (ch_item->checkState(KColumnName) == Qt::Checked)
				ch_item->setCheckState(KColumnName,
						       Qt::Unchecked);
			else
				ch_item->setCheckState(KColumnName,
						       Qt::Checked);
		}
	}
}

void VCSliderProperties::slotLevelByGroupClicked()
{
	bool ok = false;
	QString group;

	group = QInputDialog::getItem(this,
				      "Select channels by group",
				      "Select a channel group",
				      QLCChannel::groupList(), 0,
				      false, &ok);

	if (ok == true)
		levelSelectChannelsByGroup(group);
}

/*****************************************************************************
 * OK & Cancel
 *****************************************************************************/

void VCSliderProperties::storeLevelChannels()
{
	QTreeWidgetItem* fxi_item;
	QTreeWidgetItem* ch_item;
	t_fixture_id fxi_id;
	t_channel ch_num;
	int i;
	int j;
	
	/* Clear all channels from the slider first */
	m_slider->clearLevelChannels();

	/* Go thru all fixtures and their channels, add checked channels */
	for (i = 0; i < m_levelList->topLevelItemCount(); i++)
	{
		fxi_item = m_levelList->topLevelItem(i);
		Q_ASSERT(fxi_item != NULL);

		fxi_id = fxi_item->text(KColumnID).toInt();

		for (j = 0; j < fxi_item->childCount(); j++)
		{
			ch_item = fxi_item->child(j);
			Q_ASSERT(ch_item != NULL);

			if (ch_item->checkState(KColumnName) == Qt::Checked)
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
	m_slider->setBus(m_busCombo->currentIndex());
	m_slider->setBusLowLimit(m_busLowLimitSpin->value());
	m_slider->setBusHighLimit(m_busHighLimitSpin->value());

	/* Level page */
	m_slider->setLevelLowLimit(m_levelLowLimitSpin->value());
	m_slider->setLevelHighLimit(m_levelHighLimitSpin->value());
	storeLevelChannels();

	/* Slider mode */
	if (m_modeBusRadio->isChecked() == true)
	{
		m_slider->setSliderMode(VCSlider::Bus);
	}
	else if (m_modeLevelRadio->isChecked() == true)
	{
		m_slider->setSliderMode(VCSlider::Level);
		m_slider->setCaption(m_nameEdit->text());
	}
	else
	{
		m_slider->setSliderMode(VCSlider::Submaster);
	}

	/* Value style */
	if (m_valueExactRadio->isChecked() == true)
		m_slider->setValueDisplayStyle(VCSlider::ExactValue);
	else
		m_slider->setValueDisplayStyle(VCSlider::PercentageValue);

	/* Close dialog */
	QDialog::accept();
}
