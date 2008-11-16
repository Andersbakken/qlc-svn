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

/* These macros define platform-specific paths & extensions for input plugins */
#ifdef WIN32
	#define PLUGINPATH "C:\\QLC\\Plugins\\Input"
	#define PLUGINEXT ".dll"
#else
	#ifdef __APPLE__
	#define PLUGINPATH "/../Plugins/input"
	#define PLUGINEXT ".dylib"
	#else
	#define PLUGINPATH "/usr/lib/qlc/input"
	#define PLUGINEXT ".so"
	#endif
#endif

/*****************************************************************************
 * Initialization
 *****************************************************************************/

InputMap::InputMap(QObject*parent, t_input_universe universes) : QObject(parent)
{
	m_universes = universes;

	initPatch();
	loadPlugins();

#ifdef Q_WS_X11
	/* First, load user templates (overrides system templates) */
	QDir dir(QString(getenv("HOME")));
	loadTemplates(dir.absoluteFilePath(QString(USERINPUTTEMPLATEDIR)));
#endif
	/* Then, load system templates */
	loadTemplates(INPUTTEMPLATEDIR);
	
	loadDefaults();
}

InputMap::~InputMap()
{
	for (t_input_universe i = 0; i < m_universes; i++)
		delete m_patch[i];

	while (m_plugins.isEmpty() == false)
		delete m_plugins.takeFirst();
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
			const QString& templateName)
{
	/* Check that the universe that we're doing mapping for is valid */
	if (universe >= m_universes)
	{
		qWarning() << "Universe" << universe << "out of bounds.";
		return false;
	}

	/* Don't care if plugin or template is NULL. */
	m_patch[universe]->set(plugin(pluginName), input,
			       deviceTemplate(templateName));

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

QString InputMap::pluginPath() const
{
	return QString(PLUGINPATH);
}

void InputMap::loadPlugins()
{
	/* Find plugins from pluginPath(), sort by name, get regular files */
	QDir dir(pluginPath(), QString("*%1").arg(PLUGINEXT),
		 QDir::Name, QDir::Files);

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
	QStringListIterator it(dir.entryList());
	while (it.hasNext() == true)
	{
		QLCInPlugin* plugin;
		QString path;

		path = dir.absoluteFilePath(it.next());

		QPluginLoader loader(path, this);
		plugin = qobject_cast<QLCInPlugin*> (loader.instance());
		if (plugin == NULL)
		{
			qWarning() << "Unable to find an input plugin from"
				   << path;
		}
		else
		{
			plugin->init();
			appendPlugin(plugin);
			plugin->connectInputData(this);
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
 * Device templates
 *****************************************************************************/

void InputMap::loadTemplates(QString templatePath)
{
	/* Find *.qxi from templatePath(), sort by name, get regular files */
	QDir dir(templatePath, QString("*%1").arg(KExtInputTemplate),
		 QDir::Name, QDir::Files);
	if (dir.exists() == false || dir.isReadable() == false)
	{
		qWarning() << "Unable to load input templates from"
			   << templatePath;
		return;
	}

	/* Go thru all found file entries and attempt to load an input
	   template from each of them. */
	QStringListIterator it(dir.entryList());
	while (it.hasNext() == true)
	{
		QLCInputDevice* dt;
		QString path;

		path = dir.absoluteFilePath(it.next());
		dt = QLCInputDevice::loader(this, path);
		if (dt != NULL)
		{
			/* Check for duplicates */
			if (deviceTemplate(dt->name()) == NULL)
				m_deviceTemplates.append(dt);
			else
				delete dt;
		}
		else
		{
			qWarning() << "Unable to find an input template from"
				   << path;
		}
	}
}

QStringList InputMap::deviceTemplateNames()
{
	QStringList list;
	QListIterator <QLCInputDevice*> it(m_deviceTemplates);
	while (it.hasNext() == true)
		list << it.next()->name();
	return list;
}

QLCInputDevice* InputMap::deviceTemplate(const QString& name)
{
	QLCInputDevice* deviceTemplate;
	QListIterator <QLCInputDevice*> it(m_deviceTemplates);
	while (it.hasNext() == true)
	{
		deviceTemplate = it.next();
		if (deviceTemplate->name() == name)
			return deviceTemplate;
	}
	
	return NULL;
}

void InputMap::addDeviceTemplate(QLCInputDevice* deviceTemplate)
{
	Q_ASSERT(deviceTemplate != NULL);
	m_deviceTemplates.append(deviceTemplate);
}

void InputMap::removeDeviceTemplate(const QString& name)
{
	QLCInputDevice* deviceTemplate;
	QMutableListIterator <QLCInputDevice*> it(m_deviceTemplates);
	while (it.hasNext() == true)
	{
		deviceTemplate = it.next();
		if (deviceTemplate->name() == name)
		{
			it.remove();
			delete deviceTemplate;
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

	Q_ASSERT(doc != NULL);
	Q_ASSERT(wksp_root != NULL);

	/* InputMap entry */
	root = doc->createElement(KXMLQLCInputMap);
	wksp_root->appendChild(root);

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

	/* Patches */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		
		if (tag.tagName() == KXMLQLCInputPatch)
			InputPatch::loader(doc, &tag, this);
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
	QString templateName;
	QSettings settings;
	QString plugin;
	QString input;
	QString key;

	for (t_input_universe i = 0; i < m_universes; i++)
	{
		/* Plugin name */
		key = QString("/inputmap/universe%2/plugin/").arg(i);
		plugin = settings.value(key).toString();

		/* Plugin input */
		key = QString("/inputmap/universe%2/input/").arg(i);
		input = settings.value(key).toString();

		/* Input template */
		key = QString("/inputmap/universe%2/template/").arg(i);
		templateName = settings.value(key).toString();

		/* Do the mapping */
		if (plugin.length() > 0 && input.length() > 0)
			setPatch(i, plugin, input.toInt(), templateName);
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

		/* Save empty values if the patch doesn't have a valid plugin */
		if (pat->plugin() != NULL)
		{
			/* Plugin name */
			key = QString("/inputmap/universe%2/plugin/").arg(i);
			settings.setValue(key, pat->plugin()->name());

			/* Plugin input */
			key = QString("/inputmap/universe%2/input/").arg(i);
			settings.setValue(key, str.setNum(pat->input()));

			/* Device template */
			key = QString("/inputmap/universe%2/template/").arg(i);
			settings.setValue(key, pat->templateName());
		}
		else
		{
			/* Plugin name */
			key = QString("/inputmap/universe%2/plugin/").arg(i);
			settings.setValue(key, "");

			/* Plugin input */
			key = QString("/inputmap/universe%2/input/").arg(i);
			settings.setValue(key, "");

			/* Device template */
			key = QString("/inputmap/universe%2/template/").arg(i);
			settings.setValue(key, "");
		}
	}
}
