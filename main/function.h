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
#include <QThread>
#include <QString>
#include <QList>

#include "common/qlctypes.h"

class QDomDocument;
class QDomElement;
class QIcon;

class EventBuffer;
class Function;
class Bus;

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

class Function : public QThread
{
	Q_OBJECT

public:
	enum Type
	{
		Undefined  = 0,
		Scene      = 1 << 0,
		Chaser     = 1 << 1,
		EFX        = 1 << 2,
		Collection = 1 << 3
	};

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
	Function(QObject* parent, Function::Type type);

	/**
	 * Destroy this function
	 */
	virtual ~Function();
	
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
	
protected:
	t_function_id m_id;
	
	/*********************************************************************
	 * Name
	 *********************************************************************/
public:
	/**
	 * Set a name for this function
	 *
	 * @param name The function's new name
	 */
	virtual void setName(QString name);
	
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
	/**
	 * Return the type of this function (see the enum above)
	 */
	Function::Type type() const { return m_type; }

	/**
	 * Return the type of this function as a string
	 */
	QString typeString() const;
	
	/**
	 * Convert a type to string
	 *
	 * @param type The type to convert
	 */
	static QString typeToString(Function::Type type);
	
	/**
	 * Convert a string to Type
	 *
	 * @param str The string to convert
	 */
	static Type stringToType(QString str);
	
	/**
	 * Get an icon (representing the function's type) to be used in lists
	 */
	virtual QIcon icon() const;

protected:
	Type m_type;
	
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
	virtual void setRunOrder(Function::RunOrder order);

	/**
	 * Get this function's running order
	 */
	virtual Function::RunOrder runOrder() const { return m_runOrder; }

	/**
	 * Convert a RunOrder to string
	 *
	 * @param order RunOrder to convert
	 */
	static QString runOrderToString(Function::RunOrder);

	/**
	 * Convert a string to RunOrder
	 *
	 * @param str The string to convert
	 */
	static Function::RunOrder stringToRunOrder(QString str);

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
	virtual void setDirection(Function::Direction dir);

	/**
	 * Get this function's direction
	 */
	virtual Function::Direction direction() const { return m_direction; }

	/**
	 * Convert a Direction to a string
	 *
	 * @param dir Direction to convert
	 */
	static QString directionToString(Function::Direction dir);

	/**
	 * Convert a string to Direction
	 *
	 * @param str The string to convert
	 */
	static Function::Direction stringToDirection(QString str);

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
	virtual void setBus(t_bus_id id);

	/**
	 * Get the bus used for setting the speed of this function
	 */
	t_bus_id busID() const { return m_busID; }
	
	/**
	 * Get a textual representation of the function's bus (ID: Name)
	 */
	virtual QString busName() const;

public slots:
	/**
	 * Callback for bus value changes
	 *
	 * @param id The ID of the bus whose value has changed
	 * @param value The changed bus value
	 */
	virtual void slotBusValueChanged(t_bus_id, t_bus_value)
	{ /* NOP */ }

protected:
	t_bus_id m_busID;
	
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
	virtual bool loadXML(QDomDocument* doc, QDomElement* root) = 0;

	/**
	 * Load any function from an XML tag
	 *
	 * @param doc An XML document to load from
	 * @param root An XML root element of a function
	 */
	static Function* loader(QDomDocument* doc, QDomElement* root);

	/*********************************************************************
	 * Running
	 *********************************************************************/
public:
	/**
	 * When the mode is changed to Operate, this is called to make all mem
	 * allocations so they are not done during run-time (and thus creating
	 * huge overhead)
	 */
	virtual void arm() { /* NOP */ }
	
	/**
	 * When the mode is changed back to Design, this is called to free
	 * any run-time allocations.
	 */
	virtual void disarm() { /* NOP */ }

	/**
	 * Start the function
	 */
	virtual bool start();
	
	/**
	 * Stop the function
	 */
	virtual void stop() = 0;

	/**
	 * Tell this function that it has been removed from FunctionConsumer's
	 * list of running functions (as a result of having an emptied-down
	 * EventBuffer, and having finished its run() method). Usually,
	 * functions have little more to do than emit a stopped() signal.
	 */
	virtual void finale() { emit stopped(m_id); }

signals:
	/**
	 * Signal that is emitted to this function's parent(s) when this
	 * function is started
	 */
	void running(t_function_id id);

	/**
	 * Signal that is emitted to this function's parent(s) when this
	 * function is really finished (i.e. removed from FunctionConsumer's
	 * list of running functions).
	 */
	void stopped(t_function_id id);

	/*********************************************************************
	 * Event buffer
	 *********************************************************************/
public:
	/**
	 * Return the eventbuffer object. Only for FunctionConsumer's use.
	 */
	EventBuffer* eventBuffer() const { return m_eventBuffer; }

protected:
	EventBuffer* m_eventBuffer;
};

#endif
