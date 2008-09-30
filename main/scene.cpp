/*
  Q Light Controller
  scene.cpp

  Copyright (c) Heikki Junnila

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

#include <QApplication>
#include <iostream>
#include <QtDebug>
#include <QList>
#include <QFile>
#include <QtXml>

#include <time.h>

#include "common/qlcfixturedef.h"
#include "common/qlcfile.h"

#include "functionconsumer.h"
#include "eventbuffer.h"
#include "outputmap.h"
#include "scene.h"
#include "app.h"
#include "doc.h"
#include "bus.h"

using namespace std;

extern App* _app;

/*****************************************************************************
 * SceneValue
 *****************************************************************************/

SceneValue::SceneValue(t_fixture_id id, t_channel ch, t_value val) :
	fxi     ( id ),
	channel ( ch ),
	value   ( val )
{
}

SceneValue::SceneValue(const SceneValue& scv)
{
	fxi = scv.fxi;
	channel = scv.channel;
	value = scv.value;
}

SceneValue::SceneValue(QDomElement* tag)
{
	Q_ASSERT(tag != NULL);

	if (tag->tagName() != KXMLQLCSceneValue)
	{
		fxi = KNoID;
		channel = 0;
		value = 0;

		qWarning() << "Node is not a scene value tag:"
			   << tag->tagName();
	}
	else
	{
		fxi = tag->attribute(KXMLQLCSceneValueFixture).toInt();
		channel = tag->attribute(KXMLQLCSceneValueChannel).toInt();
		value = tag->text().toInt();
	}
}

SceneValue::~SceneValue()
{
}

bool SceneValue::isValid()
{
	if (fxi == KNoID)
		return false;
	else
		return true;
}

bool SceneValue::saveXML(QDomDocument* doc, QDomElement* scene_root) const
{
	QDomElement tag;
	QDomText text;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(scene_root != NULL);

	/* Value tag and its attributes */
	tag = doc->createElement(KXMLQLCSceneValue);
	tag.setAttribute(KXMLQLCSceneValueFixture, fxi);
	tag.setAttribute(KXMLQLCSceneValueChannel, channel);
	scene_root->appendChild(tag);

	/* The actual value as node text */
	text = doc->createTextNode(QString("%1").arg(value));
	tag.appendChild(text);
	
	return true;
}

bool SceneValue::operator< (const SceneValue& scv) const
{
	if (fxi < scv.fxi)
	{
		return true;
	}
	else if (fxi == scv.fxi)
	{
		if (channel < scv.channel)
			return true;
		else
			return false;
	}
	else
	{
		return false;
	}
}

bool SceneValue::operator== (const SceneValue& scv) const
{
	if (fxi == scv.fxi && channel == scv.channel)
		return true;
	else
		return false;
}

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Scene::Scene(QObject* parent) : Function(parent, Function::Scene)
{
	m_timeSpan = 255;
	m_elapsedTime = 0;
	m_channels = NULL;
	m_channelData = NULL;
	
	setBus(KBusIDDefaultFade);
}

Scene::~Scene()
{
}

void Scene::copyFrom(Scene* scene)
{
	Q_ASSERT(scene != NULL);

	setName(scene->name());
	setBus(scene->busID());

	m_values.clear();
	m_values = scene->m_values;
}

/*****************************************************************************
 * Values
 *****************************************************************************/

void Scene::setValue(SceneValue scv)
{
	int index = m_values.indexOf(scv);
	if (index == -1)
	{
		m_values.append(scv);
		qSort(m_values.begin(), m_values.end());
	}
	else
	{
		m_values.replace(index, scv);
	}

	_app->doc()->setModified();
	_app->doc()->emitFunctionChanged(m_id);
}

void Scene::setValue(t_fixture_id fxi, t_channel ch, t_value value)
{
	setValue(SceneValue(fxi, ch, value));
}

void Scene::unsetValue(t_fixture_id fxi, t_channel ch)
{
	m_values.removeAll(SceneValue(fxi, ch, 0));
}

t_value Scene::value(t_fixture_id fxi, t_channel ch)
{
	SceneValue scv(fxi, ch, 0);
	int index = m_values.indexOf(scv);
	if (index == -1)
		return 0;
	else
		return m_values.at(index).value;
}

void Scene::writeValues(t_fixture_id fxi_id)
{
	for (int i = 0; i < m_values.count(); i++)
	{
		if (fxi_id == KNoID || m_values[i].fxi == fxi_id)
		{
			_app->outputMap()->setValue(m_channels[i].address,
						    m_values[i].value);
		}
	}
}

/*****************************************************************************
 * Fixtures
 *****************************************************************************/

void Scene::slotFixtureRemoved(t_fixture_id fxi_id)
{
	QMutableListIterator <SceneValue> it(m_values);
	while (it.hasNext() == true)
	{
		SceneValue scv = it.next();
		if (scv.fxi == fxi_id)
			it.remove();
	}
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool Scene::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(wksp_root != NULL);

	/* Function tag */
	root = doc->createElement(KXMLQLCFunction);
	wksp_root->appendChild(root);

	root.setAttribute(KXMLQLCFunctionID, id());
	root.setAttribute(KXMLQLCFunctionType, Function::typeToString(m_type));
	root.setAttribute(KXMLQLCFunctionName, name());

	/* Speed bus */
	tag = doc->createElement(KXMLQLCBus);
	root.appendChild(tag);
	tag.setAttribute(KXMLQLCBusRole, KXMLQLCBusFade);
	str.setNum(busID());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Scene contents */
	QListIterator <SceneValue> it(m_values);
	while (it.hasNext() == true)
		it.next().saveXML(doc, &root);
		
	return true;
}

bool Scene::loadXML(QDomDocument*, QDomElement* root)
{
	QString str;
	
	QDomNode node;
	QDomElement tag;
	
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCFunction)
	{
		qWarning("Function node not found!");
		return false;
	}

	/* Load scene contents */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		
		if (tag.tagName() == KXMLQLCBus)
		{
			/* Bus */
			str = tag.attribute(KXMLQLCBusRole);
			Q_ASSERT(str == KXMLQLCBusFade);

			setBus(tag.text().toInt());
		}
		else if (tag.tagName() == KXMLQLCFunctionValue)
		{
			/* Channel value */
			SceneValue scv(&tag);
			if (scv.isValid() == true)
				setValue(scv);
		}
		else
		{
			qWarning() << "Unknown scene tag:" << tag.tagName();
		}
		
		node = node.nextSibling();
	}

	return true;
}

/*****************************************************************************
 * Bus
 *****************************************************************************/

void Scene::slotBusValueChanged(t_bus_id id, t_bus_value value)
{
	if (id != m_busID)
		return;

	if (isRunning() == true)
		speedChange(value);
	else
		m_timeSpan = value;
}

void Scene::speedChange(t_bus_value newTimeSpan)
{
	m_timeSpan = newTimeSpan;
	if (m_timeSpan == 0)
	{
		m_timeSpan = static_cast<t_bus_value>
			(1.0 / static_cast<float> (KFrequency));
	}
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void Scene::arm()
{
	int i = 0;

	if (m_channels == NULL)
		m_channels = new SceneChannel[m_values.count()];

	if (m_channelData == NULL)
		m_channelData = new t_buffer_data[m_values.count()];
	
	/* TODO: Shifting right (=division by two) causes rounding errors!! */
	if (m_eventBuffer == NULL)
		m_eventBuffer = new EventBuffer(m_values.count(),
						KFrequency >> 1);

	/* Fill in the actual channel numbers because they can't change
	   after functions have been armed for operate mode. */
	QListIterator <SceneValue> it(m_values);
	while (it.hasNext() == true)
	{
		SceneValue scv = it.next();
		Fixture* fxi = _app->doc()->fixture(scv.fxi);
		Q_ASSERT(fxi != NULL);

		m_channels[i].address = fxi->universeAddress() + scv.channel;
		m_channelData[i] = (fxi->universeAddress() + scv.channel) << 8;
		i++;
	}
}

void Scene::disarm()
{
	if (m_channels != NULL)
		delete [] m_channels;
	m_channels = NULL;
	
	if (m_channelData != NULL)
		delete [] m_channelData;
	m_channelData = NULL;
	
	if (m_eventBuffer != NULL)
		delete m_eventBuffer;
	m_eventBuffer = NULL;
}

void Scene::stop()
{
	m_stopped = true;
}

void Scene::run()
{
	t_channel channels = m_values.count();
	t_channel ready = channels;
	t_channel i;

	emit running(m_id);

	// Initialize this scene for running
	m_stopped = false;
	
	// No time has yet passed for this scene.
	m_elapsedTime = 0;
	
	// Get speed
	m_timeSpan = Bus::value(m_busID);

	// Set initial speed
	speedChange(m_timeSpan);

	// Append this function to running functions' list
	_app->functionConsumer()->cue(this);

	/* Fill run-time buffers with the data for the first step */
	for (i = 0; i < channels; i++)
	{
		m_channels[i].current = m_channels[i].start =
			static_cast<float> 
			(_app->outputMap()->getValue(m_channels[i].address));

		m_channels[i].target = 
			static_cast<float> (m_values.at(i).value);

		/* Check, whether this scene needs to play at all */
		if (m_channels[i].current == m_values.at(i).value)
		{
			m_channels[i].ready = true;
			ready--;
		}
		else
		{
			m_channels[i].ready = false;
		}
	}

	if (ready == 0)
	{
		/* This scene does not need to be played because all target
		   values are already where they are supposed to be. */
		m_stopped = true;
	}

	for (m_elapsedTime = 0;
	     m_elapsedTime < m_timeSpan && m_stopped == false;
	     m_elapsedTime++)
	{
		for (i = 0; i < channels; i++)
		{
			/* Calculate the current value based on what
			   it should be after m_elapsedTime, so that it
			   will be ready when elapsedTime == timeSpan */
			m_channels[i].current = m_channels[i].start
				+ (m_channels[i].target - m_channels[i].start)
				* (float(m_elapsedTime) / m_timeSpan);
			
			/* The address is in the first 8 bits, so
			   preserve that part with AND. Then add the
			   value to the lowest 8 bits with OR. */
			m_channelData[i] = (m_channelData[i] & 0xff00)
				| static_cast<t_buffer_data>
				(m_channels[i].current);
		}
		
		m_eventBuffer->put(m_channelData);
	}
	
	/* Write the last step exactly to target because timespan might have
	   been set to a smaller amount than what has elapsed. Also, because
	   floats are NEVER exact numbers, it might be that we never quite
	   reach the target within the given timespan (in case the values don't
	   add up because of rounding errors and inaccuracy of floats). */
	for (i = 0; i < channels && m_stopped == false; i++)
	{
		/* Just set the target value */
		m_channelData[i] = (m_channelData[i] & 0xff00) |
			static_cast<t_buffer_data> (m_channels[i].target);
	}
	
	if (m_stopped == false)
	{
		/* Put the last values to the buffer */
		m_eventBuffer->put(m_channelData);
	}
	else
	{
		/* This scene was stopped. Clear buffer so that this function
		   can finish as quickly as possible */
		m_eventBuffer->purge();
	}
}
