/*
  Q Light Controller - Fixture Editor
  editmode.cpp

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

#include <qwidget.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qpixmap.h>
#include <qinputdialog.h>
#include <qmessagebox.h>

#include "common/qlcfixture.h"
#include "common/qlcphysical.h"
#include "common/qlcfixturemode.h"
#include "editmode.h"

#define KChannelsColumnNumber 0
#define KChannelsColumnName 1
#define KChannelsColumnPointer 2

EditMode::EditMode(QWidget* parent, QLCFixtureMode* mode)
{
	m_mode = new QLCFixtureMode(mode);
}

EditMode::~EditMode()
{
	delete m_mode;
}

void EditMode::init()
{
	QString str;
	QLCPhysical physical = m_mode->physical();
	
	/* Channel buttons */
	m_addChannelButton->setIconSet(QPixmap(QString(PIXMAPS) + 
				       QString("/edit_add.png")));
	m_removeChannelButton->setIconSet(QPixmap(QString(PIXMAPS) + 
					  QString("/edit_remove.png")));
	m_raiseChannelButton->setIconSet(QPixmap(QString(PIXMAPS) + 
					 QString("/up.png")));
	m_lowerChannelButton->setIconSet(QPixmap(QString(PIXMAPS) + 
					 QString("/down.png")));
	
	/* Mode name */
	m_modeNameEdit->setText(m_mode->name());
	
	/* Channels */
	refreshChannelList();

	/* Physical properties */
	m_bulbTypeCombo->setCurrentText(physical.bulbType());
	m_bulbLumensSpin->setValue(physical.bulbLumens());
	str.sprintf("%d", physical.bulbColourTemperature());
	m_bulbTempCombo->setCurrentText(str);
	
	m_weightSpin->setValue(physical.weight());
	m_widthSpin->setValue(physical.width());
	m_heightSpin->setValue(physical.height());
	m_depthSpin->setValue(physical.depth());

	m_lensNameCombo->setCurrentText(physical.lensName());
	m_lensMinDegreesSpin->setValue(physical.lensDegreesMin());
	m_lensMaxDegreesSpin->setValue(physical.lensDegreesMax());
	
	m_focusTypeCombo->setCurrentText(physical.focusType());
	m_panMaxSpin->setValue(physical.focusPanMax());
	m_tiltMaxSpin->setValue(physical.focusTiltMax());
}

void EditMode::slotModeNameChanged(const QString& text)
{
	m_mode->setName(text);
}

/****************************************************************************
 * Channels page functions
 ****************************************************************************/

void EditMode::slotAddChannelClicked()
{
	QPtrListIterator<QLCChannel> it(*m_mode->fixture()->channels());
	QLCChannel* ch = NULL;
	QStringList list;
	bool ok = false;
	QString name;
	int index = 0;
	
	/* Create a list of channels that have not been added to this mode yet */
	while ( (ch = it.current()) != 0 )
	{
		++it;
		if (m_mode->searchChannel(ch->name()) != NULL)
			continue;
		else
			list.append(ch->name());
	}
	
	name = QInputDialog::getItem("Add channel to mode", 
				     "Select a channel to add",
				     list, 0, false, &ok, this);
	
	if (ok == true && name.isEmpty() == false)
	{
		QListViewItem* item = NULL;
		int insertat = 0;
		
		ch = m_mode->fixture()->searchChannel(name);

		// Find out the current channel number
		item = m_channelList->currentItem();
		if (item != NULL)
			insertat = item->text(KChannelsColumnNumber).toInt() - 1;
		else
			insertat = 0;
		
		// Insert the item at current selection
		m_mode->insertChannel(ch, insertat);
		
		// Easier to refresh the whole list than to increment all
		// channel numbers after the inserted item
		refreshChannelList();
		
		// Select the new channel
		selectChannel(ch->name());
	}
}

void EditMode::slotRemoveChannelClicked()
{
	QLCChannel* ch = currentChannel();
	
	if (ch != NULL)
	{
		QListViewItem* item = NULL;
		QString select;

		// Pick the item above or below to be selected next
		item = m_channelList->currentItem()->itemAbove();
		if (item == NULL)
			item = m_channelList->currentItem()->itemBelow();
		if (item != NULL)
			select = item->text(KChannelsColumnName);

		// Remove the channel and the listview item
		m_mode->removeChannel(ch);
		delete m_channelList->currentItem();

		// Easier to refresh the whole list than to decrement all
		// channel numbers after the inserted item
		refreshChannelList();

		// Select another channel
		selectChannel(select);
	}
}

void EditMode::slotRaiseChannelClicked()
{
	QLCChannel* ch = currentChannel();
	int index = 0;
	
	if (ch == NULL)
		return;
	
	index = m_mode->channelNumber(ch) - 1;

	// Don't move beyond the beginning of the list
	if (index < 0)
		return;
	
	m_mode->removeChannel(ch);
	m_mode->insertChannel(ch, index);
	
	refreshChannelList();
	selectChannel(ch->name());
}

void EditMode::slotLowerChannelClicked()
{
	QLCChannel* ch = currentChannel();
	int index = 0;
	
	if (ch == NULL)
		return;
	
	index = m_mode->channelNumber(ch) + 1;
	
	// Don't move beyond the end of the list
	if (index >= m_mode->channels())
		return;
	
	m_mode->removeChannel(ch);
	m_mode->insertChannel(ch, index);
	
	refreshChannelList();
	selectChannel(ch->name());
}

void EditMode::refreshChannelList()
{
	QListViewItem* item = NULL;
	QLCChannel* ch = NULL;
	QString str;
	
	m_channelList->clear();
	
	for (int i = 0; i < m_mode->channels(); i++)
	{
		ch = m_mode->channel(i);
		item = new QListViewItem(m_channelList);
		str.sprintf("%.3d", i + 1);
		item->setText(KChannelsColumnNumber, str);
		item->setText(KChannelsColumnName, ch->name());
		
		// Store the channel pointer to the listview as a string
		str.sprintf("%d", (unsigned long) ch);
		item->setText(KChannelsColumnPointer, str);
	}
}

QLCChannel* EditMode::currentChannel()
{
	QLCChannel* ch = NULL;
	QListViewItem* item = NULL;

	// Convert the string-form ulong to a QLCChannel pointer and return it
	item = m_channelList->currentItem();
	if (item != NULL)
		ch = (QLCChannel*) item->text(KChannelsColumnPointer).toULong();

	return ch;
}

void EditMode::selectChannel(const QString &name)
{
	QListViewItemIterator it(m_channelList);
	
	while (it.current() != NULL)
	{
		if (it.current()->text(KChannelsColumnName) == name)
		{
			m_channelList->setSelected(it.current(), true);
			break;
		}
		
		++it;
	}
}

/****************************************************************************
 * Physical page functions
 ****************************************************************************/
void EditMode::slotBulbTypeChanged(const QString &type)
{
	QLCPhysical physical = m_mode->physical();
	physical.setBulbType(type);
	m_mode->setPhysical(physical);
}

void EditMode::slotBulbLumensChanged(int lumens)
{
	QLCPhysical physical = m_mode->physical();
	physical.setBulbLumens(lumens);
	m_mode->setPhysical(physical);
}

void EditMode::slotBulbColourTemperatureChanged(const QString &kelvins)
{
	QLCPhysical physical = m_mode->physical();
	physical.setBulbColourTemperature(kelvins.toInt());
	m_mode->setPhysical(physical);
}

void EditMode::slotWeightChanged(int weight)
{
	QLCPhysical physical = m_mode->physical();
	physical.setWeight(weight);
	m_mode->setPhysical(physical);
}

void EditMode::slotWidthChanged(int width)
{
	QLCPhysical physical = m_mode->physical();
	physical.setWidth(width);
	m_mode->setPhysical(physical);
}

void EditMode::slotHeightChanged(int height)
{
	QLCPhysical physical = m_mode->physical();
	physical.setHeight(height);
	m_mode->setPhysical(physical);
}

void EditMode::slotDepthChanged(int depth)
{
	QLCPhysical physical = m_mode->physical();
	physical.setDepth(depth);
	m_mode->setPhysical(physical);
}
	
void EditMode::slotLensNameChanged(const QString &name)
{
	QLCPhysical physical = m_mode->physical();
	physical.setLensName(name);
	m_mode->setPhysical(physical);
}

void EditMode::slotLensDegreesMinChanged(int degrees)
{
	QLCPhysical physical = m_mode->physical();
	physical.setLensDegreesMin(degrees);
	m_mode->setPhysical(physical);
}

void EditMode::slotLensDegreesMaxChanged(int degrees)
{
	QLCPhysical physical = m_mode->physical();
	physical.setLensDegreesMax(degrees);
	m_mode->setPhysical(physical);
}

void EditMode::slotFocusTypeChanged(const QString &type)
{
	QLCPhysical physical = m_mode->physical();
	physical.setFocusType(type);
	m_mode->setPhysical(physical);
}

void EditMode::slotFocusPanMaxChanged(int degrees)
{
	QLCPhysical physical = m_mode->physical();
	physical.setFocusPanMax(degrees);
	m_mode->setPhysical(physical);
}

void EditMode::slotFocusTiltMaxChanged(int degrees)
{
	QLCPhysical physical = m_mode->physical();
	physical.setFocusTiltMax(degrees);
	m_mode->setPhysical(physical);
}
