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
	 * @param fxiList List of fixture to create functions for
	 */
	IntelligentIntensityGenerator(Doc* doc,
				      const QList <Fixture*>& fxiList);
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
	QList <Fixture*> fixtures;

	Scene* odd;
	Scene* even;

	Scene* full;
	Scene* zero;

	QList <Scene*> sequence;
	QList <Scene*> random;
};

#endif
