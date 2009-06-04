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
#include <QtDebug>
#include <QList>
#include <QFile>
#include <QtXml>

#include <time.h>

#include "common/qlcfixturedef.h"
#include "common/qlcfile.h"

#include "sceneeditor.h"
#include "outputmap.h"
#include "scene.h"
#include "app.h"
#include "doc.h"
#include "bus.h"

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

Scene::Scene(QObject* parent) : Function(parent)
{
	setName(tr("New Scene"));
	setBus(KBusIDDefaultFade);
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

Function* Scene::createCopy()
{
	Function* copy = NULL;

	copy = new Scene(_app->doc());
	Q_ASSERT(copy != NULL);

	if (copy->copyFrom(this) == false)
	{
		delete copy;
		copy = NULL;
	}
	else if (_app->doc()->addFunction(copy) == false)
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

	return Function::copyFrom(function);
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

void Scene::writeZeros(t_fixture_id fxi_id)
{
	for (int i = 0; i < m_values.count(); i++)
	{
		if (fxi_id == KNoID || m_values[i].fxi == fxi_id)
		{
			_app->outputMap()->setValue(m_channels[i].address, 0);
		}
	}
}

/*****************************************************************************
 * Edit
 *****************************************************************************/

int Scene::edit()
{
	SceneEditor editor(_app, this);
	int result = editor.exec();
	if (result == QDialog::Accepted)
		emit changed(m_id);
	return result;
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

/****************************************************************************
 * SceneChannel implementation
 ****************************************************************************/

SceneChannel::SceneChannel()
{
	address = 0;
	start = 0;
	current = 0;
	target = 0;
	ready = true;
}

SceneChannel::SceneChannel(const SceneChannel& sch)
{
	*this = sch;
}

SceneChannel::~SceneChannel()
{
}

SceneChannel& SceneChannel::operator=(const SceneChannel& sch)
{
	if (this != &sch)
	{
		address = sch.address;
		start = sch.start;
		current = sch.current;
		target = sch.target;
		ready = sch.ready;
	}

	return *this;
}

/****************************************************************************
 * Flashing
 ****************************************************************************/

void Scene::flash()
{
	if (isFlashing() == false)
	{
		Function::flash();
		writeValues();
	}
}

void Scene::unFlash()
{
	if (isFlashing() == true)
	{
		Function::unFlash();
		writeZeros();
	}
}

/****************************************************************************
 * Running
 ****************************************************************************/

void Scene::arm()
{
	m_channels.clear();

	QListIterator <SceneValue> it(m_values);
	while (it.hasNext() == true)
	{
		SceneChannel channel;
		SceneValue scv(it.next());
		Fixture* fxi = _app->doc()->fixture(scv.fxi);
		Q_ASSERT(fxi != NULL);

		channel.address = fxi->universeAddress() + scv.channel;
		channel.target = scv.value;

		m_channels.append(channel);
	}
}

void Scene::disarm()
{
	m_channels.clear();
}

bool Scene::write(QByteArray* universes)
{
	/* Count ready channels so that the scene can be stopped */
	t_channel ready = m_channels.count();

	/* Iterator for all scene channels */
	QMutableListIterator <SceneChannel> it(m_channels);

	Q_ASSERT(universes != NULL);

	/* Get starting values for each channel on the first pass */
	if (m_elapsed == 0)
	{
		while (it.hasNext() == true)
		{
			SceneChannel sch = it.next();

			sch.start = sch.current =
				_app->outputMap()->value(sch.address);

			/* Mark channels ready if they are already what they
			   are supposed to be. */
			if (sch.current == sch.target)
				sch.ready = true;
			else
				sch.ready = false;

			/* Set the changed object back to the list */
			it.setValue(sch);
		}

		/* Reel back to start of list */
		it.toFront();
	}

	/* Next time unit */
	m_elapsed++;

	while (it.hasNext() == true)
	{
		SceneChannel sch = it.next();

		if (sch.ready == true)
		{
			ready--;
			continue;
		}
		if (m_elapsed >= Bus::value(m_busID))
		{
			/* When this scene's time is up, write the absolute
			   target values to get rid of rounding errors that
			   may happen in nextValue(). */
			sch.current = sch.target;
			sch.ready = true;
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

	/* When all channels are ready, this function can be stopped. */
	if (ready == 0)
		return false;
	else
		return true;
}

t_value Scene::nextValue(SceneChannel* sch)
{
	double timeScale;

	Q_ASSERT(sch != NULL);

	/*
	 *Calculate the current value based on what it should be after
	 * m_elapsed cycles, so that it will be ready when
	 * m_elapsed == Bus::value()
	 */
	timeScale = double(m_elapsed) / double(Bus::value(m_busID));
	sch->current = sch->target - sch->start;
	sch->current = int(double(sch->current) * timeScale);
	sch->current += sch->start;

	return t_value(sch->current);
}
