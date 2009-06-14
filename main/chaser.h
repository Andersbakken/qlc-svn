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

class Chaser : public Function
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/** Constructor */
	Chaser(QObject* parent);

	/** Destructor */
	virtual ~Chaser();

private:
	Q_DISABLE_COPY(Chaser)

	/*********************************************************************
	 * Function type
	 *********************************************************************/
public:
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

protected slots:
	/** Catches Doc::functionRemoved() so that destroyed members can be
	    removed immediately. */
	void slotFunctionRemoved(t_function_id);

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
	void arm();
	void disarm();

	void start();
	void stop();

	bool write(QByteArray* universes);

protected:
	/** Check, whether the chaser has gone a full round. Returns false
	    if singleshot == true and a full round has been completed. */
	bool roundCheck();

	/** Increment or decrement the next function position */
	void nextStep();

	/** Start a step function at the given index */
	void startMemberAt(int index);

	/** Stop a step function at the given index */
	void stopMemberAt(int index);

protected:
	bool m_tapped;
	bool m_stopped;

	Direction m_runTimeDirection;
	int m_runTimePosition;
};

#endif
