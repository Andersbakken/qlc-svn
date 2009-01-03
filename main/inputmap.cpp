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
#include <QApplication>
#include <QMessageBox>
#include <QStringList>
#include <QSettings>
#include <QDebug>
#include <QList>
#include <QtXml>
#include <QDir>

#include <common/qlcinplugin.h>
#include <common/qlctypes.h>
#include <common/qlcfile.h>

#include "inputpatch.h"
#include "inputmap.h"
#include "app.h"

extern App* _app;

#ifdef WIN32
	#define PLUGINEXT ".dll"
#elif __APPLE__
	#define PLUGINEXT ".dylib"
#else
	#define PLUGINEXT ".so"
#endif

/*****************************************************************************
 * Initialization
 *****************************************************************************/

InputMap::InputMap(QObject*parent, t_input_universe universes) : QObject(parent)
{
	m_universes = universes;
	m_editorUniverse = 0;
	
	initPatch();
	loadPlugins();

#ifdef Q_WS_X11
	/* First, load user devices (overrides system devices) */
	QDir dir(QString(getenv("HOME")));
	loadDevices(dir.absoluteFilePath(QString(USERINPUTDEVICEDIR)));
#endif
	/* Then, load system devices */
	loadDevices(INPUTDEVICEDIR);

	loadDefaults();
}

InputMap::~InputMap()
{
	while (m_plugins.isEmpty() == false)
		delete m_plugins.takeFirst();

	while (m_devices.isEmpty() == false)
		delete m_devices.takeFirst();
}

/*****************************************************************************
 * Universes
 *****************************************************************************/

t_input_universe InputMap::universes()
{
	return m_universes;
}

QStringList InputMap::universeNames()
{
	QStringList inputs;
	QStringList list;

	for (t_input_universe i = 0; i < m_universes; i++)
	{
		if (m_patch[i]->plugin() != NULL)
		{
			inputs = m_patch[i]->plugin()->inputs();
			list << QString("%1: %2/%3").arg(i + 1)
				.arg(m_patch[i]->plugin()->name())
				.arg(inputs.at(m_patch[i]->input()));
		}
		else
		{
			/* No assignment */
			list << QString("%1: %2").arg(i + 1).arg(KInputNone);
		}
	}

	return list;
}

t_input_universe InputMap::editorUniverse() const
{
	return m_editorUniverse;
}

void InputMap::setEditorUniverse(t_input_universe uni)
{
	if (uni < universes())
		m_editorUniverse = uni;
	else
		m_editorUniverse = 0;
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void InputMap::slotValueChanged(QLCInPlugin* plugin, t_input input,
				t_input_channel channel, t_input_value value)
{
	for (t_input_universe i = 0; i < m_universes; i++)
	{
		if (m_patch[i]->plugin() == plugin &&
		    m_patch[i]->input() == input)
		{
			emit inputValueChanged(i, channel, value);
		}
	}
}

void InputMap::feedBack(t_input_universe universe, t_input_channel channel,
			t_input_value value)
{
	InputPatch* patch;
	Q_ASSERT(universe < m_patch.size());

	patch = m_patch[universe];
	Q_ASSERT(patch != NULL);

	if (patch->plugin() != NULL)
		patch->plugin()->feedBack(patch->input(), channel, value);
}

/*****************************************************************************
 * Patch
 *****************************************************************************/

void InputMap::initPatch()
{
	for (t_input_universe i = 0; i < m_universes; i++)
		m_patch.insert(i, new InputPatch(this));
}

bool InputMap::setPatch(t_input_universe universe,
			const QString& pluginName, t_input input,
			const QString& deviceName)
{
	/* Check that the universe that we're doing mapping for is valid */
	if (universe >= m_universes)
	{
		qWarning() << "Universe" << universe << "out of bounds.";
		return false;
	}

	/* Don't care if plugin or device is NULL. */
	m_patch[universe]->set(plugin(pluginName), input,
			       device(deviceName));

	return true;
}

InputPatch* InputMap::patch(t_input_universe universe)
{
	Q_ASSERT(universe < m_universes);
	return m_patch[universe];
}

/*****************************************************************************
 * Plugins
 *****************************************************************************/

void InputMap::loadPlugins()
{
	QString path;

#ifdef __APPLE__
	path = QString("%1/%2").arg(QApplication::applicationDirPath())
				.arg(INPUTPLUGINDIR);
#else
	path = QString(INPUTPLUGINDIR);
#endif

	/* Find plugins from INPUTPLUGINDIR, sort by name, get regular files */
	QDir dir(path, QString("*%1").arg(PLUGINEXT), QDir::Name, QDir::Files);

	/* Check that we can access the directory */
	if (dir.exists() == false || dir.isReadable() == false)
	{
		qWarning() << "Unable to load input plugins from"
			   << dir.absolutePath();
		return;
	}

	/* Loop thru all files in the directory */
	QStringListIterator it(dir.entryList());
	while (it.hasNext() == true)
	{
		QLCInPlugin* p;
		QString path;

		/* Attempt to load a plugin from the path */
		path = dir.absoluteFilePath(it.next());
		QPluginLoader loader(path, this);
		p = qobject_cast<QLCInPlugin*> (loader.instance());
		if (p != NULL)
		{
			/* Check for duplicates */
			if (plugin(p->name()) == NULL)
			{
				/* New plugin. Append and init. */
				p->init();
				appendPlugin(p);
				p->connectInputData(this);
			}
			else
			{
				/* Duplicate plugin. Unload it. */
				qWarning() << "Discarded duplicate plugin"
					   << path;
				loader.unload();
			}
		}
		else
		{
			qWarning() << "No input plugin in" << path;
		}
	}
}

QStringList InputMap::pluginNames()
{
	QListIterator <QLCInPlugin*> it(m_plugins);
	QStringList list;

	while (it.hasNext() == true)
		list.append(it.next()->name());

	return list;
}

QStringList InputMap::pluginInputs(const QString& pluginName)
{
	QLCInPlugin* ip = plugin(pluginName);
	if (ip == NULL)
		return QStringList();
	else
		return ip->inputs();
}

void InputMap::configurePlugin(const QString& pluginName)
{
	QLCInPlugin* inputPlugin = plugin(pluginName);
	if (inputPlugin == NULL)
	{
		QMessageBox::warning(_app,
				     tr("Unable to configure plugin"),
				     tr("Plugin \"%1\" not found!")
				     .arg(pluginName));
	}
	else
		inputPlugin->configure();
}

QString InputMap::pluginStatus(const QString& pluginName, t_input input)
{
	QLCInPlugin* inputPlugin = NULL;
	QString info;

	if (pluginName != QString::null)
		inputPlugin = plugin(pluginName);

	if (inputPlugin != NULL)
		info = inputPlugin->infoText(input);
	else
		info = tr("No information");

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
 * Devices
 *****************************************************************************/

void InputMap::loadDevices(const QString& devicePath)
{
	/* Find *.qxi from devicePath, sort by name, get regular files */
	QDir dir(devicePath, QString("*%1").arg(KExtInputDevice),
		 QDir::Name, QDir::Files);
	if (dir.exists() == false || dir.isReadable() == false)
	{
		qWarning() << "Unable to load input devices from"
			   << devicePath;
		return;
	}

	/* Go thru all found file entries and attempt to load an input
	   device from each of them. */
	QStringListIterator it(dir.entryList());
	while (it.hasNext() == true)
	{
		QLCInputDevice* dev;
		QString path;

		path = dir.absoluteFilePath(it.next());
		dev = QLCInputDevice::loader(path);
		if (dev != NULL)
		{
			/* Check for duplicates */
			if (device(dev->name()) == NULL)
				m_devices.append(dev);
			else
				delete dev;
		}
		else
		{
			qWarning() << "Unable to find an input device from"
				   << path;
		}
	}
}

QStringList InputMap::deviceNames()
{
	QStringList list;
	QListIterator <QLCInputDevice*> it(m_devices);
	while (it.hasNext() == true)
		list << it.next()->name();
	return list;
}

QLCInputDevice* InputMap::device(const QString& name)
{
	QListIterator <QLCInputDevice*> it(m_devices);
	while (it.hasNext() == true)
	{
		QLCInputDevice* dev = it.next();
		if (dev->name() == name)
			return dev;
	}

	return NULL;
}

void InputMap::addDevice(QLCInputDevice* device)
{
	Q_ASSERT(device != NULL);
	m_devices.append(device);
}

void InputMap::removeDevice(const QString& name)
{
	QLCInputDevice* device;
	QMutableListIterator <QLCInputDevice*> it(m_devices);
	while (it.hasNext() == true)
	{
		device = it.next();
		if (device->name() == name)
		{
			it.remove();
			delete device;
			break;
		}
	}
}

/*****************************************************************************
 * Save & Load
 *****************************************************************************/

bool InputMap::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(wksp_root != NULL);

	/* InputMap entry */
	root = doc->createElement(KXMLQLCInputMap);
	wksp_root->appendChild(root);

	/* Editor universe */
	tag = doc->createElement(KXMLQLCInputMapEditorUniverse);
	root.appendChild(tag);
	text = doc->createTextNode(QString("%1").arg(editorUniverse()));
	tag.appendChild(text);

	/* Patches */
	for (t_input_universe i = 0; i < m_universes; i++)
		m_patch[i]->saveXML(doc, &root, i);

	return true;
}

bool InputMap::loadXML(QDomDocument* doc, QDomElement* root)
{
	QDomNode node;
	QDomElement tag;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCInputMap)
	{
		qWarning() << "InputMap node not found!";
		return false;
	}

	/* Children */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();

		if (tag.tagName() == KXMLQLCInputPatch)
			InputPatch::loader(doc, &tag, this);
		else if (tag.tagName() == KXMLQLCInputMapEditorUniverse)
			setEditorUniverse(tag.text().toInt());
		else
			qWarning() << "Unknown InputMap tag:" << tag.tagName();

		node = node.nextSibling();
	}

	return true;
}

/*****************************************************************************
 * Defaults
 *****************************************************************************/

void InputMap::loadDefaults()
{
	QString deviceName;
	QSettings settings;
	QString plugin;
	QVariant value;
	QString input;
	QString key;

	/* Editor universe */
	key = QString("/inputmap/editoruniverse/");
	value = settings.value(key);
	if (value.isValid() == true)
		setEditorUniverse(value.toInt());

	for (t_input_universe i = 0; i < m_universes; i++)
	{
		/* Plugin name */
		key = QString("/inputmap/universe%2/plugin/").arg(i);
		plugin = settings.value(key).toString();

		/* Plugin input */
		key = QString("/inputmap/universe%2/input/").arg(i);
		input = settings.value(key).toString();

		/* Input device */
		key = QString("/inputmap/universe%2/device/").arg(i);
		deviceName = settings.value(key).toString();

		/* Do the mapping */
		if (plugin.length() > 0 && input.length() > 0)
			setPatch(i, plugin, input.toInt(), deviceName);
	}
}

void InputMap::saveDefaults()
{
	QSettings settings;
	QString key;
	QString str;

	for (t_input_universe i = 0; i < m_universes; i++)
	{
		InputPatch* pat = patch(i);
		Q_ASSERT(pat != NULL);

		/* Editor universe */
		key = QString("/inputmap/editoruniverse/");
		settings.setValue(key, m_editorUniverse);

		if (pat->plugin() != NULL)
		{
			/* Plugin name */
			key = QString("/inputmap/universe%2/plugin/").arg(i);
			settings.setValue(key, pat->plugin()->name());

			/* Plugin input */
			key = QString("/inputmap/universe%2/input/").arg(i);
			settings.setValue(key, str.setNum(pat->input()));

			/* Input device */
			key = QString("/inputmap/universe%2/device/").arg(i);
			settings.setValue(key, pat->deviceName());
		}
		else
		{
			/* Plugin name */
			key = QString("/inputmap/universe%2/plugin/").arg(i);
			settings.setValue(key, "");

			/* Plugin input */
			key = QString("/inputmap/universe%2/input/").arg(i);
			settings.setValue(key, "");

			/* Input device */
			key = QString("/inputmap/universe%2/device/").arg(i);
			settings.setValue(key, "");
		}
	}
}
