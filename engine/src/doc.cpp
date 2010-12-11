/*
  Q Light Controller
  doc.cpp

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

#include <QStringList>
#include <QString>
#include <QDebug>
#include <QList>
#include <QtXml>
#include <QDir>

#include "qlcfixturedefcache.h"
#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlcfile.h"

#include "collection.h"
#include "function.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "efx.h"
#include "doc.h"

Doc::Doc(QObject* parent, const QLCFixtureDefCache& fixtureDefCache)
        : QObject(parent)
        , m_mode(Design)
        , m_fixtureDefCache(fixtureDefCache)
{
    // Allocate fixture array
    m_fixtureArray = (Fixture**) malloc(sizeof(Fixture*) *
                                        KFixtureArraySize);
    for (t_fixture_id i = 0; i < KFixtureArraySize; i++)
        m_fixtureArray[i] = NULL;
    m_fixtureAllocation = 0;

    // Allocate function array
    m_functionArray = (Function**) malloc(sizeof(Function*) *
                                          KFunctionArraySize);
    for (t_function_id i = 0; i < KFunctionArraySize; i++)
        m_functionArray[i] = NULL;
    m_functionAllocation = 0;

    /* Connect to bus emitter so that Doc can be marked as modified when
       bus name changes. */
    connect(Bus::instance(), SIGNAL(nameChanged(quint32,const QString&)),
            this, SLOT(slotBusNameChanged()));

    resetModified();
}

Doc::~Doc()
{
    // Delete all functions
    for (t_function_id i = 0; i < KFunctionArraySize; i++)
    {
        if (m_functionArray[i] != NULL)
        {
            delete m_functionArray[i];
            m_functionArray[i] = NULL;

            emit functionRemoved(i);
        }
    }
    delete [] m_functionArray;
    m_functionAllocation = 0;

    // Delete all fixture instances
    for (t_fixture_id i = 0; i < KFixtureArraySize; i++)
    {
        if (m_fixtureArray[i] != NULL)
        {
            delete m_fixtureArray[i];
            m_fixtureArray[i] = NULL;

            emit fixtureRemoved(i);
        }
    }
    delete [] m_fixtureArray;
    m_fixtureAllocation = 0;
}

/*****************************************************************************
 * Modified status
 *****************************************************************************/

bool Doc::isModified() const
{
    return m_modified;
}

void Doc::setModified()
{
    m_modified = true;
    emit modified(true);
}

void Doc::resetModified()
{
    m_modified = false;
    emit modified(false);
}

/*****************************************************************************
 * Main operating mode
 *****************************************************************************/

void Doc::setMode(Doc::Mode mode)
{
    /* Don't do mode switching twice */
    if (m_mode == mode)
        return;
    m_mode = mode;

    /* Go thru all functions and arm/disarm them, depending on new mode */
    for (int i = 0; i < KFunctionArraySize; i++)
    {
        Function* function = m_functionArray[i];
        if (function == NULL)
            continue;
        else if (mode == Design)
            function->disarm();
        else if (mode == Operate)
            function->arm();
    }

    emit modeChanged(m_mode);
}

Doc::Mode Doc::mode() const
{
    return m_mode;
}

/*************************************************************************
 * Fixture definition cache
 *************************************************************************/

const QLCFixtureDefCache& Doc::fixtureDefCache() const
{
    return m_fixtureDefCache;
}

/*****************************************************************************
 * Fixtures
 *****************************************************************************/

bool Doc::addFixture(Fixture* fixture, t_fixture_id id)
{
    bool ok = false;

    Q_ASSERT(fixture != NULL);

    if (m_fixtureAllocation == KFixtureArraySize)
        return false;

    if (id == Fixture::invalidId())
    {
        /* Find the next free slot for a new fixture */
        for (t_fixture_id i = 0; i < KFixtureArraySize; i++)
        {
            if (m_fixtureArray[i] == NULL)
            {
                assignFixture(fixture, i);
                ok = true;
                break;
            }
        }
    }
    else if (id >= 0 && id < KFunctionArraySize)
    {
        if (m_fixtureArray[id] == NULL)
        {
            assignFixture(fixture, id);
            ok = true;
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unable to assign fixture"
                       << fixture->name() << "to ID" << id
                       << "because another fixture already has the same ID.";
        }
    }
    else
    {
        /* Pure and honest epic fail */
    }

    return ok;
}

bool Doc::deleteFixture(t_fixture_id id)
{
    if (id < 0 || id > KFixtureArraySize)
        return false;

    if (m_fixtureArray[id] != NULL)
    {
        delete m_fixtureArray[id];
        m_fixtureArray[id] = NULL;
        m_fixtureAllocation--;

        emit fixtureRemoved(id);
        setModified();
        return true;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "No such fixture ID:" << id;
        return false;
    }
}

Fixture* Doc::fixture(t_fixture_id id) const
{
    if (id >= 0 && id < KFixtureArraySize)
        return m_fixtureArray[id];
    else
        return NULL;
}

quint32 Doc::findAddress(quint32 numChannels) const
{
    /* Try to find contiguous space from one universe at a time */
    for (quint32 universe = 0; universe < KUniverseCount; universe++)
    {
        quint32 ch = findAddress(universe, numChannels);
        if (ch != QLCChannel::invalid())
            return ch;
    }

    return QLCChannel::invalid();
}

quint32 Doc::findAddress(quint32 universe, quint32 numChannels) const
{
    quint32 freeSpace = 0;
    quint32 maxChannels = 512;

    /* Construct a map of unallocated channels */
    int map[maxChannels];
    std::fill(map, map + maxChannels, 0);

    /* Go thru all fixtures and mark their address spaces to the map */
    for (t_fixture_id fxi_id = 0; fxi_id < KFixtureArraySize; fxi_id++)
    {
        Fixture* fxi = m_fixtureArray[fxi_id];
        if (fxi == NULL)
            continue;

        if (fxi->universe() != universe)
            continue;

        for (quint32 ch = 0; ch < fxi->channels(); ch++)
            map[fxi->universeAddress() + ch] = 1;
    }

    /* Try to find the next contiguous free address space */
    for (quint32 ch = 0; ch < maxChannels; ch++)
    {
        if (map[ch] == 0)
            freeSpace++;
        else
            freeSpace = 0;

        if (freeSpace == numChannels)
            return (ch - freeSpace + 1) | (universe << 9);
    }

    return QLCChannel::invalid();
}

int Doc::fixtures() const
{
    return m_fixtureAllocation;
}

int Doc::totalPowerConsumption(int& fuzzy) const
{
    int totalPowerConsumption = 0;

    // Make sure fuzzy starts from zero
    fuzzy = 0;

    for (t_fixture_id i = 0; i < KFixtureArraySize; i++)
    {
        if (m_fixtureArray[i] == NULL)
            continue;

        // Generic dimmer has no mode and physical
        if (m_fixtureArray[i]->isDimmer() == false)
        {
            QLCPhysical phys = m_fixtureArray[i]->fixtureMode()->physical();
            if (phys.powerConsumption() > 0)
                totalPowerConsumption += phys.powerConsumption();
            else
                fuzzy++;
        }
        else
        {
            fuzzy++;
        }
    }

    return totalPowerConsumption;
}

void Doc::assignFixture(Fixture* fixture, t_fixture_id id)
{
    Q_ASSERT(fixture != NULL);
    Q_ASSERT(id >= 0 && id < KFixtureArraySize);

    /* Patch fixture change signals thru Doc */
    connect(fixture, SIGNAL(changed(t_fixture_id)),
            this, SLOT(slotFixtureChanged(t_fixture_id)));

    m_fixtureArray[id] = fixture;
    fixture->setID(id);
    m_fixtureAllocation++;
    emit fixtureAdded(id);

    setModified();
}

/*****************************************************************************
 * Functions
 *****************************************************************************/

bool Doc::addFunction(Function* function, t_function_id id)
{
    bool ok = false;

    Q_ASSERT(function != NULL);

    if (functions() >= KFunctionArraySize)
    {
        qWarning() << Q_FUNC_INFO << "Cannot add more than" << KFunctionArraySize
                   << "functions";
        return false;
    }

    if (id == Function::invalidId())
    {
        /**
         * Find the next free space from function array.
         *
         * @todo Already with a couple hundred functions this becomes
         * unbearably slow. With a thousand functions... Oh boy...!
         */
        for (t_function_id i = 0; i < KFunctionArraySize; i++)
        {
            if (m_functionArray[i] == NULL)
            {
                /* Found a place for the function */
                assignFunction(function, i);
                ok = true;
                break;
            }
        }
    }
    else if (id >= 0 && id < KFunctionArraySize)
    {
        if (m_functionArray[id] == NULL)
        {
            /* Found a place for the function */
            assignFunction(function, id);
            ok = true;
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unable to assign function"
                       << function->name() << "to ID" << id
                       << "because another function already has the same ID.";
        }
    }
    else
    {
        /* Pure and honest epic fail */
    }

    return ok;
}

int Doc::functions() const
{
    return m_functionAllocation;
}

quint32 Doc::functionsFree() const
{
    return KFunctionArraySize - functions();
}

bool Doc::deleteFunction(t_function_id id)
{
    if (m_functionArray[id] != NULL)
    {
        delete m_functionArray[id];
        m_functionArray[id] = NULL;
        m_functionAllocation--;

        emit functionRemoved(id);
        setModified();

        return true;
    }
    else
    {
        return false;
    }
}

Function* Doc::function(t_function_id id)
{
    if (id >= 0 && id < KFunctionArraySize)
        return m_functionArray[id];
    else
        return NULL;
}

void Doc::assignFunction(Function* function, t_function_id id)
{
    Q_ASSERT(function != NULL);
    Q_ASSERT(id >= 0 && id < KFunctionArraySize);

    /* Pass function change signals thru Doc */
    connect(function, SIGNAL(changed(t_function_id)),
            this, SLOT(slotFunctionChanged(t_function_id)));

    /* Make the function listen to fixture removals so that it can
       get rid of nonexisting members. */
    connect(this, SIGNAL(fixtureRemoved(t_fixture_id)),
            function, SLOT(slotFixtureRemoved(t_fixture_id)));

    m_functionAllocation++;
    m_functionArray[id] = function;
    function->setID(id);
    emit functionAdded(id);
    setModified();
}

void Doc::slotFunctionChanged(t_function_id fid)
{
    setModified();
    emit functionChanged(fid);
}

/*****************************************************************************
 * Monitoring/listening methods
 *****************************************************************************/

void Doc::slotFixtureChanged(t_fixture_id id)
{
    setModified();
    emit fixtureChanged(id);
}

void Doc::slotBusNameChanged()
{
    setModified();
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool Doc::loadXML(const QDomElement* root)
{
    QDomElement tag;
    QDomNode node;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCEngine)
    {
        qWarning() << Q_FUNC_INFO << "Engine node not found";
        return false;
    }

    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();

        if (tag.tagName() == KXMLFixture)
        {
            Fixture::loader(&tag, this);
        }
        else if (tag.tagName() == KXMLQLCFunction)
        {
            Function::loader(&tag, this);
        }
        else if (tag.tagName() == KXMLQLCBus)
        {
            Bus::instance()->loadXML(&tag);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown engine tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool Doc::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Create the master Engine node */
    root = doc->createElement(KXMLQLCEngine);
    wksp_root->appendChild(root);

    /* Write fixtures into an XML document */
    for (t_fixture_id i = 0; i < KFixtureArraySize; i++)
    {
        if (m_fixtureArray[i] != NULL)
        {
            m_fixtureArray[i]->saveXML(doc, &root);
        }
    }

    /* Write functions into an XML document */
    for (t_function_id i = 0; i < KFunctionArraySize; i++)
    {
        if (m_functionArray[i] != NULL)
        {
            m_functionArray[i]->saveXML(doc, &root);
        }
    }

    /* Write buses */
    Bus::instance()->saveXML(doc, &root);

    return true;
}
