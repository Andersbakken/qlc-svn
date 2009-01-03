/*
  Q Light Controller
  inputpatch.cpp

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

#include <QObject>
#include <QDebug>
#include <QtXml>

#include "common/qlcinplugin.h"
#include "common/qlctypes.h"

#include "inputpatch.h"
#include "inputmap.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

InputPatch::InputPatch(QObject* parent) : QObject(parent)
{
	Q_ASSERT(parent != NULL);

	m_plugin = NULL;
	m_input = KInputInvalid;
	m_device = NULL;
}

InputPatch::~InputPatch()
{
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void InputPatch::set(QLCInPlugin* plugin, t_input input, QLCInputDevice* device)
{
	/* TODO: This closes the plugin line always, regardless of whether
	   the line has been assigned to more than one input universe */
	if (m_plugin != NULL && m_input != KInputInvalid)
		m_plugin->close(m_input);

	m_plugin = plugin;
	m_input = input;
	m_device = device;

	/* Open the assigned plugin input */
	if (m_plugin != NULL && input != KInputInvalid)
		m_plugin->open(m_input);
}

QString InputPatch::pluginName() const
{
	if (m_plugin != NULL)
		return m_plugin->name();
	else
		return KInputNone;
}

t_input InputPatch::input() const
{
	if (m_plugin != NULL && m_input < m_plugin->inputs().count())
		return m_input;
	else
		return KInputInvalid;
}

QString InputPatch::inputName() const
{
	if (m_plugin != NULL && m_input != KInputInvalid &&
	    m_input < m_plugin->inputs().count())
		return m_plugin->inputs()[m_input];
	else
		return KInputNone;
}

QString InputPatch::deviceName() const
{
	if (m_device != NULL)
		return m_device->name();
	else
		return KInputNone;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool InputPatch::saveXML(QDomDocument* doc, QDomElement* map_root,
			 t_input_universe universe)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);

	/* Patch entry */
	root = doc->createElement(KXMLQLCInputPatch);
	map_root->appendChild(root);

	/* Universe */
	str.setNum(universe);
	root.setAttribute(KXMLQLCInputPatchUniverse, str);

	/* Plugin */
	tag = doc->createElement(KXMLQLCInputPatchPlugin);
	root.appendChild(tag);
	text = doc->createTextNode(pluginName());
	tag.appendChild(text);

	/* Input */
	tag = doc->createElement(KXMLQLCInputPatchInput);
	root.appendChild(tag);
	str.setNum(m_input);
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Device */
	tag = doc->createElement(KXMLQLCInputPatchDevice);
	root.appendChild(tag);
	text = doc->createTextNode(deviceName());
	tag.appendChild(text);

	return true;
}

bool InputPatch::loader(QDomDocument*, QDomElement* root, InputMap* inputMap)
{
	t_input_universe universe = 0;
	QString deviceName;
	QString pluginName;
	QDomElement tag;
	QDomNode node;
	t_input input = 0;
	QString str;

	Q_ASSERT(root != NULL);
	Q_ASSERT(inputMap != NULL);

	if (root->tagName() != KXMLQLCInputPatch)
	{
		qWarning() << "Input patch node not found!";
		return false;
	}

	/* QLC input universe that this patch has been made for */
	universe = root->attribute(KXMLQLCInputPatchUniverse).toInt();

	/* Load patch contents */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();

		if (tag.tagName() == KXMLQLCInputPatchPlugin)
		{
			/* Plugin name */
			pluginName = tag.text();
		}
		else if (tag.tagName() == KXMLQLCInputPatchInput)
		{
			/* Plugin input */
			input = tag.text().toInt();
		}
		else if (tag.tagName() == KXMLQLCInputPatchDevice)
		{
			/* Device */
			deviceName = tag.text();
		}
		else
		{
			qWarning() << "Unknown Input patch tag: "
				   << tag.tagName();
		}

		node = node.nextSibling();
	}

	return inputMap->setPatch(universe, pluginName, input, deviceName);
}
