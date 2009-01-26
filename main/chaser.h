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

class Event;
class QFile;
class QString;
class QDomDocument;

class Chaser : public Function
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/** Constructor */
	Chaser(QObject* parent);

	/**
	 * Copy the contents of the given chaser into this chaser.
	 *
	 * @param ch Chaser to copy from
	 * @param append If true, existing contents will not be cleared;
	 *               new steps appear after existing steps. If false,
	 *               the existing steps of this chaser are cleared.
	 */
	void copyFrom(Chaser* ch, bool append = false);

	/** Destructor */
	virtual ~Chaser();

private:
	Q_DISABLE_COPY(Chaser)

	/*********************************************************************
	 * Chaser contents
	 *********************************************************************/
public:
	/** Add the given function to the end of this chaser's step list */
	void addStep(t_function_id fid);

	/** Remove a function from the given step index */
	void removeStep(unsigned int index = 0);

	/** Raise the given step once (move it one step earlier) */
	bool raiseStep(unsigned int index);

	/** Lower the given step once (move it one step later) */
	bool lowerStep(unsigned int index);

	/** Get this chaser's list of steps */
	QList <t_function_id> *steps() { return &m_steps; }

protected:
	QList <t_function_id> m_steps;

	/*********************************************************************
	 * Save & Load
	 *********************************************************************/
public:
	/** Save this function to an XML document */
	bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

	/** Load this function contents from an XML document */
	bool loadXML(QDomDocument* doc, QDomElement* root);

	/*********************************************************************
	 * Running
	 *********************************************************************/
protected slots:
	/** Skip to next step when a tapped signal is received from bus */
	void slotBusTapped(t_bus_id id);

	/** Slot that receives child functions' stopped() signals and
	    toggles next chaser step. */
	void slotChildStopped(t_function_id id);

public:
	/** Stop this function */
	void stop();

	/** Allocate everything needed in run-time */
	void arm();

	/** Delete everything needed in run-time */
	void disarm();

	/** Do some post-run cleanup (called by FunctionConsumer) */
	void cleanup();

protected:
	/** Main producer thread */
	void run();

	/** Start a step function at the given index */
	void startMemberAt(int index);

	/** Stop a step function at the given index */
	void stopMemberAt(int index);

protected:
	bool m_childRunning;
	bool m_stopped;

	Direction m_runTimeDirection;
	int m_runTimePosition;
};

#endif
