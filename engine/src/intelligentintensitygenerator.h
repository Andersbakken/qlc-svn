/*
  Q Light Controller
  intelligentintensitygenerator.h

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

#ifndef INTELLIGENTINTENSITYGENERATOR_H
#define INTELLIGENTINTENSITYGENERATOR_H

#include <QList>
#include "qlctypes.h"

class QLCChannel;
class Fixture;
class Scene;
class Doc;

/**
 * Generates scenes and chasers for intelligent lights (scanners, moving heads
 * and the like) using their intensity channel(s).
 */
class IntelligentIntensityGenerator
{
public:
	/**
	 * Create a new instance.
	 *
	 * @param doc The Doc instance that should take the created functions
	 * @param fxis List of fixtures to create functions for
	 */
	IntelligentIntensityGenerator(Doc* doc, const QList <Fixture*>& fxis);

	/** Destructor */
	~IntelligentIntensityGenerator();

	/**
	 * Create a chaser that blinks every other intensity channel
	 * sequentially. Attempts to add $odd and $even to $doc as well
	 * as the chaser that is created.
	 *
	 * If just one of the functions cannot be added to $doc, the whole
	 * operation will fail and nothing gets added to $doc.
	 */
	bool createOddEvenChaser();

	/**
	 * Create a chaser that blinks all intensity channels on and off
	 * sequentially. Attempts to add $full and $zero to $doc as well
	 * as the chaser that is created.
	 *
	 * If just one of the functions cannot be added to $doc, the whole
	 * operation will fail and nothing gets added to $doc.
	 */
	bool createFullZeroChaser();

	/**
	 * Create a chaser that runs each intensity channels on and then off
	 * separately. Attempts to add the contents of $sequence to $doc as
	 * well as the chaser that is created.
	 *
	 * If just one of the functions cannot be added to $doc, the whole
	 * operation will fail and nothing gets added to $doc.
	 */
	bool createSequenceChasers();

	/**
	 * Create a chaser that toggles a random set of intensity channels
	 * on or off sequentially. Attempts to add the contents of $random to
	 * $doc as well as the chaser that is created.
	 *
	 * If just one of the functions cannot be added to $doc, the whole
	 * operation will fail and nothing gets added to $doc.
	 */
	bool createRandomChaser();

protected:
	/** Create the scenes that are used in the generated chasers */
	void createScenes();

	/** Create a sequence chaser and make it run forwards or backwards */
	bool createSequenceChaser(bool forward);

	/** Find the minimum and maximum intensity values from the channel */
	bool findMinMax(const QLCChannel* channel, t_value* min, t_value* max);

public:
	/** Find a list of fixture channels belonging to the given group */
	static QList <t_channel> findChannels(const Fixture* fixture,
					      const QString& group);

private:
	Doc* m_doc;
	QList <Fixture*> m_fixtures;

	Scene* m_odd;
	Scene* m_even;

	Scene* m_full;
	Scene* m_zero;

	QList <Scene*> m_sequence;
	QList <Scene*> m_random;
};

#endif
