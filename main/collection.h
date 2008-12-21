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

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	Collection(QObject* parent);
	virtual ~Collection();

	/** Copy given function's contents to this */
	void copyFrom(const Collection* fc, bool append = false);

	/** Collections don't belong to any particular fixture */
	void setFixture(t_fixture_id) { /* NOP */ }

private:
	/* Disable copying with a copy constructor & operator= */
	Q_DISABLE_COPY(Collection)

	/*********************************************************************
	 * Contents
	 *********************************************************************/
public:
	/** Add a function to this collection */
	void addItem(t_function_id fid);

	/** Remove a function from this collection */
	void removeItem(t_function_id fid);

	/** Get this function's list of steps */
	QList <t_function_id> *steps() { return &m_steps; }

protected:
	QList <t_function_id> m_steps;

	/*********************************************************************
	 * Save & Load
	 *********************************************************************/
public:
	/** Save function's contents to an XML document */
	bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

	/** Load function's contents from an XML document */
	bool loadXML(QDomDocument* doc, QDomElement* root);

	/*********************************************************************
	 * Running
	 *********************************************************************/
public:
	/** Allocate some stuff for run-time */
	void arm();

	/** Delete run-time allocations */
	void disarm();

	/** Stop this function */
	void stop();

protected:
	/** Main producer thread */
	void run();

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
