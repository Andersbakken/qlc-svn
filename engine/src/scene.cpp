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

#include <QtDebug>
#include <QList>
#include <QFile>
#include <QtXml>

#include "qlcfixturedef.h"
#include "qlcfile.h"

#include "mastertimer.h"
#include "scene.h"
#include "doc.h"
#include "bus.h"

/****************************************************************************
 * SceneChannel
 ****************************************************************************/

SceneChannel::SceneChannel()
{
	address = 0;
	start = 0;
	current = 0;
	target = 0;
}

SceneChannel::SceneChannel(const SceneChannel& sch)
{
	address = sch.address;
	start = sch.start;
	current = sch.current;
	target = sch.target;
}

SceneChannel::~SceneChannel()
{
}

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

SceneValue::~SceneValue()
{
}

bool SceneValue::isValid()
{
	if (fxi == Fixture::invalidId())
		return false;
	else
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

bool SceneValue::loadXML(const QDomElement* tag)
{
	Q_ASSERT(tag != NULL);

	if (tag->tagName() != KXMLQLCSceneValue)
	{
		qWarning() << "Node is not a scene value tag:"
			   << tag->tagName();
		return false;
	}

	fxi = t_fixture_id(tag->attribute(KXMLQLCSceneValueFixture).toInt());
	if (fxi < 0 || fxi >= KFixtureArraySize)
		return false;

	channel = t_channel(tag->attribute(KXMLQLCSceneValueChannel).toInt());
	if (channel >= KChannelMax)
		return false;

	value = t_value(tag->text().toUInt());

	return isValid();
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

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Scene::Scene(Doc* doc) : Function(doc)
{
	setName(tr("New Scene"));
	setBus(Bus::defaultFade());
}

Scene::~Scene()
{
}

/*****************************************************************************
 * Function type
 *****************************************************************************/

Function::Type Scene::type() const
{
	return Function::Scene;
}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* Scene::createCopy(Doc* doc)
{
	Q_ASSERT(doc != NULL);

	Function* copy = new Scene(doc);
	Q_ASSERT(copy != NULL);

	if (copy->copyFrom(this) == false)
	{
		delete copy;
		copy = NULL;
	}
	else if (doc->addFunction(copy) == false)
	{
		delete copy;
		copy = NULL;
	}
	else
	{
		copy->setName(tr("Copy of %1").arg(name()));
	}

	return copy;
}

bool Scene::copyFrom(const Function* function)
{
	const Scene* scene = qobject_cast<const Scene*> (function);
	if (scene == NULL)
		return false;

	m_values.clear();
	m_values = scene->m_values;

	bool result = Function::copyFrom(function);

	emit changed(m_id);

	return result;
}

/*****************************************************************************
 * Values
 *****************************************************************************/

void Scene::setValue(const SceneValue& scv)
{
	int index = m_values.indexOf(scv);
	if (index == -1)
		m_values.append(scv);
	else
		m_values.replace(index, scv);

	emit changed(m_id);
}

void Scene::setValue(t_fixture_id fxi, t_channel ch, t_value value)
{
	setValue(SceneValue(fxi, ch, value));
}

void Scene::unsetValue(t_fixture_id fxi, t_channel ch)
{
	m_values.removeAll(SceneValue(fxi, ch, 0));
	emit changed(m_id);
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

	emit changed(m_id);
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
	root.setAttribute(KXMLQLCFunctionType, Function::typeToString(type()));
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

bool Scene::loadXML(const QDomElement* root)
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

	if (root->attribute(KXMLQLCFunctionType) !=
	    typeToString(Function::Scene))
	{
		qWarning("Function is not a scene!");
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
			setBus(tag.text().toUInt());
		}
		else if (tag.tagName() == KXMLQLCFunctionValue)
		{
			/* Channel value */
			SceneValue scv;
			if (scv.loadXML(&tag) == true)
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

/****************************************************************************
 * Flashing
 ****************************************************************************/

void Scene::flash(MasterTimer* timer)
{
	if (flashing() == true)
		return;

	Q_ASSERT(timer != NULL);
	Function::flash(timer);
	timer->registerDMXSource(this);
}

void Scene::unFlash(MasterTimer* timer)
{
	if (flashing() == false)
		return;

	Q_ASSERT(timer != NULL);
	Function::unFlash(timer);
}

void Scene::writeDMX(MasterTimer* timer, QByteArray* universes)
{
	Q_ASSERT(timer != NULL);
	Q_ASSERT(universes != NULL);

	if (flashing() == true)
	{
		writeValues(universes);
	}
	else
	{
		writeZeros(universes);
		timer->unregisterDMXSource(this);
	}
}

/****************************************************************************
 * Running
 ****************************************************************************/

void Scene::arm()
{
	m_armedChannels.clear();

	/* Scenes cannot run unless they are children of Doc */
	Doc* doc = qobject_cast <Doc*> (parent());
	Q_ASSERT(doc != NULL);

	/* Get exact address numbers from fixtures and fixate them to this
	   scene for running. */
	QMutableListIterator <SceneValue> it(m_values);
	while (it.hasNext() == true)
	{
		SceneValue scv(it.next());

		Fixture* fxi = doc->fixture(scv.fxi);
		if (fxi == NULL)
		{
			qWarning() << "Channel" << scv.channel << "from an"
				   << "unavailable fixture ID" << scv.fxi
				   << "taking part in scene" << name()
				   << ". Removing the channel.";
			it.remove();
			continue;
		}

		if (scv.channel < fxi->channels())
		{
			SceneChannel channel;
			channel.address = fxi->universeAddress() + scv.channel;
			channel.target = scv.value;
			m_armedChannels.append(channel);
		}
		else
		{
			qWarning() << "Scene " << name() << "channel"
				   << scv.channel
				   << "is out of its fixture" << fxi->name()
				   << "channel count ( <" << fxi->channels()
				   << ") bounds. Removing the channel.";
			it.remove();
		}
	}

	resetElapsed();
}

void Scene::disarm()
{
	m_armedChannels.clear();
}

void Scene::write(MasterTimer* timer, QByteArray* universes)
{
	Q_UNUSED(timer);
	Q_ASSERT(universes != NULL);

	/* Count ready channels so that the scene can be stopped */
	t_channel ready = m_armedChannels.count();

	/* Iterator for all scene channels */
	QMutableListIterator <SceneChannel> it(m_armedChannels);

	/* Get starting values for each channel on the first pass */
	if (elapsed() == 0)
	{
		while (it.hasNext() == true)
		{
			SceneChannel sch = it.next();

			/* Get the starting value from universes. Important
			   to cast to t_value, since QByteArray handles signed
			   char, whereas t_value is unsigned. Without cast,
			   this will result in negative values when x > 127 */
			sch.start = t_value(universes->data()[sch.address]);
			sch.current = sch.start;

			/* Set the changed object back to the list */
			it.setValue(sch);
		}

		/* Reel back to start of list */
		it.toFront();
	}

	while (it.hasNext() == true)
	{
		SceneChannel sch = it.next();

		if (sch.current == sch.target)
		{
			ready--;
			continue;
		}
		if (elapsed() >= Bus::instance()->value(m_busID))
		{
			/* When this scene's time is up, write the absolute
			   target values to get rid of rounding errors that
			   may happen in nextValue(). */
			sch.current = sch.target;
			ready--;

			/* Write the target value to the universe */
			universes->data()[sch.address] = sch.target;
		}
		else
		{
			/* Write the next value to the universe buffer */
			universes->data()[sch.address] = nextValue(&sch);
		}

		/* Set the changed object back to the list */
		it.setValue(sch);
	}

	/* Next time unit */
	incrementElapsed();

	/* When all channels are ready, this function can be stopped. */
	if (ready == 0)
		stop();
}

void Scene::writeValues(QByteArray* universes, t_fixture_id fxi_id)
{
	Q_ASSERT(universes != NULL);

	for (int i = 0; i < m_values.count(); i++)
	{
		if (fxi_id == Fixture::invalidId() || m_values[i].fxi == fxi_id)
		{
			t_channel addr = m_armedChannels[i].address;
			universes->data()[addr] = m_values[i].value;
		}
	}
}

void Scene::writeZeros(QByteArray* universes, t_fixture_id fxi_id)
{
	Q_ASSERT(universes != NULL);

	for (int i = 0; i < m_values.count(); i++)
	{
		if (fxi_id == Fixture::invalidId() || m_values[i].fxi == fxi_id)
		{
			t_channel addr = m_armedChannels[i].address;
			universes->data()[addr] = 0;
		}
	}
}

t_value Scene::nextValue(SceneChannel* sch)
{
	Q_ASSERT(sch != NULL);

	/* Ensure that bus value is never zero */
	qreal busValue = qreal(Bus::instance()->value(m_busID)) + 1.0;

	/* Time scale is basically a percentage (0.0 - 1.0) of remaining time */
	qreal timeScale = qreal(elapsed() + 1) / busValue;

	/*
	 * Calculate the current value based on what it should be after
	 * Function::elapsed() cycles, so that it will be ready when
	 * Function::elapsed() == Bus::instance()->value()
	 */
	sch->current = sch->target - sch->start;
	sch->current = int(qreal(sch->current) * timeScale);
	sch->current += sch->start;

	return t_value(sch->current);
}
