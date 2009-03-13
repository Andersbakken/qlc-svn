/*
  Q Light Controller
  inputpatch.h

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

#ifndef INPUTPATCH_H
#define INPUTPATCH_H

#include <QObject>

#include "common/qlcinputdevice.h"
#include "common/qlctypes.h"

class InputPatchEditor;
class InputMapEditor;
class QLCInPlugin;
class InputMap;

class QDomDocument;
class QDomElement;

#define KXMLQLCInputPatchDevice "Device"
#define KXMLQLCInputPatchUniverse "Universe"
#define KXMLQLCInputPatchPluginNone "None"
#define KXMLQLCInputPatchPlugin "Plugin"
#define KXMLQLCInputPatchInput "Input"
#define KXMLQLCInputPatch "Patch"

class InputPatch : public QObject
{
	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	InputPatch(QObject* parent);
	virtual ~InputPatch();

private:
	Q_DISABLE_COPY(InputPatch);

	/********************************************************************
	 * Properties
	 ********************************************************************/
public:
	void set(QLCInPlugin* plugin, t_input input, QLCInputDevice* device);

	QLCInPlugin* plugin() const { return m_plugin; }
	QString pluginName() const;

	t_input input() const;
	QString inputName() const;

	QLCInputDevice* device() const { return m_device; }
	QString deviceName() const;

protected:
	QLCInPlugin* m_plugin;
	t_input m_input;
	QLCInputDevice* m_device;

	/********************************************************************
	 * Load & Save
	 ********************************************************************/
public:
	/**
	 * Save an InputPatch's properties into an XML document
	 *
	 * @param doc An XML document to save to
	 * @param map_root An XML root node (InputMap) to save under
	 * @param universe The internal universe number that the patch is
	 *                 addressed to
	 * @return true if successful, otherwise false
	 */
	bool saveXML(QDomDocument* doc, QDomElement* map_root,
		     t_input_universe universe);

	/**
	 * Create and load an InputPatch's properties from an XML document
	 *
	 * @param doc An XML document to load from
	 * @param root An XML node containing an InputPatch to load from
	 * @param inputMap InputMap object that contains the loaded patch
	 * @return true if successful, otherwise false
	 */
	static bool loader(const QDomElement* root, InputMap* inputMap);
};

#endif
