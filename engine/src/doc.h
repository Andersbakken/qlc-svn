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
#include <QFile>

#include "function.h"
#include "fixture.h"
#include "bus.h"

class QDomDocument;
class QString;

#define KXMLQLCEngine "Engine"

class Doc : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Doc)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    /**
     * Create a new Doc instance for the given parent.
     *
     * @param parent The parent object who owns the Doc instance
     * @param fixtureDefCache A fixture definition cache instance that
     *                        owns all available fixture definitions.
     */
    Doc(QObject* parent, const QLCFixtureDefCache& fixtureDefCache);

    /**
     * Destructor.
     */
    ~Doc();

    /*********************************************************************
     * Main operating mode
     *********************************************************************/
public:
    enum Mode
    {
        Design  = 0, //! Editing allowed
        Operate = 1  //! Running allowed, editing disabled
    };

    void setMode(Mode mode);
    Mode mode() const;

signals:
    void modeChanged(Doc::Mode mode);

protected:
    Mode m_mode;

    /*********************************************************************
     * Fixture definition cache
     *********************************************************************/
public:
    /** Get the fixture definition cache */
    const QLCFixtureDefCache& fixtureDefCache() const;

protected:
    const QLCFixtureDefCache& m_fixtureDefCache;

    /*********************************************************************
     * Modified status
     *********************************************************************/
public:
    /**
     * Check, whether Doc has been modified (and is in need of saving)
     */
    bool isModified() const;

    /**
     * Set Doc into modified state (i.e. it is in need of saving)
     */
    void setModified();

    /**
     * Reset Doc's modified state (i.e. it is no longer in need of saving)
     */
    void resetModified();

protected:
    /** Modified status (true; needs saving, false; does not) */
    bool m_modified;

    /*********************************************************************
     * Fixture Instances
     *********************************************************************/
public:
    /**
     * Add the given fixture to doc's fixture array.
     * If id == Fixture::invalidId(), doc assigns the fixture a new ID and
     * takes ownership, unless there is no more room for more fixtures.
     *
     * If id != Fixture::invalidId(), doc attempts to put the fixture at
     * that exact index, unless another fixture already occupies it.
     *
     * @param fixture The fixture to add
     * @param id The requested ID for the fixture
     * @return true if the fixture was successfully added to doc,
     *         otherwise false.
     */
    bool addFixture(Fixture* fixture, t_fixture_id id = Fixture::invalidId());

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
    Fixture* fixture(t_fixture_id id) const;

    /**
     * Attempt to find the next contiguous free address space for the given
     * number of channels. The address will not span multiple universes.
     * If a suitable address space cannot be found, QLCChannel::invalid() is
     * returned
     *
     * @param numChannels Number of channels in the address space
     * @return The address or QLCChannel::invalid() if not found
     */
    quint32 findAddress(quint32 numChannels) const;

    /**
     * Get the number of fixtures currently present/allocated.
     *
     * @return Number of fixtures
     */
    int fixtures() const;

    /**
     * Get the total power consumption of all fixtures in the current
     * workspace.
     *
     * @param[out] fuzzy Number of fixtures without power consumption value
     * @return Total power consumption
     */
    int totalPowerConsumption(int& fuzzy) const;

protected:
    /**
     * Try to find the next free address from the given universe for
     * the given number of channels. QLCChannel::invalid() is returned if
     * an adequate address range cannot be found.
     *
     * @param universe The universe to search from
     * @param numChannels Number of free channels required
     * @return An address or QLCChannel::invalid() if address space not available
     */
    quint32 findAddress(quint32 universe, quint32 numChannels) const;

    /**
     * Assign the given fixture ID to the fixture, placing the fixture
     * at the same index in m_fixtureArray, increase fixture count and
     * emit fixtureAdded() signal.
     *
     * @param fixture The fixture to assign
     * @param id The ID to assign to the fixture
     */
    void assignFixture(Fixture* fixture, t_fixture_id id);

signals:
    /** Signal that a fixture has been added */
    void fixtureAdded(t_fixture_id fxi_id);

    /** Signal that a fixture has been removed */
    void fixtureRemoved(t_fixture_id fxi_id);

    /** Signal that a fixture's properties have changed */
    void fixtureChanged(t_fixture_id fxi_id);

protected:
    /** Array that holds all fixtures */
    Fixture** m_fixtureArray;

    /** Number of allocated fixtures in fixture array */
    int m_fixtureAllocation;

    /*********************************************************************
     * Functions
     *********************************************************************/
public:
    /**
     * Add the given function to doc's function array.
     * If id == Function::invalidId(), doc assigns the function a new ID
     * and takes ownership, unless there is no more room for more functions.
     *
     * If id != Function::invalidId(), doc attempts to put the function at
     * that exact index, unless another function already occupies it.
     *
     * @param function The function to add
     * @param id The requested ID for the function
     * @return true if the function was successfully added to doc,
     *         otherwise false.
     */
    bool addFunction(Function* function,
                     t_function_id id = Function::invalidId());

    /**
     * Get the number of functions currently present/allocated
     *
     * @return Number of functions
     */
    int functions() const;

    /**
     * Get the number of available function slots.
     *
     * @return Number of functions that sill fit to Doc
     */
    quint32 functionsFree() const;

    /**
     * Delete the given function
     *
     * @param id The ID of the function to delete
     * @return true if the function was found and deleted
     */
    bool deleteFunction(t_function_id id);

    /**
     * Get a function that has the given ID
     *
     * @param id The ID of the function to get
     * @return A function at the given ID or NULL if not found
     */
    Function* function(t_function_id id);

protected:
    /**
     * Assign the given function ID to the function, place the function
     * at the same index in m_functionArray, increase function count and
     * emit functionAdded() signal.
     *
     * @param function The function to assign
     * @param id The ID to assign to the function
     */
    void assignFunction(Function* function, t_function_id id);

public slots:
    /** Catch fixture property changes */
    void slotFixtureChanged(t_fixture_id fxi_id);

protected slots:
    /** Slot that catches function change signals */
    void slotFunctionChanged(t_function_id fid);

    /** Catches bus name changes so that Doc can be marked as modified */
    void slotBusNameChanged();

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
    /** Array that holds all functions */
    Function** m_functionArray;

    /** Number of allocated functions */
    int m_functionAllocation;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /**
     * Load contents from the given XML document
     *
     * @param root The Engine XML root node to load from
     * @return true if successful, otherwise false
     */
    bool loadXML(const QDomElement* root);

    /**
     * Save contents to the given XML file.
     *
     * @param doc The XML document to save to
     * @param wksp_root The workspace root node to save under
     * @return true if successful, otherwise false
     */
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);
};

#endif
