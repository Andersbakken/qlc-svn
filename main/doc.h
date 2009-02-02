/*
  Q Light Controller
  doc.h

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

#ifndef DOC_H
#define DOC_H

#include <QObject>
#include <QList>

#include "function.h"
#include "fixture.h"
#include "app.h"
#include "bus.h"

class QDomDocument;
class QLCFixtureDef;
class QLCFixtureMode;

#define KXMLQLCWorkspace "Workspace"

class Doc : public QObject
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	Doc();
	~Doc();

private:
	Q_DISABLE_COPY(Doc)

	/*********************************************************************
	 * Modified status
	 *********************************************************************/
public:
	/**
	 * Check, whether Doc has been modified (and is in need of saving)
	 */
	bool isModified() { return m_modified; }

	/**
	 * Set Doc into modified state (i.e. it is in need of saving)
	 */
	void setModified();

	/**
	 * Reset Doc's modified state (i.e. it is no longer in need of saving)
	 */
	void resetModified();

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	/**
	 * Get the name of the current workspace file
	 */
	QString fileName() { return m_fileName; }

	/**
	 * Load the Doc's contents from the given XML file
	 *
	 * @param fileName The name of the file to load from
	 * @return TRUE if successful, otherwise FALSE
	 */
	bool loadXML(const QString& fileName);

	/**
	 * Load the Doc's contents from the given XML document
	 *
	 * @param doc The XML document to read from
	 * @return TRUE if successful, otherwise FALSE
	 */
	bool loadXML(QDomDocument* doc);

	/**
	 * Save the Doc's contents to the given XML file. Also resets
	 * the doc's modified status.
	 *
	 * @param fileName The name of the file to save to
	 * @return TRUE if successful, otherwise FALSE
	 */
	bool saveXML(const QString& fileName);

	/*********************************************************************
	 * Fixture Instances
	 *********************************************************************/
public:
	/**
	 * Create a new fixture instance from the given fixture definition
	 *
	 * @param fixture A fixture definition from which to create the instance
	 * @param name The friendly name of the fixture instance
	 * @param address The fixture's DMX address
	 * @param universe The fixture's DMX universe
	 * @param name The fixture's friendly name
	 */
	Fixture* newFixture(QLCFixtureDef* fixtureDef,
			    QLCFixtureMode* mode,
			    t_channel address,
			    t_channel universe,
			    QString name);

	/**
	 * Create a new generic fixture instance
	 *
	 * @param address The fixture's DMX address
	 * @param universe The fixture's DMX universe
	 * @param channels Number of channels to use for the fixture
	 * @param name The fixture's friendly name
	 */
	Fixture* newGenericFixture(t_channel address,
				   t_channel universe,
				   t_channel channels,
				   QString name);

	/**
	 * Insert a new fixture instance into Doc's fixture array. Use this
	 * ONLY when loading a workspace from a file.
	 *
	 * @param fxi The fixture instance to insert
	 * @return TRUE if successful, otherwise FALSE
	 */
	bool newFixture(Fixture* fxi);

	/**
	 * Delete the given fixture instance from Doc
	 *
	 * @param id The ID of the fixture instance to delete
	 */
	bool deleteFixture(t_fixture_id id);

	/**
	 * Get the fixture instance that has the given ID
	 *
	 * @param id The ID of the fixture to get
	 */
	Fixture* fixture(t_fixture_id id);

	/**
	 * Attempt to find the next contiguous free address space for the given
	 * number of channels. The address might span multiple universes. If
	 * an address cannot be found, returns KChannelInvalid.
	 */
	t_channel findAddress(t_channel numChannels);

signals:
	/** Signal that a fixture has been added */
	void fixtureAdded(t_fixture_id fxi_id);

	/** Signal that a fixture has been removed */
	void fixtureRemoved(t_fixture_id fxi_id);

	/** Signal that a fixture's properties have changed */
	void fixtureChanged(t_fixture_id fxi_id);

	/*********************************************************************
	 * Functions
	 *********************************************************************/
public:
	/**
	 * Create a new function
	 *
	 * @param type The type of the new function
	 * @param fxi_id The ID of the fixture instance that owns the function
	 */
	Function* newFunction(Function::Type type);

	/**
	 * Insert a new function instance into Doc's function array. Use this
	 * ONLY when loading a workspace from a file.
	 *
	 * @param type The type of the new function
	 * @param fid The ID of the function (as loaded from a file)
	 * @param name The name of the function
	 * @param doc An XML document to load the function from
	 * @param root An XML "Function" tag to load the function contents from
	 */
	Function* newFunction(Function::Type type, t_function_id fid,
			      QString name, QDomDocument* doc,
			      QDomElement* root);

protected:
	/**
	 * Create a new function that is of the given type
	 *
	 * @param type Type of function to create
	 */
	Function* createFunction(Function::Type type);

public:
	/**
	 * Delete the given function
	 *
	 * @param id The ID of the function to delete
	 */
	void deleteFunction(t_function_id id);

	/**
	 * Get a function that has the given ID
	 *
	 * @param id The ID of the function to get
	 */
	Function* function(t_function_id id);

public:
	/**
	 * Emit a functionChanged() signal.
	 *
	 * Because Functions are not QObjects, they cannot emit signals by
	 * themselves. Therefore, they must call this function to make Doc
	 * emit the function change signal, instead.
	 *
	 * @param id The ID of the function that needs to be signalled
	 */
	void emitFunctionChanged(t_function_id id);

public slots:
	/** Catch QLC App mode changes */
	void slotModeChanged(App::Mode mode);

	/** Catch fixture property changes */
	void slotFixtureChanged(t_fixture_id fxi_id);

signals:
	/** Signal that this Doc has been modified (or unmodified) */
	void modified(bool state);

	/** Signal that a function has been added */
	void functionAdded(t_function_id function);

	/** Signal that a function has been removed */
	void functionRemoved(t_function_id function);

	/** Signal that a function has been changed */
	void functionChanged(t_function_id function);

protected:
	/** Current Doc file name */
	QString m_fileName;

	/** Modified status (true; needs saving, false; does not) */
	bool m_modified;

	/** Array that holds all functions */
	Function** m_functionArray;

	/** Array that holds all fixtures */
	Fixture** m_fixtureArray;
};

#endif

