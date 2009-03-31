/*
  Q Light Controller
  cue.h

  Copyright (C) Christopher Staite

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

#ifndef CUE_H
#define CUE_H

#include "scene.h"

#define KXMLQLCSceneValue "Value"
#define KXMLQLCSceneValueFixture "Fixture"
#define KXMLQLCSceneValueChannel "Channel"

/*****************************************************************************
 * CueChannel
 *****************************************************************************/

class CueChannel : public SceneChannel
{
public:
	CueChannel();
	CueChannel(const CueChannel& sch);
	~CueChannel();

	CueChannel& operator=(const CueChannel& sch);

public:
	/** The universe and channel that this object refers to */
	t_channel address;

	/** The value of the channel where a scene started fading from */
	int start;

	/** The current value set by a scene */
	int current;

	/** The target value to eventually fade to */
	int target;

	/** If true, this value is ready, don't set it anymore to DMX */
	bool ready;
};

/*****************************************************************************
 * CueValue
 *****************************************************************************/

class CueValue : public SceneValue
{
public:
	/** Normal constructor */
	CueValue(t_fixture_id fxi_id, t_channel channel, t_value value);

	/** Copy constructor */
	CueValue(const SceneValue& scv);

	/** Load constructor */
	CueValue(QDomElement* tag);

	/** Destructor */
	~CueValue();

	/** A SceneValue is not valid if .fxi == KNoID */
	bool isValid();

	/** Comparator function for qSort() */
	bool operator< (const CueValue& scv) const;

	/** Comparator function for matching SceneValues */
	bool operator== (const CueValue& scv) const;

	/** Save this SceneValue to XML file */
	bool saveXML(QDomDocument* doc, QDomElement* scene_root) const;

public:
	t_fixture_id fxi;
	t_channel channel;
	t_value value;
};

/*****************************************************************************
 * Cue
 *****************************************************************************/

class Cue : public Scene
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/** Construct a new cue function, with parent object (Doc) */
	Cue(QObject* parent);

	/** Destroy the cue */
	~Cue();

private:
	/* Disable copying with a copy constructor & operator= */
	Q_DISABLE_COPY(Cue)

	/*********************************************************************
	 * Copying
	 *********************************************************************/
public:
	/** Create a copy of this function */
	Function* createCopy();

	/** Copy the contents for this function from another function */
	bool copyFrom(const Function* function);

	/*********************************************************************
	 * Values
	 *********************************************************************/
public:
	/**
	 * Set the value of one fixture channel, using a predefined CueValue
	 */
	void setValue(CueValue scv);

	/**
	 * Set the value of one fixture channel, specify parameters separately
	 */
	void setValue(t_fixture_id fxi, t_channel ch, t_value value);

	/**
	 * Clear the value of one fixture channel
	 */
	void unsetValue(t_fixture_id fxi, t_channel ch);

	/**
	 * Get the value of one fixture channel
	 */
	t_value value(t_fixture_id fxi, t_channel ch);

	/**
	 * Write the scene values to OutputMap. If fxi_id is given, writes
	 * values only for the specified fixture. 
	 */
	void writeValues(t_fixture_id fxi_id = KNoID);

	/**
	 * Write zeros for all involved channels to OutputMap. If fxi_id is
	 * given, writes zeros only for the specified fixture. 
	 */
	void writeZeros(t_fixture_id fxi_id = KNoID);

	/**
	 * Get a list of values in this scene
	 */
	QList <CueValue> *values() { return &m_values; }

protected:
	QList <CueValue> m_values;

	/*********************************************************************
	 * Functions
	 *********************************************************************/
public:
	/** Adds a function to the cue such that it is started with it */
	bool addFunction(Function f);
	/** Removes a function from the cue */
	bool removeFunction(Function f);

private:
	QList<Function> m_functions;

	/*********************************************************************
	 * Fading
	 *********************************************************************/
public:
	/**
	 * Prepares the fixtures for this cue if they are not in use
	 */
	void prepareCue();

private:
	/** Time to wait before running this cue */
	int delayUpTime;
	/** Time to fade up the cue */
	int fadeUpTime;
	/** The time to stay on this cue before automatically going on, -1 is for manual cue */
	int delayDownTime;
	/** Time to wait when next cue is triggered before getting it go */
	int waitDownTime;

	/*********************************************************************
	 * Edit
	 *********************************************************************/
public:
	/** Edit the function. Returns QDialog::DialogCode. */
	int edit();

	/*********************************************************************
	 * Fixtures
	 *********************************************************************/
public slots:
	void slotFixtureRemoved(t_fixture_id fxi_id);

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	bool saveXML(QDomDocument* doc, QDomElement* wksp_root);
	bool loadXML(const QDomElement* root);

	/*********************************************************************
	 * Flash
	 *********************************************************************/
public:
	void flash();
	void unFlash();

	/*********************************************************************
	 * Running
	 *********************************************************************/
public:
	void arm();
	void disarm();

	bool write(QByteArray* universes);

protected:
	t_value nextValue(SceneChannel* sch);

protected:
	QList <CueChannel> m_channels;
};

#endif
