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

#include "qlcinplugin.h"
#include "qlcconfig.h"
#include "qlctypes.h"
#include "qlcfile.h"

#include "inputpatch.h"
#include "inputmap.h"

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
}

InputMap::~InputMap()
{
	/* Clear patching table so that when it gets out of scope AFTER this
	   destructor is run, it won't attempt to do close() on already-deleted
	   plugin pointers. */
	for (t_input_universe i = 0; i < m_universes; i++)
	{
		delete m_patch[i];
		m_patch[i] = NULL;
	}

	while (m_plugins.isEmpty() == false)
		delete m_plugins.takeFirst();

	while (m_profiles.isEmpty() == false)
		delete m_profiles.takeFirst();
}

/*****************************************************************************
 * Universes
 *****************************************************************************/

t_input_universe InputMap::universes() const
{
	return m_universes;
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

bool InputMap::feedBack(t_input_universe universe, t_input_channel channel,
			t_input_value value)
{
	if (universe >= m_patch.size())
		return false;

	InputPatch* patch = m_patch[universe];
	Q_ASSERT(patch != NULL);

	if (patch->plugin() != NULL)
	{
		patch->plugin()->feedBack(patch->input(), channel, value);
		return true;
	}
	else
	{
		return false;
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
			const QString& profileName)
{
	/* Check that the universe that we're doing mapping for is valid */
	if (universe >= m_universes)
	{
		qWarning() << "Universe" << universe << "out of bounds.";
		return false;
	}

	/* Don't care if plugin or profile is NULL. It must be possible to
	   clear the patch completely. */
	m_patch[universe]->set(plugin(pluginName), input, profile(profileName));

	return true;
}

InputPatch* InputMap::patch(t_input_universe universe) const
{
	if (universe < m_universes)
		return m_patch[universe];
	else
		return NULL;
}

int InputMap::mapping(const QString& pluginName, t_input input) const
{
	for (int uni = 0; uni < universes(); uni++)
	{
		const InputPatch* p = patch(uni);
		if (p->pluginName() == pluginName && p->input() == input)
			return uni;
	}

	return -1;
}

/*****************************************************************************
 * Plugins
 *****************************************************************************/

void InputMap::loadPlugins()
{
	QString path;

#ifdef __APPLE__
	path = QString("%1/../%2").arg(QApplication::applicationDirPath())
				  .arg(INPUTPLUGINDIR);
#else
	path = QString(INPUTPLUGINDIR);
#endif

	/* Find plugins from input plugin dir, sort by name, get regular files */
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
			qWarning() << "Unable to load an input plugin from"
				   << path << "because:"
				   << loader.errorString();
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
	if (inputPlugin != NULL)
		inputPlugin->configure();
}

QString InputMap::pluginStatus(const QString& pluginName, t_input input)
{
	QLCInPlugin* inputPlugin = NULL;
	QString info;

	if (pluginName.isEmpty() == false)
		inputPlugin = plugin(pluginName);

	if (inputPlugin != NULL)
	{
		info = inputPlugin->infoText(input);
	}
	else
	{
                /* Nothing selected */
                info += tr("<HTML><HEAD><TITLE>None</TITLE></HEAD><BODY>");
                info += tr("<H3>No input plugin selected</H3>");
                info += tr("<P>You can download input plugins from ");
                info += tr("<A HREF=\"http://www.sf.net/projects/qlc/files\">");
                info += tr("http://www.sf.net/projects/qlc/files</A>.");
                info += tr("</BODY></HTML>");
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
 * Profiles
 *****************************************************************************/

void InputMap::loadProfiles(const QString& profilePath)
{
	/* Find *.qxi from profilePath, sort by name, get regular files */
	QDir dir(profilePath, QString("*%1").arg(KExtInputProfile),
		 QDir::Name, QDir::Files);
	if (dir.exists() == false || dir.isReadable() == false)
	{
		qWarning() << "Unable to load input profiles from"
			   << profilePath;
		return;
	}

	/* Go thru all found file entries and attempt to load an input
	   profile from each of them. */
	QStringListIterator it(dir.entryList());
	while (it.hasNext() == true)
	{
		QLCInputProfile* prof;
		QString path;

		path = dir.absoluteFilePath(it.next());
		prof = QLCInputProfile::loader(path);
		if (prof != NULL)
		{
			/* Check for duplicates */
			if (profile(prof->name()) == NULL)
				addProfile(prof);
			else
				delete prof;
		}
		else
		{
			qWarning() << "Unable to find an input profile from"
				   << path;
		}
	}
}

QStringList InputMap::profileNames()
{
	QStringList list;
	QListIterator <QLCInputProfile*> it(m_profiles);
	while (it.hasNext() == true)
		list << it.next()->name();
	return list;
}

QLCInputProfile* InputMap::profile(const QString& name)
{
	QListIterator <QLCInputProfile*> it(m_profiles);
	while (it.hasNext() == true)
	{
		QLCInputProfile* profile = it.next();
		if (profile->name() == name)
			return profile;
	}

	return NULL;
}

bool InputMap::addProfile(QLCInputProfile* profile)
{
	Q_ASSERT(profile != NULL);

	/* Don't add the same profile twice */
	if (m_profiles.contains(profile) == false)
	{
		m_profiles.append(profile);
		return true;
	}
	else
	{
		return false;
	}
}

bool InputMap::removeProfile(const QString& name)
{
	QLCInputProfile* profile;
	QMutableListIterator <QLCInputProfile*> it(m_profiles);
	while (it.hasNext() == true)
	{
		profile = it.next();
		if (profile->name() == name)
		{
			it.remove();
			delete profile;
			return true;
		}
	}

	return false;
}

/*****************************************************************************
 * Defaults
 *****************************************************************************/

void InputMap::loadDefaults()
{
	QString profileName;
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

		/* Input profile */
		key = QString("/inputmap/universe%2/profile/").arg(i);
		profileName = settings.value(key).toString();

		/* Do the mapping */
		if (plugin.length() > 0 && input.length() > 0)
		{
			/* Check that the same plugin & input are not mapped
			   to more than one universe at a time. */
			int m = mapping(plugin, input.toInt());
			if (m == -1 || m == i)
				setPatch(i, plugin, input.toInt(), profileName);
		}
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

			/* Input profile */
			key = QString("/inputmap/universe%2/profile/").arg(i);
			settings.setValue(key, pat->profileName());
		}
		else
		{
			/* Plugin name */
			key = QString("/inputmap/universe%2/plugin/").arg(i);
			settings.setValue(key, "");

			/* Plugin input */
			key = QString("/inputmap/universe%2/input/").arg(i);
			settings.setValue(key, "");

			/* Input profile */
			key = QString("/inputmap/universe%2/profile/").arg(i);
			settings.setValue(key, "");
		}
	}
}
