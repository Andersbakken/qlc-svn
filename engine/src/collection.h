/*
  Q Light Controller
  collection.h

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

#ifndef COLLECTION_H
#define COLLECTION_H

#include <QMutex>
#include <QList>

#include "function.h"

class QDomDocument;

class Collection : public Function
{
    Q_OBJECT
    Q_DISABLE_COPY(Collection)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    Collection(Doc* doc);
    virtual ~Collection();

    /** Collections don't belong to any particular fixture */
    void setFixture(t_fixture_id) { /* NOP */ }

    /*********************************************************************
     * Function type
     *********************************************************************/
public:
    /** @reimpl */
    Function::Type type() const;

    /*********************************************************************
     * Copying
     *********************************************************************/
public:
    /** @reimpl */
    Function* createCopy(Doc* doc);

    /** Copy the contents for this function from another function */
    bool copyFrom(const Function* function);

    /*********************************************************************
     * Contents
     *********************************************************************/
public:
    /**
     * Add a function to this collection. If the function is already a
     * member of the collection, this call fails.
     *
     * @param fid The function to add
     * @return true if successful, otherwise false
     */
    bool addFunction(t_function_id fid);

    /**
     * Remove a function from this collection. If the function is not a
     * member of the collection, this call fails.
     *
     * @param fid The function to remove
     * @return true if successful, otherwise false
     */
    bool removeFunction(t_function_id fid);

    /**
     * Get this function's list of member functions
     */
    QList <t_function_id> functions() const;

public slots:
    /** Catches Doc::functionRemoved() so that destroyed members can be
        removed immediately. */
    void slotFunctionRemoved(t_function_id function);

protected:
    QList <t_function_id> m_functions;

    /*********************************************************************
     * Save & Load
     *********************************************************************/
public:
    /** Save function's contents to an XML document */
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

    /** Load function's contents from an XML document */
    bool loadXML(const QDomElement* root);

    /*********************************************************************
     * Running
     *********************************************************************/
public:
    /** @reimpl */
    void arm();

    /** @reimpl */
    void disarm();

    /** @reimpl */
    void preRun(MasterTimer* timer);

    /** @reimpl */
    void postRun(MasterTimer* timer, UniverseArray* universes);

    /** @reimpl */
    void write(MasterTimer* timer, UniverseArray* universes);

protected slots:
    /** Called whenever one of this function's child functions stops */
    void slotChildStopped(t_function_id fid);

protected:
    /** Number of currently running children */
    int m_childCount;

    /** Mutex guarding the access to m_childCount */
    QMutex m_childCountMutex;
};

#endif
