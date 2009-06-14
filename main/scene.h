/*
  Q Light Controller
  scene.h

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

#ifndef SCENE_H
#define SCENE_H

#include <QList>
#include <QtXml>

#include "common/qlctypes.h"
#include "function.h"
#include "fixture.h"

#define KXMLQLCSceneValue "Value"
#define KXMLQLCSceneValueFixture "Fixture"
#define KXMLQLCSceneValueChannel "Channel"

class QByteArray;

/*****************************************************************************
 * SceneChannel
 *****************************************************************************/

/**
 * SceneChannel is a helper class used to store individual RUNTIME values for
 * channels as they are operated by a Scene function during Operate mode.
 *
 * Although t_value is an UNSIGNED char (0-255), these variables must be SIGNED
 * because the floating-point calculations that Scene does don't necessarily
 * stop exactly at 0.0 and 255.0, but might go slightly over/under. If these
 * variables were unsigned, an overflow would occur in some cases, resulting in
 * UINT_MAX values and everything would go wacko.
 */
class SceneChannel
{
public:
	SceneChannel();
	SceneChannel(const SceneChannel& sch);
	~SceneChannel();

	SceneChannel& operator=(const SceneChannel& sch);

public:
	/** The universe and channel that this object refers to */
	t_channel address;

	/** The value of the channel where a scene started fading from */
	qint32 start;

	/** The current value set by a scene */
	qint32 current;

	/** The target value to eventually fade to */
	qint32 target;

	/** If true, this value is ready, don't set it anymore to DMX */
	bool ready;
};

/*****************************************************************************
 * SceneValue
 *****************************************************************************/

/**
 * SceneValue is a helper class used to store individual channel TARGET values
 * for Scene functions. Each channel that is taking part of a scene is
 * represented with a SceneValue.
 */
class SceneValue
{
public:
	/** Normal constructor */
	SceneValue(t_fixture_id fxi_id, t_channel channel, t_value value);

	/** Copy constructor */
	SceneValue(const SceneValue& scv);

	/** Load constructor */
	SceneValue(QDomElement* tag);

	/** Destructor */
	~SceneValue();

	/** A SceneValue is not valid if .fxi == KNoID */
	bool isValid();

	/** Comparator function for qSort() */
	bool operator< (const SceneValue& scv) const;

	/** Comparator function for matching SceneValues */
	bool operator== (const SceneValue& scv) const;

	/** Save this SceneValue to XML file */
	bool saveXML(QDomDocument* doc, QDomElement* scene_root) const;

public:
	t_fixture_id fxi;
	t_channel channel;
	t_value value;
};

/*****************************************************************************
 * Scene
 *****************************************************************************/

/**
 * Scene encapsulates the values of selected channels from one or more fixture
 * instances. When a scene is started, the duration it takes for its channels
 * to reach their target values depends on the function's speed setting. Bus
 * number 1 (Default Fade) is assigned to all newly created scenes by default.
 * If the bus' value is 0 seconds, scene values are set immediately and no
 * fading occurs. Otherwise values are always faded from what they currently
 * are, to the target values defined in the scene (with SceneValue instances).
 * Channels that are not enabled in the scene will not be touched at all.
 */
class Scene : public Function
{
	Q_OBJECT
	Q_DISABLE_COPY(Scene)

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/**
	 * Construct a new scene function, with given parent object. If the
	 * parent is not a Doc* object, the debug build asserts.
	 *
	 * @param parent The parent object who owns the scene
	 */
	Scene(QObject* parent);

	/**
	 * Destroy the scene
	 */
	~Scene();

	/*********************************************************************
	 * Function type
	 *********************************************************************/
public:
	/** @reimpl */
	Function::Type type() const;

	/*********************************************************************
	 * Copying
	 *********************************************************************/
public:
	/** @reimpl */
	Function* createCopy(Doc* doc);

	/** @reimpl */
	bool copyFrom(const Function* function);

	/*********************************************************************
	 * Values
	 *********************************************************************/
public:
	/**
	 * Set the value of one fixture channel, using a predefined SceneValue
	 */
	void setValue(SceneValue scv);

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
	void writeValues(QByteArray* universes, t_fixture_id fxi_id = KNoID);

	/**
	 * Write zeros for all involved channels to OutputMap. If fxi_id is
	 * given, writes zeros only for the specified fixture. 
	 */
	void writeZeros(QByteArray* universes, t_fixture_id fxi_id = KNoID);

	/**
	 * Get a list of values in this scene
	 */
	QList <SceneValue> *values() { return &m_values; }

protected:
	QList <SceneValue> m_values;

	/*********************************************************************
	 * Fixtures
	 *********************************************************************/
public slots:
	void slotFixtureRemoved(t_fixture_id fxi_id);

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	/** @reimpl */
	bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

	/** @reimpl */
	bool loadXML(const QDomElement* root);

	/*********************************************************************
	 * Flash
	 *********************************************************************/
public:
	/** @reimpl */
	void flash(QByteArray* universes);

	/** @reimpl */
	void unFlash(QByteArray* universes);

	/*********************************************************************
	 * Running
	 *********************************************************************/
public:
	/** @reimpl */
	void arm();

	/** @reimpl */
	void disarm();

	/** @reimpl */
	bool write(QByteArray* universes);

protected:
	t_value nextValue(SceneChannel* sch);

protected:
	QList <SceneChannel> m_channels;
};

#endif
