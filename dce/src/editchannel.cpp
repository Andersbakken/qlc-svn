/*
  Q Light Controller - Device Class Editor
  editchannel.cpp

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

#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlistview.h>
#include <qmessagebox.h>

#include "common/qlcchannel.h"
#include "common/qlccapability.h"
#include "editchannel.h"
#include "editcapability.h"

#define KColumnMin 0
#define KColumnMax 1
#define KColumnName 2
#define KColumnPointer 3

EditChannel::EditChannel(QWidget* parent, QLCChannel* channel)
	: UI_EditChannel(parent)
{
	m_channel = new QLCChannel(channel);
}

EditChannel::~EditChannel()
{
	if (m_channel != NULL)
		delete m_channel;
}

void EditChannel::init()
{
	QStringList groupList;

	Q_ASSERT(m_channel != NULL);
	
	/* Set some button pixmaps */
	m_addCapabilityButton->setIconSet(QPixmap(QString(PIXMAPS) +
					  QString("/edit_add.png")));
	m_removeCapabilityButton->setIconSet(QPixmap(QString(PIXMAPS) +
					     QString("/edit_remove.png")));
	m_editCapabilityButton->setIconSet(QPixmap(QString(PIXMAPS) +
					   QString("/edit.png")));

	/* Get available groups */
	QLCChannel::groups(groupList);

	/* Set window caption */
	setCaption(QString("Edit Channel: ") + m_channel->name());

	/* Set name edit */
	m_nameEdit->setText(m_channel->name());

	/* Insert groups into the combo */
	m_groupCombo->insertStringList(groupList);

	/* Select group */
	for (int i = 0; i < m_groupCombo->count(); i++)
	{
		if (m_groupCombo->text(i) == m_channel->group())
		{
			m_groupCombo->setCurrentItem(i);
			break;
		}
	}
	
	refreshCapabilities();
}

void EditChannel::slotNameChanged(const QString& name)
{
	m_channel->setName(name);
}

void EditChannel::slotGroupActivated(const QString& group)
{
	m_channel->setGroup(group);

	if (group == QString(KPanGroup) || group == QString(KTiltGroup))
	{
		m_controlByteGroup->setEnabled(true);
		m_controlByteGroup->setButton(m_channel->controlByte());
	}
	else
	{
		m_controlByteGroup->setEnabled(false);
		m_controlByteGroup->setButton(m_channel->controlByte());
	}
}

void EditChannel::slotControlByteActivated(int button)
{
	m_channel->setControlByte(button);
}

/****************************************************************************
 * Capability list functions
 ****************************************************************************/

void EditChannel::slotCapabilityListSelectionChanged(QListViewItem* item)
{
	if (item == NULL)
	{
		m_removeCapabilityButton->setEnabled(false);
		m_editCapabilityButton->setEnabled(false);
	}
	else
	{
		m_removeCapabilityButton->setEnabled(true);
		m_editCapabilityButton->setEnabled(true);
	}
}

void EditChannel::slotAddCapabilityClicked()
{
	EditCapability* ec = NULL;
	QLCCapability* cap = NULL;
	bool ok = false;
	
	ec = new EditCapability(this);
	ec->init();

	while (ok == false)
	{
		if (ec->exec() == QDialog::Accepted)
		{
			cap = new QLCCapability(ec->capability());

			if (m_channel->addCapability(cap) == false)
			{
				QMessageBox::warning(this, 
					QString("Overlapping values"),
					QString("The capability's values overlap with another capability!"));
				delete cap;
				ok = false;
			}
			else
			{
				refreshCapabilities();
				ok = true;
			}
		}
		else
		{
			ok = true;
		}
	}
	
	delete ec;
}

void EditChannel::slotRemoveCapabilityClicked()
{
	// This also deletes the capability
	m_channel->removeCapability(currentCapability());
	
	refreshCapabilities();
	slotCapabilityListSelectionChanged(m_capabilityList->currentItem());
}

void EditChannel::slotEditCapabilityClicked()
{
	EditCapability* ec = NULL;
	QLCCapability* real = NULL;
	QLCCapability* min = NULL;
	QLCCapability* max = NULL;
	bool ok = false;
	
	real = currentCapability();
	if (real == NULL)
		return;

	ec = new EditCapability(this, real);
	ec->init();
	
	while (ok == false)
	{
		if (ec->exec() == QDialog::Accepted)
		{
			min = m_channel->searchCapability(ec->capability()->min());
			max = m_channel->searchCapability(ec->capability()->max());
			if ((min != NULL && min != real) ||
			    (max != NULL && max != real))
			{
				QMessageBox::warning(this, 
					QString("Overlapping values"),
					QString("The capability's values overlap with another capability!"));
				ok = false;
			}
			else
			{
				*real = *ec->capability();
				refreshCapabilities();
				ok = true;
			}
		}
		else
		{
			ok = true;
		}
	}
	
	delete ec;
}

void EditChannel::refreshCapabilities()
{
	QPtrListIterator<QLCCapability> it(*m_channel->capabilities());
	QLCCapability* cap = NULL;
	QListViewItem* item = NULL;
	QString str;
	
	m_capabilityList->clear();
	
	/* Fill capabilities */
	while ( (cap = it.current()) != 0)
	{
		++it;

		item = new QListViewItem(m_capabilityList);
		
		// Min
		str.sprintf("%.3d", cap->min());
		item->setText(KColumnMin, str);

		// Max
		str.sprintf("%.3d", cap->max());
		item->setText(KColumnMax, str);
		
		// Name
		item->setText(KColumnName, cap->name());
		
		// Store the capability pointer to the listview as a string
		str.sprintf("%d", (unsigned long) cap);
		item->setText(KColumnPointer, str);
	}
	
	slotCapabilityListSelectionChanged(m_capabilityList->currentItem());
}

QLCCapability* EditChannel::currentCapability()
{
	QLCCapability* cap = NULL;
	QListViewItem* item = NULL;

	// Convert the string-form ulong to a QLCChannel pointer and return it
	item = m_capabilityList->currentItem();
	if (item != NULL)
		cap = (QLCCapability*) item->text(KColumnPointer).toULong();

	return cap;
}
