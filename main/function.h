/*
  Q Light Controller
  function.h

  Copyright (C) 2004 Heikki Junnila

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

#ifndef FUNCTION_H
#define FUNCTION_H

#include <QObject>
#include <QString>
#include <QList>

#include "common/qlctypes.h"

class QDomDocument;
class QDomElement;
class QIcon;

class MasterTimer;
class Function;
class Bus;
class Doc;

#define KXMLQLCFunction "Function"
#define KXMLQLCFunctionName "Name"
#define KXMLQLCFunctionID "ID"
#define KXMLQLCFunctionType "Type"
#define KXMLQLCFunctionData "Data"

#define KXMLQLCFunctionValue "Value"
#define KXMLQLCFunctionValueType "Type"
#define KXMLQLCFunctionChannel "Channel"

#define KXMLQLCFunctionStep "Step"
#define KXMLQLCFunctionNumber "Number"

#define KXMLQLCFunctionDirection "Direction"
#define KXMLQLCFunctionRunOrder "RunOrder"

#define KXMLQLCFunctionEnabled "Enabled"

class Function : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(Function)

public:
	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/**
	 * Create a new function
	 *
	 * @param parent The parent object that owns this function (Doc)
	 * @param type The type of this function
	 */
	Function(QObject* parent);

	/**
	 * Destroy this function
	 */
	virtual ~Function();

	/*********************************************************************
	 * Copying
	 *********************************************************************/
public:
	/**
	 * Create a copy of a function to the given doc object and return the
	 * newly-created function, unless an error has occurred.
	 *
	 * @param doc The QLC Doc object that owns all functions
	 * @return The newly-created function or NULL in case of an error
	 */
	virtual Function* createCopy(Doc* doc) = 0;

	/** Copy this function's contents from the given function */
	virtual bool copyFrom(const Function* function);

	/*********************************************************************
	 * ID
	 *********************************************************************/
public:
	/**
	 * Set this function's unique ID
	 *
	 * @param id This function's unique ID
	 */
	void setID(t_function_id id);

	/**
	 * Get this function's unique ID
	 */
	t_function_id id() const { return m_id; }

	/**
	 * Get the value for an invalid function ID (for comparison etc.)
	 */
	static t_function_id invalidId();

protected:
	t_function_id m_id;

signals:
	/** Signal telling that the contents of this function have changed */
	void changed(t_function_id fid);

	/*********************************************************************
	 * Name
	 *********************************************************************/
public:
	/**
	 * Set a name for this function
	 *
	 * @param name The function's new name
	 */
	virtual void setName(const QString& name);

	/**
	 * Return the name of this function
	 */
	virtual QString name() const { return m_name; }

protected:
	QString m_name;

	/*********************************************************************
	 * Type
	 *********************************************************************/
public:
	/** This is a bit mask because FunctionSelection does type filtering */
	enum Type
	{
		Undefined  = 0,
		Scene      = 1 << 0,
		Chaser     = 1 << 1,
		EFX        = 1 << 2,
		Collection = 1 << 3
	};

	/**
	 * Return the type of this function (see the enum above)
	 */
	virtual Function::Type type() const = 0;

	/**
	 * Return the type of this function as a string
	 */
	QString typeString() const;

	/**
	 * Convert a type to string
	 *
	 * @param type The type to convert
	 */
	static QString typeToString(const Function::Type& type);

	/**
	 * Convert a string to Type
	 *
	 * @param str The string to convert
	 */
	static Type stringToType(const QString& str);

	/**
	 * Get an icon (representing the function's type) to be used in lists
	 */
	virtual QIcon icon() const;

	/*********************************************************************
	 * Running order
	 *********************************************************************/
public:
	enum RunOrder {	Loop, SingleShot, PingPong };

public:
	/**
	 * Set this function's running order
	 *
	 * @param dir This function's running order
	 */
	virtual void setRunOrder(const Function::RunOrder& order);

	/**
	 * Get this function's running order
	 */
	virtual Function::RunOrder runOrder() const { return m_runOrder; }

	/**
	 * Convert a RunOrder to string
	 *
	 * @param order RunOrder to convert
	 */
	static QString runOrderToString(const Function::RunOrder& order);

	/**
	 * Convert a string to RunOrder
	 *
	 * @param str The string to convert
	 */
	static Function::RunOrder stringToRunOrder(const QString& str);

protected:
	RunOrder m_runOrder;

	/*********************************************************************
	 * Direction
	 *********************************************************************/
public:
	enum Direction { Forward, Backward };

public:
	/**
	 * Set this function's direction
	 *
	 * @param dir This function's direction
	 */
	virtual void setDirection(const Function::Direction& dir);

	/**
	 * Get this function's direction
	 */
	virtual Function::Direction direction() const { return m_direction; }

	/**
	 * Convert a Direction to a string
	 *
	 * @param dir Direction to convert
	 */
	static QString directionToString(const Function::Direction& dir);

	/**
	 * Convert a string to Direction
	 *
	 * @param str The string to convert
	 */
	static Function::Direction stringToDirection(const QString& str);

protected:
	Direction m_direction;

	/*********************************************************************
	 * Bus
	 *********************************************************************/
public:
	/**
	 * Set the function's speed bus
	 *
	 * @param id The ID of the bus
	 */
	virtual void setBus(quint32 id);

	/**
	 * Get the bus used for setting the speed of this function
	 */
	quint32 busID() const { return m_busID; }

protected:
	quint32 m_busID;

	/*********************************************************************
	 * Fixtures
	 *********************************************************************/

public slots:
	/** Slot that captures Doc::fixtureRemoved signals */
	virtual void slotFixtureRemoved(t_fixture_id fxi_id);

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	/**
	 * Save this function to an XML document
	 *
	 * @param doc The XML document to save to
	 * @wksp_root A QLC workspace XML root node to save under
	 */
	virtual bool saveXML(QDomDocument* doc, QDomElement* wksp_root) = 0;

	/**
	 * Read this function's contents from an XML document
	 *
	 * @param doc An XML document to load from
	 * @param root An XML root element of a function
	 */
	virtual bool loadXML(const QDomElement* root) = 0;

	/**
	 * Load a new function from an XML tag and add it to the given doc
	 * object, if loading was successful.
	 *
	 * @param root An XML root element of a function
	 * @param doc The QLC document object, that owns all functions
	 */
	static void loader(const QDomElement* root, Doc* doc);

	/*********************************************************************
	 * Flash
	 *********************************************************************/
public:
	virtual void flash(QByteArray* universes);
	virtual void unFlash(QByteArray* universes);

	virtual bool isFlashing() const { return m_flashing; }

signals:
	void flashing(t_function_id fid, bool state);

protected:
	bool m_flashing;

	/*********************************************************************
	 * Running
	 *********************************************************************/
public:
	/** Allocate run-time stuff */
	virtual void arm() = 0;

	/** Free any run-time allocations */
	virtual void disarm() = 0;

	/**
	 * Start the function in the given MasterTimer instance. A function
	 * can be running only in one MasterTimer at a time.
	 *
	 * @param timer The MasterTimer to run the function in
	 */
	virtual void start(MasterTimer* timer);

	/**
	 * Stop running the function in the given MasterTimer instance.
	 *
	 * @param timer The MasterTimer to stop the function in
	 */
	virtual void stop(MasterTimer* timer);

	/** Check, whether the function is running */
	bool isRunning() const { return m_running; }

	/**
	 * Write next values to universes. This method is called periodically
	 * (once every 1/KFrequency:th of a second) by the MasterTimer that the
	 * function has been set to run in, to write the next values to the
	 * given DMX universe buffer. This method is called for each function
	 * in the order that they were set to run. When this method has been
	 * called for each running function, the buffer is written to OutputMap.
	 *
	 * MasterTimer calls this method for each function to get their DMX
	 * data for the given array of universes. When this method returns
	 * false, MasterTimer immediately removes the function from its list,
	 * stopping the function as a result. This method will be called again
	 * for each running function that returns true.
	 *
	 * @param universes The DMX universe buffer to write values into
	 * @return true if the function has more data for the next round,
	 *         false if the function has done its task and can be stopped.
	 */
	virtual bool write(QByteArray* universes) = 0;

signals:
	/**
	 * Emitted when a function is started (i.e. added to MasterTimer's
	 * list of running functions).
	 *
	 * @param id The ID of the started function
	 */
	void running(t_function_id id);

	/**
	 * Emitted when a function is really finished (i.e. removed from
	 * MasterTimer's list of running functions).
	 *
	 * @param id The ID of the stopped function
	 */
	void stopped(t_function_id id);

protected:
	bool m_running;
	quint32 m_elapsed;
};

#endif
