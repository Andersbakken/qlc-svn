/*
  Q Light Controller
  outputpatch.h

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

#ifndef OUTPUTPATCH_H
#define OUTPUTPATCH_H

#include <QObject>

#include "common/qlctypes.h"

class QDomDocument;
class QDomElement;

class QLCOutPlugin;
class OutputMap;

#define KXMLQLCOutputPatch "Patch"
#define KXMLQLCOutputPatchUniverse "Universe"
#define KXMLQLCOutputPatchPlugin "Plugin"
#define KXMLQLCOutputPatchOutput "Output"

class OutputPatch : public QObject
{
	Q_OBJECT

	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	OutputPatch(QObject* parent);
	virtual ~OutputPatch();

private:
	Q_DISABLE_COPY(OutputPatch)

	/********************************************************************
	 * Plugin & output
	 ********************************************************************/
public:
	void set(QLCOutPlugin* plugin, int output);

	QLCOutPlugin* plugin() const { return m_plugin; }
	QString pluginName() const;

	int output() const { return m_output; }
	QString outputName() const;

protected:
	QLCOutPlugin* m_plugin;
	int m_output;

	/********************************************************************
	 * Value dump
	 ********************************************************************/
public:
	/** Write the contents of a 512 channel value buffer to the plugin.
	  * Called periodically by OutputMap. No need to call manually. */
	void dump(const char* universe);
};

#endif
