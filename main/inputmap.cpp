/*
  Q Light Controller
  inputmap.cpp
  
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

#include <QPluginLoader>
#include <QMessageBox>
#include <QStringList>
#include <QSettings>
#include <QDebug>
#include <QList>
#include <QtXml>
#include <QDir>

#include "common/qlcinplugin.h"
#include "common/qlctypes.h"

#include "inputmap.h"
#include "app.h"

using namespace std;

extern App* _app;

/*****************************************************************************
 * InputPatch
 *****************************************************************************/

bool InputPatch::saveXML(QDomDocument* doc, QDomElement* map_root, int universe)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(this->plugin != NULL);

	/* Patch entry */
	root = doc->createElement(KXMLQLCInputPatch);
	map_root->appendChild(root);

	/* Universe */
	str.setNum(universe);
	root.setAttribute(KXMLQLCInputPatchUniverse, str);

	/* Plugin */
	tag = doc->createElement(KXMLQLCInputPatchPlugin);
	root.appendChild(tag);
	text = doc->createTextNode(this->plugin->name());
	tag.appendChild(text);

	/* Input */
	tag = doc->createElement(KXMLQLCInputPatchInput);
	root.appendChild(tag);
	str.setNum(this->input);
	text = doc->createTextNode(str);
	tag.appendChild(text);

	return true;
}

bool InputPatch::loader(QDomDocument*, QDomElement* root, InputMap* inputMap)
{
	QDomNode node;
	QDomElement tag;
	QString str;
	QString pluginName;
	int input = 0;
	int universe = 0;
	
	Q_ASSERT(root != NULL);
	Q_ASSERT(dmxMap != NULL);

	if (root->tagName() != KXMLQLCInputPatch)
	{
		qWarning() << "Patch node not found!";
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
		else
		{
			qWarning() << "Unknown InputPatch tag: " << tag.tagName();
		}
		
		node = node.nextSibling();
	}

	return inputMap->setPatch(universe, pluginName, input);
}

/*****************************************************************************
 * Initialization
 *****************************************************************************/

InputMap::InputMap(int universes)
{
	m_universes = universes;

	initPatch();
	load();
}

InputMap::~InputMap()
{
	while (m_plugins.isEmpty() == false)
	{
		QLCInPlugin* plugin = m_plugins.takeFirst();
		delete plugin;
	}
}

void InputMap::load()
{
	QSettings s;
	
	QString pluginPath = s.value("directories/plugins").toString();
	if (pluginPath.isEmpty() == true)
	{
#ifdef WIN32
		pluginPath = "%%SystemRoot%%\\QLC\\Plugins";
#else
		pluginPath = "/usr/lib/qlc";
#endif
		s.setValue("directories/plugins", pluginPath);
	}

#ifdef WIN32
	QDir dir(pluginPath + "\\Input", "*.dll", QDir::Name, QDir::Files);
#else
	QDir dir(pluginPath + "/input", "*.so", QDir::Name, QDir::Files);
#endif

	/* Check that we can access the directory */
	if (dir.exists() == false)
	{
		qWarning() << "Input plugin path" << dir.absolutePath()
			   << "doesn't exist.";
		return;
	}
	else if (dir.isReadable() == false)
	{
		qWarning() << "Input plugin path" << dir.absolutePath()
			   << "is not accessible";
		return;
	}

	/* Loop thru all files in the directory */
	QStringList dirlist(dir.entryList());
	QStringList::Iterator it;
	for (it = dirlist.begin(); it != dirlist.end(); ++it)
	{
		QPluginLoader loader(dir.absoluteFilePath(*it), this);
		QLCInPlugin* plugin;

		plugin = qobject_cast<QLCInPlugin*> (loader.instance());
		if (plugin == NULL)
		{
			qWarning() << "Unable to find an input plugin in"
				   << dir.absoluteFilePath(*it);
		}
		else
		{
			plugin->init();
			appendPlugin(plugin);
			plugin->connectInputData(this);
		}
	}
}

/*****************************************************************************
 * Patch
 *****************************************************************************/

void InputMap::initPatch()
{
	InputPatch* inputPatch = NULL;
	int i = 0;

	for (i = 0; i < m_universes; i++)
	{
		inputPatch = new InputPatch(NULL, i);
		m_patch.insert(i, inputPatch);
	}
}

bool InputMap::setPatch(unsigned int universe, const QString& pluginName,
			unsigned int input)
{
	if (int(universe) >= m_patch.size())
	{
		qWarning() << "Universe number out of bounds:" << universe
			   << "Unable to set patch.";
		return false;
	}

	QLCInPlugin* inputPlugin = plugin(pluginName);
	if (inputPlugin == NULL)
	{
		qWarning() << "Plugin" << pluginName << "for universe number"
			   << universe << "not found.";
		return false;
	}
	else
	{
		//m_patch[universe]->plugin->close();
		m_patch[universe]->plugin = inputPlugin;
		m_patch[universe]->input = input;
		//m_patch[universe]->plugin->open();

		return true;
	}
}

InputPatch* InputMap::patch(int universe)
{
	Q_ASSERT(universe >= 0 && universe < KInputUniverseCount);
	return m_patch[universe];
}

/*****************************************************************************
 * Plugins
 *****************************************************************************/

QStringList InputMap::pluginNames()
{
	QListIterator <QLCInPlugin*> it(m_plugins);
	QStringList list;

	while (it.hasNext() == true)
		list.append(it.next()->name());
	
	return list;
}

int InputMap::pluginInputs(const QString&)
{
	/* TODO */
	return 0;
}

void InputMap::configurePlugin(const QString& pluginName)
{
	QLCInPlugin* inputPlugin = plugin(pluginName);
	if (inputPlugin == NULL)
		QMessageBox::warning(_app,
				     "Unable to configure plugin",
				     pluginName + QString(" not found!"));
	else
		inputPlugin->configure();
}

QString InputMap::pluginStatus(const QString& pluginName)
{
	QLCInPlugin* inputPlugin;
	QString info;

	if (pluginName != QString::null)
		inputPlugin = plugin(pluginName);
	else
		inputPlugin = NULL;
	
	if (inputPlugin == NULL)
	{
		/* Overall plugin info */

		// HTML header
		info += QString("<HTML>");
		info += QString("<HEAD>");
		info += QString("<TITLE>Input mapping status</TITLE>");
		info += QString("</HEAD>");
		info += QString("<BODY>");

		// Mapping status title
		info += QString("<TABLE COLS=\"1\" WIDTH=\"100%\">");
		info += QString("<TR>");
		info += QString("<TD BGCOLOR=\"");
		//info += _app->colorGroup().highlight().name();
		info += QString("\">");
		info += QString("<FONT COLOR=\"");
		//info += _app->colorGroup().highlightedText().name();
		info += QString("\" SIZE=\"5\">");
		info += QString("Input mapping status");
		info += QString("</FONT>");
		info += QString("</TD>");
		info += QString("</TR>");
		info += QString("</TABLE>");

		info += QString("TODO...");
	}
	else
	{
		/* Plugin-specific info */
		info = inputPlugin->infoText();
	}

	return info;
}

bool InputMap::appendPlugin(QLCInPlugin* inputPlugin)
{
	Q_ASSERT(inputPlugin != NULL);

	if (plugin(inputPlugin->name()) == NULL)
	{
		qDebug() << "Found an input plugin:" << inputPlugin->name();
		m_plugins.append(inputPlugin);
		return true;
	}
	else
	{
		qWarning() << "Input plugin" << inputPlugin->name()
			   << "is already loaded. Skipping.";
		return false;
	}
}

QLCInPlugin* InputMap::plugin(const QString& name)
{
	QListIterator <QLCInPlugin*> it(m_plugins);

	while (it.hasNext() == true)
	{
		QLCInPlugin* plugin = it.next();
		if (plugin->name() == name)
			return plugin;
	}
	
	return NULL;
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void InputMap::slotValueChanged(QLCInPlugin* plugin, t_input input,
				t_input_channel channel, t_input_value value)
{
	qDebug() << plugin->name() << input << channel << value;
}

/*****************************************************************************
 * Defaults
 *****************************************************************************/

void InputMap::loadDefaults()
{
}

void InputMap::saveDefaults()
{
}
