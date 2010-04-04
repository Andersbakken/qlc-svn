/*
  Q Light Controller
  functionwizard.cpp

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

#include <QString>
#include <QDebug>
#include <QHash>

#include "fixtureselection.h"
#include "functionwizard.h"
#include "fixture.h"
#include "scene.h"
#include "app.h"
#include "doc.h"

#include "qlccapability.h"
#include "qlcchannel.h"

#define KColumnName 0
#define KColumnCaps 1
#define KColumnID   2

extern App* _app;

FunctionWizard::FunctionWizard(QWidget* parent) : QDialog(parent)
{
	setupUi(this);
}

FunctionWizard::~FunctionWizard()
{
}

void FunctionWizard::slotAddClicked()
{
	FixtureSelection fs(this, _app->doc(), true, QList <t_fixture_id>());
	if (fs.exec() == QDialog::Accepted)
	{
		QListIterator <t_fixture_id> it(fs.selection);
		while (it.hasNext() == true)
			addFixture(it.next());
	}
}

void FunctionWizard::slotRemoveClicked()
{
	QListIterator <QTreeWidgetItem*> it(m_fixtureTree->selectedItems());
	while (it.hasNext() == true)
		delete it.next();
}

void FunctionWizard::accept()
{
	/* Create functions for each selected fixture */
	QListIterator <Fixture*> fxiit(fixtures());
	while (fxiit.hasNext() == true)
	{
		Fixture* fxi(fxiit.next());
		Q_ASSERT(fxi != NULL);

		if (m_coloursCheck->isChecked() == true)
			createGroupFunctions(fxi, KQLCChannelGroupColour);
		if (m_goboCheck->isChecked() == true)
			createGroupFunctions(fxi, KQLCChannelGroupGobo);
		if (m_shutterCheck->isChecked() == true)
			createGroupFunctions(fxi, KQLCChannelGroupShutter);
	}

	/* Add all created functions to Doc */
	QHashIterator <QString,Scene*> hashit(m_scenes);
	while (hashit.hasNext() == true)
	{
		hashit.next();
		_app->doc()->addFunction(hashit.value());
	}

	QDialog::accept();
}

/****************************************************************************
 * Fixtures
 ****************************************************************************/

void FunctionWizard::addFixture(t_fixture_id fxi_id)
{
	Fixture* fxi = _app->doc()->fixture(fxi_id);
	Q_ASSERT(fxi != NULL);

	QTreeWidgetItem* item = new QTreeWidgetItem(m_fixtureTree);
	item->setText(KColumnName, fxi->name());
	item->setData(KColumnID, Qt::UserRole, fxi_id);

	QString caps;
	if (findChannels(fxi, KQLCChannelGroupColour).isEmpty() == false)
		caps.append(KQLCChannelGroupColour);
	if (findChannels(fxi, KQLCChannelGroupGobo).isEmpty() == false)
		caps.append(QString(", ") + KQLCChannelGroupGobo);
	if (findChannels(fxi, KQLCChannelGroupShutter).isEmpty() == false)
		caps.append(QString(", ") + KQLCChannelGroupShutter);
	item->setText(KColumnCaps, caps);
}

QList <Fixture*> FunctionWizard::fixtures() const
{
	QList <Fixture*> list;
	for (int i = 0; i < m_fixtureTree->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item(m_fixtureTree->topLevelItem(i));
		Q_ASSERT(item != NULL);

		t_fixture_id id = item->data(KColumnID, Qt::UserRole).toInt();
		Fixture* fxi = _app->doc()->fixture(id);
		Q_ASSERT(fxi != NULL);

		// Skip fixtures that we know nothing about
		if (fxi->isDimmer() == false)
			list << fxi;
	}

	return list;
}

/****************************************************************************
 * Function creation
 ****************************************************************************/

void FunctionWizard::createGroupFunctions(const Fixture* fxi, const QString& group)
{
	QList <t_channel> channels = findChannels(fxi, group);
	for (int i = 0; i < channels.size(); i++)
	{
		t_channel ch = channels.at(i);
		const QLCChannel* channel = fxi->channel(ch);
		if (channel == NULL)
			return;

		QListIterator <QLCCapability*> capit(channel->capabilities());
		while (capit.hasNext() == true)
		{
			const QLCCapability* cap = capit.next();
			Q_ASSERT(cap != NULL);

			QString name;
			if (channels.size() > 1)
			{
				// There's more than one channel (for example two
				// gobo channels, several distinct color wheels...)
				name = tr("%1 (%2) - %3").arg(group).arg(i + 1)
							 .arg(cap->name());
			}
			else
			{
				// There's only one channel of the requested group
				// in the fixture (for example one gobo wheel)
				name = tr("%1 - %2").arg(group).arg(cap->name());
			}

			Scene* scene = NULL;
			if (m_scenes.contains(name) == true)
			{
				// Append this fixture's values to a scene with the
				// same name, that contains other similar values.
				scene = m_scenes[name];
			}
			else
			{
				scene = new Scene(_app->doc());
				scene->setName(name);
				m_scenes[name] = scene;
			}

			SceneValue val(fxi->id(), ch, cap->middle());
			scene->setValue(val);
		}
	}
}

QList <t_channel> FunctionWizard::findChannels(const Fixture* fixture,
						const QString& group) const
{
	QList <t_channel> channels;

	Q_ASSERT(fixture != NULL);
	for (t_channel ch = 0; ch < fixture->channels(); ch++)
	{
		const QLCChannel* channel(fixture->channel(ch));
		Q_ASSERT(channel != NULL);
		if (channel->group() == group && channel->capabilities().size() > 2)
			channels << ch;
	}

	return channels;
}
