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
	 * Values
	 ********************************************************************/
public:
	/** Get the value for a channel from the internal buffer */
	t_value value(t_channel channel) const;

	/** Set the value for a channel to the internal buffer */
	void setValue(t_channel channel, t_value value);

	/** Write the contents of the internal buffer to the actual plugin.
	    Called periodically by OutputMap. You don't need to call this. */
	void dump();

	/** Check, whether this universe is in blackout mode */
	bool blackout() const;

	/** Set or release this universe to/from blackout mode */
	void setBlackout(bool blackout);

protected:
	t_value m_values[512];
	bool m_blackout;

	/********************************************************************
	 * Load & Save
	 ********************************************************************/
public:
	/**
	 * Save a OutputPatch's properties into an XML document
	 *
	 * @param doc An XML document to save to
	 * @param map_root An XML root node (OutputMap) to save under
	 * @param universe The internal universe number that the patch is
	 *                 addressed to
	 * @return true if successful, otherwise false
	 */
	bool saveXML(QDomDocument* doc, QDomElement* map_root, int universe);

	/**
	 * Create and load a OutputPatch's properties from an XML document
	 *
	 * @param doc An XML document to load from
	 * @param root An XML node containing a OutputPatch to load from
	 * @param universe The universe number that the OutputPatch is for
	 * @return true if successful, otherwise false
	 */
	static bool loader(QDomDocument* doc, QDomElement* root,
			   OutputMap* outputMap);
};

#endif
