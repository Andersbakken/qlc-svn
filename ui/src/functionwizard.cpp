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

#include <cstdlib>

#include "fixtureselection.h"
#include "functionwizard.h"
#include "fixture.h"
#include "chaser.h"
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
	createGroupScenes();
	createIntensityChasers();

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

	QStringList caps;
	if (findChannels(fxi, KQLCChannelGroupColour).isEmpty() == false)
		caps << KQLCChannelGroupColour;
	if (findChannels(fxi, KQLCChannelGroupGobo).isEmpty() == false)
		caps << KQLCChannelGroupGobo;
	if (findChannels(fxi, KQLCChannelGroupShutter).isEmpty() == false)
		caps << KQLCChannelGroupShutter;
	if (findChannels(fxi, KQLCChannelGroupIntensity).isEmpty() == false)
		caps << KQLCChannelGroupIntensity;
	item->setText(KColumnCaps, caps.join(", "));
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

void FunctionWizard::createGroupScenes()
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
}

void FunctionWizard::createGroupFunctions(const Fixture* fxi, const QString& group)
{
	QList <t_channel> channels = findChannels(fxi, group);
	for (int i = 0; i < channels.size(); i++)
	{
		t_channel ch = channels.at(i);
		const QLCChannel* channel = fxi->channel(ch);
		Q_ASSERT(channel != NULL);

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
				name = QString("%1 (%2) - %3").arg(group).arg(i + 1)
							      .arg(cap->name());
			}
			else
			{
				// There's only one channel of the requested group
				// in the fixture (for example one gobo wheel)
				name = QString("%1 - %2").arg(group).arg(cap->name());
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

void FunctionWizard::createIntensityChasers()
{
	// Get the list of fixtures that user has selected
	QList <Fixture*> list(fixtures());

	// Even fixtures' intensity channels set full, others zero
	Scene* even = new Scene(_app->doc());
	even->setName("Automatic Even");

	// Odd fixtures' intensity channels set full, others zero
	Scene* odd = new Scene(_app->doc());
	odd->setName("Automatic Odd");

	// All fixtures' intensity channel set full
	Scene* full = new Scene(_app->doc());
	full->setName("Intensity - Full");

	// All fixtures' intensity channel set zero
	Scene* zero = new Scene(_app->doc());
	zero->setName("Intensity - Zero");

	QList <Scene*> sequence;
	QList <Scene*> random;
	int i = 1;
	foreach (Fixture* fxi, list)
	{
		Scene* sq = new Scene(_app->doc());
		sq->setName(tr("Intensity - ") + fxi->name());
		sequence << sq;

		sq = new Scene(_app->doc());
		sq->setName(tr("Intensity - Random - %1").arg(i++));
		random << sq;
	}

	// Scene list's size should be the same as fixture list's size
	Q_ASSERT(sequence.size() == list.size());
	Q_ASSERT(random.size() == list.size());

	// Initialize random seed
	srand(QDateTime::currentDateTime().toTime_t());

	// Go thru all fixtures and create functions for each of them
	for (int i = 0; i < list.size(); i++)
	{
		Fixture* fxi = list[i];
		Q_ASSERT(fxi != NULL);

		QList <t_channel> channels(findChannels(fxi, KQLCChannelGroupIntensity));
		for (int j = 0; j < channels.size(); j++)
		{
			t_channel ch = channels.at(j);
			const QLCChannel* channel = fxi->channel(ch);
			Q_ASSERT(channel != NULL);

			t_value min = 0;
			t_value max = 255;

			// The intensity channel contains also something else
			// than just a dimmer if it has more than one capability.
			// Try to be smart and guess which capability provides
			// dimmer intensity. Otherwise 0-255 is used.
			if (channel->capabilities().size() > 1)
			{
				const QLCCapability* cap = NULL;

				// Search for (I|i)ntensity or (D|d)immer capability
				cap = channel->searchCapability("ntensity", false);
				if (cap == NULL)
					cap = channel->searchCapability("immer", false);

				if (cap != NULL)
				{
					min = cap->min();
					max = cap->max();
				}
			}

			SceneValue fullValue(fxi->id(), ch, max);
			full->setValue(fullValue);

			SceneValue zeroValue(fxi->id(), ch, min);
			zero->setValue(zeroValue);

			if ((i % 2) == 0)
			{
				even->setValue(fxi->id(), ch, max);
				odd->setValue(fxi->id(), ch, min);
			}
			else
			{
				even->setValue(fxi->id(), ch, min);
				odd->setValue(fxi->id(), ch, max);
			}

			for (int s = 0; s < sequence.size(); s++)
			{
				if (s == i)
					sequence[s]->setValue(fullValue);
				else
					sequence[s]->setValue(zeroValue);

				if ((rand() % 2) == 0)
					random[s]->setValue(fullValue);
				else
					random[s]->setValue(zeroValue);
			}
		}
	}

	if (createOddEvenChaser(odd, even) == false)
	{
		delete odd;
		odd = NULL;
		delete even;
		even = NULL;
	}

	if (createFullZeroChaser(full, zero) == false)
	{
		delete full;
		full = NULL;
		delete zero;
		zero = NULL;
	}

	if (createSequenceChasers(sequence) == false)
	{
		foreach (Scene* sc, sequence)
			delete sc;
		sequence.clear();
	}

	if (createRandomChaser(random) == false)
	{
		foreach (Scene* sc, random)
			delete sc;
		random.clear();
	}
}

bool FunctionWizard::createOddEvenChaser(Scene* odd, Scene* even)
{
	if (odd == NULL || even == NULL)
		return false;

	// Create the chaser only if both steps contain something
	if (odd->values().size() != 0 && even->values().size() != 0)
	{
		// Abort if doc won't take the scenes
		if (_app->doc()->addFunction(odd) == false)
			return false;
		if (_app->doc()->addFunction(even) == false)
			return false;

		Chaser* evenOddChaser = new Chaser(_app->doc());
		evenOddChaser->setName("Intensity - Even/Odd");
		evenOddChaser->addStep(odd->id());
		evenOddChaser->addStep(even->id());

		// Abort if doc won't take the chaser
		if (_app->doc()->addFunction(evenOddChaser) == false)
		{
			delete evenOddChaser;
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		return false;
	}
}

bool FunctionWizard::createFullZeroChaser(Scene* full, Scene* zero)
{
	if (full == NULL || zero == NULL)
		return false;

	// Create the chaser only if both steps contain something
	if (full->values().size() != 0 && zero->values().size() != 0)
	{
		// Abort if doc won't take the scenes
		if (_app->doc()->addFunction(full) == false)
			return false;
		if (_app->doc()->addFunction(zero) == false)
			return false;

		Chaser* fullZeroChaser = new Chaser(_app->doc());
		fullZeroChaser->setName("Intensity - Full/Zero");
		fullZeroChaser->addStep(full->id());
		fullZeroChaser->addStep(zero->id());

		if (_app->doc()->addFunction(fullZeroChaser) == false)
		{
			delete fullZeroChaser;
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		return false;
	}
}

bool FunctionWizard::createSequenceChasers(const QList <Scene*>& sequence)
{
	// Abort immediately if doc won't take all sequence steps
	foreach (Scene* sq, sequence)
	{
		if (_app->doc()->addFunction(sq) == false)
			return false;
	}

	if (createForwardSequenceChaser(sequence) == false)
		return false;

	if (createBackwardSequenceChaser(sequence) == false)
		return false;

	return true;
}

bool FunctionWizard::createForwardSequenceChaser(const QList <Scene*>& sequence)
{
	if (sequence.size() == 0)
		return false;

	// Create a forward sequence chaser
	Chaser* seqChaser = new Chaser(_app->doc());
	seqChaser->setName("Intensity - Sequence Forward");
	if (_app->doc()->addFunction(seqChaser) == false)
	{
		// Abort if doc won't accept the chaser
		delete seqChaser;
		return false;
	}
	else
	{
		for (int i = 0; i < sequence.size(); i++)
			seqChaser->addStep(sequence[i]->id());
		return true;
	}
}

bool FunctionWizard::createBackwardSequenceChaser(const QList <Scene*>& sequence)
{
	if (sequence.size() == 0)
		return false;

	// Create a forward sequence chaser
	Chaser* seqChaser = new Chaser(_app->doc());
	seqChaser->setName("Intensity - Sequence Forward");
	if (_app->doc()->addFunction(seqChaser) == false)
	{
		// Abort if doc won't accept the chaser
		delete seqChaser;
		return false;
	}
	else
	{
		for (int i = sequence.size() - 1; i > 0; i--)
			seqChaser->addStep(sequence[i]->id());
		return true;
	}
}

bool FunctionWizard::createRandomChaser(const QList <Scene*>& random)
{
	if (random.size() == 0)
		return false;

	// Abort immediately if doc won't take all sequence steps
	foreach (Scene* rnd, random)
	{
		if (_app->doc()->addFunction(rnd) == false)
			return false;
	}

	// Create a random chaser
	Chaser* rndChaser = new Chaser(_app->doc());
	rndChaser->setName("Intensity - Random");
	if (_app->doc()->addFunction(rndChaser) == false)
	{
		// Abort if doc won't accept the chaser
		delete rndChaser;
		return false;
	}
	else
	{
		for (int i = 0; i < random.size(); i++)
			rndChaser->addStep(random[i]->id());
		return true;
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
		if (channel->group() == group)
			channels << ch;
	}

	return channels;
}
