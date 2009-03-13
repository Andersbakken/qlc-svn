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

class Function : public QObject
{
	Q_OBJECT

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

private:
	Q_DISABLE_COPY(Function)

	/*********************************************************************
	 * Copying
	 *********************************************************************/
public:
	/** Create a copy of this function and return it. The function is put
	    to Doc automatically. */
	virtual Function* createCopy() = 0;

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

protected:
	t_bus_id m_busID;

	/*********************************************************************
	 * Edit
	 *********************************************************************/
public:
	/** Edit the function. Returns QDialog::DialogCode. */
	virtual int edit() = 0;

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
	 * Load any function from an XML tag
	 *
	 * @param doc An XML document to load from
	 * @param root An XML root element of a function
	 */
	static Function* loader(const QDomElement* root);

	/*********************************************************************
	 * Flash
	 *********************************************************************/
public:
	virtual void flash();
	virtual void unFlash();

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

	/** Start the function */
	virtual void start();

	/** Stop the function */
	virtual void stop();

	bool isRunning() const { return m_running; }

	/** Write next values to universes */
	virtual bool write(QByteArray* universes) = 0;

signals:
	/**
	 * Signal that is emitted to this function's parent(s) when this
	 * function is started
	 */
	void running(t_function_id id);

	/**
	 * Signal that is emitted to this function's parent(s) when this
	 * function is really finished (i.e. removed from Master Timer's
	 * list of running functions).
	 */
	void stopped(t_function_id id);

protected:
	bool m_running;
	t_bus_value m_elapsed;
};

#endif
