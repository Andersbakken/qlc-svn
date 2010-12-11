/*
  Q Light Controller
  chaser.h

  Copyright (C) Heikki Junnila

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

#ifndef CHASER_H
#define CHASER_H

#include <QList>
#include "function.h"

class QFile;
class QString;
class QDomDocument;
class MasterTimer;
class ChaserRunner;

/**
 * Chaser is a meta-function; it consists of other functions that are run in a
 * sequential order. Chaser contains information only on the running order,
 * direction and interval applied for its member functions. Therefore, a chaser
 * itself is nothing without other functions (more than usually, scenes) as its
 * members. If a member function's behaviour is changed, those changes are also
 * applied automatically to those chasers that hold the changed function(s) as
 * their members.
 */
class Chaser : public Function
{
    Q_OBJECT
    Q_DISABLE_COPY(Chaser)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    Chaser(Doc* doc);
    virtual ~Chaser();

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
     * Chaser contents
     *********************************************************************/
public:
    /**
     * Add the given function to the end of this chaser's step list. The
     * same function can exist any number of times in a chaser. No checks
     * for the function's validity are made at this point, except that the
     * chaser's own ID cannot be added (i.e. a chaser cannot be its own
     * direct member).
     *
     * @param fid The ID of the function to add
     */
    bool addStep(t_function_id fid);

    /**
     * Remove a function from the given step index. If the given index is
     * out of bounds, this returns false.
     *
     * @param index The index number to remove
     * @return true if successful, otherwise false
     */
    bool removeStep(int index);

    /**
     * Raise the given step once from the given index to index - 1. If the
     * index is already 0, this function fails.
     *
     * @param index The index of the step to raise
     * @return true if successful, otherwise false
     */
    bool raiseStep(int index);

    /**
     * Lower the given step once from the given index to index + 1. If the
     * index is already size - 1, this function fails.
     *
     * @param index The index of the step to lower
     * @return true if successful, otherwise false
     */
    bool lowerStep(int index);

    /**
     * Get the chaser's list of steps as function IDs
     *
     * @return List of function IDs
     */
    QList <t_function_id> steps() const;

    /**
     * Get the chaser's list of steps as function pointers
     *
     * @return List of function pointers
     */
    QList <Function*> stepFunctions() const;

public slots:
    /**
     * Catches Doc::functionRemoved() so that destroyed members can be
     * removed immediately. This method removes all occurrences of the
     * given function ID, while removeStep() only removes one function ID
     * at the given index.
     *
     * @param fid The ID of the function that was removed
     */
    void slotFunctionRemoved(t_function_id fid);

protected:
    QList <t_function_id> m_steps;

    /*********************************************************************
     * Save & Load
     *********************************************************************/
public:
    /** Save this function to an XML document */
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

    /** Load this function contents from an XML document */
    bool loadXML(const QDomElement* root);

    /*********************************************************************
     * Running
     *********************************************************************/
protected slots:
    void slotBusTapped(quint32 id);

public:
    /** @reimpl */
    void arm();

    /** @reimpl */
    void disarm();

    /** @reimpl */
    void preRun(MasterTimer* timer);

    /** @reimpl */
    //void postRun(MasterTimer* timer, UniverseArray* universes);

    /** @reimpl */
    void write(MasterTimer* timer, UniverseArray* universes);
#if 0
protected:
    /** Check, whether the chaser has gone a full round. Returns false
        if singleshot == true and a full round has been completed. */
    bool roundCheck();

    /** Increment or decrement the next function position */
    void nextStep();

    /** Start the current function */
    void startCurrent(MasterTimer* timer);

    /** Stop the current function */
    void stopCurrent();
#endif
protected:
    bool m_tapped;
    ChaserRunner* m_runner;
    Direction m_runTimeDirection;
    int m_runTimePosition;
};

#endif
