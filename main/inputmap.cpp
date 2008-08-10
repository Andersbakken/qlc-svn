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
#include <QDir>

#include "common/qlcinplugin.h"
#include "common/qlctypes.h"

#include "inputmap.h"
#include "app.h"

using namespace std;

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

InputMap::InputMap()
{
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
