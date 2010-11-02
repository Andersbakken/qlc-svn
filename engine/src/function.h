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

#include <QWaitCondition>
#include <QObject>
#include <QString>
#include <QMutex>
#include <QList>

#include "qlctypes.h"

class QDomDocument;
class QDomElement;

class UniverseArray;
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
     * @param doc The parent object that owns this function (Doc)
     */
    Function(Doc* doc);

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
    t_function_id id() const;

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
    virtual QString name() const;

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
    virtual Function::RunOrder runOrder() const;

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
    virtual Function::Direction direction() const;

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
    quint32 busID() const;

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
     * @return true if successful, otherwise false
     */
    static bool loader(const QDomElement* root, Doc* doc);

    /*********************************************************************
     * Flash
     *********************************************************************/
public:
    /** Flash the function */
    virtual void flash(MasterTimer* timer);

    /** UnFlash the function */
    virtual void unFlash(MasterTimer* timer);

    /** Check, whether the function is flashing */
    virtual bool flashing() const;

signals:
    /**
     * Tells listeners that this function is flashing or that is just
     * stopped flashing.
     *
     * @param fid The flashing function's ID
     * @param state true if the function flashing, false if the function
     *              just stopped flashing
     */
    void flashing(t_function_id fid, bool state);

private:
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
     * Called by MasterTimer when the function is started. MasterTimer's
     * function list mutex is locked during this call, so functions must
     * not attempt to start/stop additional functions from their preRun()
     * methods because it would result in a deadlock.
     *
     * @param timer The MasterTimer instance that takes care of running
     *              the function in correct intervals.
     */
    virtual void preRun(MasterTimer* timer);

    /**
     * Write next values to universes. This method is called periodically
     * by the MasterTimer instance that the
     * function has been set to run in, to write the next values to the
     * given DMX universe buffer. This method is called for each function
     * in the order that they were set to run. When this method has been
     * called for each running function, the buffer is written to OutputMap.
     *
     * MasterTimer calls this method for each function to get their DMX
     * data for the given array of universes. This method will be called
     * for each running function until Function::stopped() returns true.
     *
     * @param timer The MasterTimer that is running the function
     * @param universes The DMX universe buffer to write values into
     */
    virtual void write(MasterTimer* timer, UniverseArray* universes) = 0;

    /**
     * Called by MasterTimer when the function is stopped. No more write()
     * calls will arrive to the function after this call. The function may
     * still write its last data packet to universes during this call.
     * Used by e.g. EFX to write its stop scene values. MasterTimer's
     * function list mutex is locked during this call, so functions must
     * not attempt to start/stop additional functions from their postRun()
     * methods because it would result in a deadlock.
     *
     * @param timer The MasterTimer that has stopped running the function
     * @param universes Universe buffer to write the function's exit data
     */
    virtual void postRun(MasterTimer* timer, UniverseArray* universes);

	/**
     * Check, whether the function was started by another function.
     *
	 * @return true If the function was started by another function.
     *              Otherwise false.
	 */
    bool initiatedByOtherFunction() const;

    /**
     * Set function as "started by another function".
     *
     * @param state true to set the function as started by another.
     */
    void setInitiatedByOtherFunction(bool state);

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

private:
    bool m_initiatedByOtherFunction;

    /*********************************************************************
     * Elapsed
     *********************************************************************/
public:
    /**
     * Get number of elapsed ticks for this function (0 unless the function
     * is running).
     *
     * @return Number of elapsed timer ticks since the function was started
     */
    quint32 elapsed() const;

protected:
    /** Reset elapsed timer ticks to zero */
    void resetElapsed();

    /** Increment the elapsed timer ticks by one */
    void incrementElapsed();

private:
    quint32 m_elapsed;

    /*********************************************************************
     * Stopping
     *********************************************************************/
public:
    /**
     * Mark the function to be stopped ASAP. MasterTimer will stop running
     * the function on the next pass after this method has been called.
     * There is no way to cancel it, but the function can be started again
     * normally.
     */
    virtual void stop();

    /**
     * Check, whether the function should be stopped ASAP.
     *
     * @return true if the function should be stopped, otherwise false.
     */
    virtual bool stopped() const;

    /**
     * Mark the function to be stopped and block the calling thread until it is
     * actually stopped. To prevent deadlocks the function only waits for 2s.
     *
     * @return true if the function was stopped. false if the function did not
     *              stop withing two seconds
     */
    bool stopAndWait();

private:
    /** Stop flag, private to keep functions from modifying it. */
    bool m_stop;

    QMutex m_stopMutex;
    QWaitCondition m_functionStopped;
};

#endif
