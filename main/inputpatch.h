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

#include "common/qlcinputprofile.h"
#include "common/qlctypes.h"

class InputPatchEditor;
class InputMapEditor;
class QLCInPlugin;
class InputMap;

class QDomDocument;
class QDomElement;

#define KXMLQLCInputPatchProfile "Profile"
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
	void set(QLCInPlugin* plugin, t_input input, QLCInputProfile* profile);

	QLCInPlugin* plugin() const { return m_plugin; }
	QString pluginName() const;

	t_input input() const;
	QString inputName() const;

	QLCInputProfile* profile() const { return m_profile; }
	QString profileName() const;

protected:
	QLCInPlugin* m_plugin;
	t_input m_input;
	QLCInputProfile* m_profile;
};

#endif
