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

#include "common/qlcchannel.h"
#include "common/qlccapability.h"
#include "editchannel.h"

static const int KColumnMin    ( 0 );
static const int KColumnMax    ( 1 );
static const int KColumnName   ( 2 );

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
	Q_ASSERT(m_channel != NULL);
	
	QPtrListIterator<QLCCapability> it(*m_channel->capabilities());
	QLCCapability* cap = NULL;
	QListViewItem* item = NULL;
	QStringList groupList;
	QString str;

	/* Set some button pixmaps */
	m_addCapabilityButton->setIconSet(QPixmap(QString(PIXMAPS) +
					  QString("/edit_add.png")));
	m_removeCapabilityButton->setIconSet(QPixmap(QString(PIXMAPS) +
					     QString("/edit_remove.png")));
	m_editCapabilityButton->setIconSet(QPixmap(QString(PIXMAPS) +
					   QString("/edit.png")));

	/* Get available groups */
	QLCChannel::groups(groupList);

	/* Insert groups into the combo */
	m_groupCombo->insertStringList(groupList);

	/* Set edit window caption */
	setCaption(QString("Edit Channel: ") + m_channel->name());

	/* Set name edit */
	m_nameEdit->setText(m_channel->name());

	/* Select group */
	for (int i = 0; i < m_groupCombo->count(); i++)
	{
		if (m_groupCombo->text(i) == m_channel->group())
		{
			m_groupCombo->setCurrentItem(i);
			break;
		}
	}

	/* Fill capabilities */
	while ( (cap = it.current()) != 0)
	{
		++it;

		item = new QListViewItem(m_capabilityList);
		item->setText(KColumnName, cap->name());
		
		str.sprintf("%.3d", cap->min());
		item->setText(KColumnMin, str);

		str.sprintf("%.3d", cap->max());
		item->setText(KColumnMax, str);
	}
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
