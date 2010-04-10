/*
  Q Light Controller
  intelligentintensitygenerator.cpp

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

#include <QDateTime>

#include "qlccapability.h"
#include "qlcchannel.h"

#include "intelligentintensitygenerator.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"

IntelligentIntensityGenerator::IntelligentIntensityGenerator(Doc* doc,
                                      const QList <Fixture*>& fxiList)
	: m_doc(doc)
	, fixtures(fxiList)
	, odd(NULL)
	, even(NULL)
	, full(NULL)
	, zero(NULL)
{
	Q_ASSERT(doc != NULL);
	Q_ASSERT(fxiList.size() != 0);

	// Remove all dimmers from fixture list
	QMutableListIterator <Fixture*> it(fixtures);
	while (it.hasNext() == true)
	{
		it.next();
		if (it.value()->isDimmer() == true)
			it.remove();
	}

	// Initialize random seed
	srand(QDateTime::currentDateTime().toTime_t());

	// Create member scenes
	createScenes();
}

IntelligentIntensityGenerator::~IntelligentIntensityGenerator()
{
	// Destroy all functions that have NOT been added to Doc
	if (m_doc->function(odd->id()) == NULL)
		delete odd;
	odd = NULL;

	if (m_doc->function(even->id()) == NULL)
		delete even;
	even = NULL;

	if (m_doc->function(full->id()) == NULL)
		delete full;
	full = NULL;

	if (m_doc->function(zero->id()) == NULL)
		delete zero;
	zero = NULL;

	QListIterator <Scene*> seqit(sequence);
	while (seqit.hasNext() == true)
	{
		Scene* scene(seqit.next());
		if (m_doc->function(scene->id()) == NULL)
			delete scene;
	}
	sequence.clear();

	QListIterator <Scene*> rndit(random);
	while (rndit.hasNext() == true)
	{
		Scene* scene(rndit.next());
		if (m_doc->function(scene->id()) == NULL)
			delete scene;
	}
	random.clear();
}

bool IntelligentIntensityGenerator::createOddEvenChaser()
{
	if (odd == NULL || even == NULL)
		return false;

	// Abort if doc can't fit the chaser and its two members
	if (m_doc->functionsFree() < 3)
		return false;

	// Create the chaser only if both steps contain something
	if (odd->values().size() != 0 && even->values().size() != 0)
	{
		// Abort if doc won't take the scenes
		if (m_doc->addFunction(odd) == false)
			return false;
		if (m_doc->addFunction(even) == false)
			return false;

		Chaser* chaser = new Chaser(m_doc);
		chaser->setName("Intensity - Even/Odd");
		chaser->addStep(odd->id());
		chaser->addStep(even->id());

		// Abort if doc won't take the chaser
		if (m_doc->addFunction(chaser) == false)
		{
			delete chaser;
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

bool IntelligentIntensityGenerator::createFullZeroChaser()
{
 	if (full == NULL || zero == NULL)
		return false;

	// Abort if doc can't fit the chaser and its two members
	if (m_doc->functionsFree() < 3)
		return false;

	// Create the chaser only if both steps contain something
	if (full->values().size() != 0 && zero->values().size() != 0)
	{
		// Abort if doc won't take the scenes
		if (m_doc->addFunction(full) == false)
			return false;
		if (m_doc->addFunction(zero) == false)
			return false;

		Chaser* chaser = new Chaser(m_doc);
		chaser->setName("Intensity - Full/Zero");
		chaser->addStep(full->id());
		chaser->addStep(zero->id());

		if (m_doc->addFunction(chaser) == false)
		{
			delete chaser;
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

bool IntelligentIntensityGenerator::createSequenceChasers()
{
	if (sequence.size() == 0)
		return false;

	// Abort if doc can't fit the two chasers and their sequence members
	if (m_doc->functionsFree() < quint32(sequence.size() + 2))
		return false;

	// Abort immediately if doc won't take all sequence steps
	QListIterator <Scene*> it(sequence);
	while (it.hasNext() == true)
	{
		Scene* scene(it.next());
		if (m_doc->addFunction(scene) == false)
			return false;
	}

	// Forward
	if (createSequenceChaser(true) == false)
		return false;

	// Backward
	if (createSequenceChaser(false) == false)
		return false;

	return true;
}

bool IntelligentIntensityGenerator::createRandomChaser()
{
	if (random.size() == 0)
		return false;

	// Abort if doc can't fit the chaser and its members
	if (m_doc->functionsFree() < quint32(random.size() + 1))
		return false;

	// Abort immediately if doc won't take all sequence steps
	QListIterator <Scene*> it(random);
	while (it.hasNext() == true)
	{
		if (m_doc->addFunction(it.next()) == false)
			return false;
	}

	// Create the random chaser
	Chaser* chaser = new Chaser(m_doc);
	chaser->setName("Intensity - Random");
	if (m_doc->addFunction(chaser) == false)
	{
		// Abort if doc won't accept the chaser
		delete chaser;
		return false;
	}
	else
	{
		for (int i = 0; i < random.size(); i++)
			chaser->addStep(random[i]->id());
		return true;
	}
}

void IntelligentIntensityGenerator::createScenes()
{
	odd = new Scene(m_doc);
	odd->setName("Intensity - Odd");

	even = new Scene(m_doc);
	even->setName("Intensity - Even");

	full = new Scene(m_doc);
	full->setName("Intensity - Full");

	zero = new Scene(m_doc);
	zero->setName("Intensity - Zero");

	// Create sequence & random scene lists
	int i = 0;
	QListIterator <Fixture*> it(fixtures);
	while (it.hasNext() == true)
	{
		Fixture* fxi(it.next());
		Q_ASSERT(fxi != NULL);

		Scene* sq = new Scene(m_doc);
		sq->setName(QString("Intensity - ") + fxi->name());
		sequence << sq;

		sq = new Scene(m_doc);
		sq->setName(QString("Intensity - Random - %1").arg(++i));
		random << sq;
	}

	// Go thru all fixtures
	for (int i = 0; i < fixtures.size(); i++)
	{
		Fixture* fxi = fixtures[i];
		Q_ASSERT(fxi != NULL);

		// Find such channels from the fixture that belong to the
		// given channel group.
		QList <t_channel> channels =
				findChannels(fxi, KQLCChannelGroupIntensity);

		// Insert values to member scenes for each found channel
		for (int j = 0; j < channels.size(); j++)
		{
			t_channel ch = channels.at(j);
			const QLCChannel* channel = fxi->channel(ch);
			Q_ASSERT(channel != NULL);

			t_value min = 0;
			t_value max = 255;

			// Find the minimum and maximum intensity values for
			// the current channel
			findMinMax(channel, &min, &max);

			// Set all intensity channels to max in the $full scene
			full->setValue(fxi->id(), ch, max);

			// Set all intensity channels to min in the $zero scene
			zero->setValue(fxi->id(), ch, min);

			// Create even & odd values
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

			// Create sequence and random values
                        for (int s = 0; s < sequence.size(); s++)
                        {
                                if (s == i)
                                        sequence[s]->setValue(fxi->id(), ch, max);
                                else
                                        sequence[s]->setValue(fxi->id(), ch, min);

                                if ((rand() % 2) == 0)
                                        random[s]->setValue(fxi->id(), ch, max);
                                else
                                        random[s]->setValue(fxi->id(), ch, min);
                        }
                }
        }
}

bool IntelligentIntensityGenerator::createSequenceChaser(bool forward)
{
	// Create a sequence chaser
	Chaser* chaser = new Chaser(m_doc);
	if (forward == true)
	{
		chaser->setName("Intensity - Sequence Forward");
		chaser->setDirection(Function::Forward);
	}
	else
	{
		chaser->setName("Intensity - Sequence Backward");
		chaser->setDirection(Function::Backward);
	}

	if (m_doc->addFunction(chaser) == false)
	{
		// Abort if doc won't accept the chaser
		delete chaser;
		return false;
	}
	else
	{
		for (int i = 0; i < sequence.size(); i++)
			chaser->addStep(sequence[i]->id());
		return true;
	}
}

bool IntelligentIntensityGenerator::findMinMax(const QLCChannel* channel,
						t_value* min, t_value* max)
{
	Q_ASSERT(channel != NULL);
	Q_ASSERT(min != NULL);
	Q_ASSERT(max != NULL);

	// The intensity channel contains also something else than just a
	// dimmer if it has more than one capability and a search is necessary.
	// Try to be smart and guess which capability provides dimmer
	// intensity. If a suitable capability is not found, values are not
	// modified.
	if (channel->capabilities().size() > 1)
	{
		const QLCCapability* cap = NULL;

		// Search for (I|i)ntensity or (D|d)immer capability
		cap = channel->searchCapability("ntensity", false);
		if (cap == NULL)
			cap = channel->searchCapability("immer", false);

		if (cap != NULL)
		{
			*min = cap->min();
			*max = cap->max();

			return true;
		}
	}

	return false;
}

QList <t_channel> IntelligentIntensityGenerator::findChannels(
			const Fixture* fixture, const QString& group)
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
