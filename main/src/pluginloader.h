/*
  Q Light Controller
  pluginloader.h

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

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

class DMXMap;
class InputMap;
class Plugin;

class PluginLoader
{
public:
	/**
	 * Load all input and output plugins and feed them to input map
	 * or output map, depending on the plugins' type.
	 *
	 * This is a static function.
	 *
	 * @param pluginPath Path to load plugin libraries from
	 * @param outputMap Entity that holds all output type plugins
	 * @param inputMap Entity that holds all input type plugins
	 */
	static void load(const QString& pluginPath,
			 DMXMap* outputMap, InputMap* inputMap);

protected:
	/**
	 * Create a Plugin object from the given file path
	 *
	 * @param path A full path to a .so file (supposedly containing a
	 *             valid QLC plugin).
	 * @param outputMap Entity that holds all output type plugins
	 * @param inputMap Entity that holds all input type plugins
	 */
	static void create(const QString& path,
			   DMXMap* outputMap, InputMap* inputMap);
};

#endif
