/*
  Q Light Controller
  pluginloader.cpp

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

#include <dlfcn.h>
#include <qdir.h>
#include <qstringlist.h>

#include "common/plugin.h"
#include "common/inputplugin.h"
#include "common/outputplugin.h"

#include "pluginloader.h"
#include "dmxmap.h"
#include "inputmap.h"

void PluginLoader::load(const QString& pluginPath,
			DMXMap* outputMap, InputMap* inputMap)
{
	Plugin* plugin = NULL;

	Q_ASSERT(outputMap != NULL);
	Q_ASSERT(inputMap != NULL);

	QDir pluginDir(pluginPath);
	pluginDir.setFilter(QDir::Files);
	pluginDir.setNameFilter("*.so");

	/* Check that we can access the directory */
	if (pluginDir.exists() == false)
	{
		qDebug(pluginPath + QString(" doesn't exist"));
		return;
	}
	else if (pluginDir.isReadable() == false)
	{
		qDebug(pluginPath + QString(" is not accessible"));
		return;
	}

	/* Loop thru all files in the directory */
	QStringList dirlist(pluginDir.entryList());
	QStringList::Iterator it;
	for (it = dirlist.begin(); it != dirlist.end(); ++it)
		create(pluginPath + QString("/") + *it, outputMap, inputMap);
}

void PluginLoader::create(const QString& path,
			  DMXMap* outputMap, InputMap* inputMap)
{
	QLCPluginCreateFunction create;

	void* pluginHandle = NULL;
	Plugin* plugin = NULL;

	/* Load the (presumed) shared object into memory. Don't resolve
	   symbols until they're needed */
	pluginHandle = ::dlopen(static_cast<const char*> (path), RTLD_LAZY);
	if (pluginHandle == NULL)
	{
		qDebug("Unable to open %s with dlopen(): %s\n", 
		       (const char*) path, ::dlerror());
		return;
	}
	
	/* Attempt to resolve "create" symbol from the shared object */
	create = (QLCPluginCreateFunction) ::dlsym(pluginHandle, "create");
	if (create == NULL)
	{
		::dlclose(pluginHandle);
		qDebug("Unable to resolve symbols for %s. dlsym(): %s", 
		       (const char*) path, ::dlerror());
		return;
	}

	/* Attempt to use the "create" symbol to create a Plugin instance */
	plugin = create();
	Q_ASSERT(plugin != NULL);
	
	/* We accept only output plugins here */
	if (plugin->type() == Plugin::OutputType)
	{
		plugin->setHandle(pluginHandle);

		if (outputMap->appendPlugin(
			    static_cast<OutputPlugin*> (plugin)) == false)
		{
			/* Plugin already exists, delete it */
			delete plugin;
		}
	}
	else if (plugin->type() == Plugin::InputType)
	{
		plugin->setHandle(pluginHandle);

		if (inputMap->appendPlugin(
			    static_cast<InputPlugin*> (plugin)) == false)
		{
			/* Plugin already exists, delete it */
			delete plugin;
		}
	}
	else
	{
		qWarning("Unknown plugin type: %d", plugin->type());
		delete plugin;
		::dlclose(pluginHandle);
	}
}
