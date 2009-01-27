/*
  Q Light Controller
  bus.h

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

#ifndef BUS_H
#define BUS_H

#include <QObject>
#include <QList>
#include "common/qlctypes.h"

class QDomDocument;
class QDomElement;
class QString;
class QFile;

class BusEmitter;
class Function;

#define KXMLQLCBus "Bus"
#define KXMLQLCBusID "ID"
#define KXMLQLCBusName "Name"
#define KXMLQLCBusValue "Value"
#define KXMLQLCBusLowLimit "LowLimit"
#define KXMLQLCBusHighLimit "HighLimit"

#define KXMLQLCBusRole "Role"
#define KXMLQLCBusFade "Fade"
#define KXMLQLCBusTap "Tap"

class Bus
{
	/********************************************************************
	 * Initialization
	 ********************************************************************/
protected:
	/** Constructor */
	Bus();

	/** Destructor */
	~Bus();

protected:
	t_bus_id m_id;

public:
	/** Initialize buses */
	static void init();

	/** Get the BusEmitter singleton */
	static const BusEmitter* emitter() { return s_busEmitter; }

	/********************************************************************
	 * Value
	 ********************************************************************/
public:
	/** Get the value of a bus */
	static t_bus_value value(t_bus_id id);

	/** Set the value of a bus (emits the value as well) */
	static bool setValue(t_bus_id id, t_bus_value value);

protected:
	t_bus_value m_value;

	/********************************************************************
	 * Name
	 ********************************************************************/
public:
	/** Get the name of a bus */
	static const QString& name(t_bus_id id);

	/** Set the name of a bus */
	static bool setName(t_bus_id id, const QString& name);

protected:
	QString m_name;

	/********************************************************************
	 * Tap
	 ********************************************************************/
public:
	/** Emit a tapped signal */
	static bool tap(t_bus_id id);

	/********************************************************************
	 * Load & Save
	 ********************************************************************/
public:
	/** Load all buses from an XML document */
	static bool loadXML(QDomDocument* doc, QDomElement* root);

	/** Save all buses to an XML document */
	static bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

protected:
	static Bus* s_busArray;
	static t_bus_id s_nextID;
	static BusEmitter* s_busEmitter;
};

/*****************************************************************************
 * Bus Emitter class to handle signal emission
 *****************************************************************************/

class BusEmitter : public QObject
{
	Q_OBJECT

	friend class Bus;

public:
	BusEmitter() {};
	~BusEmitter() {};

private:
	Q_DISABLE_COPY(BusEmitter)

protected:
	void emitValueChanged(t_bus_id id, t_bus_value value)
		{ emit valueChanged(id, value); }

	void emitNameChanged(t_bus_id id, const QString& name)
		{ emit nameChanged(id, name); }

	void emitTapped(t_bus_id id)
		{ emit tapped(id); }

signals:
	void valueChanged(t_bus_id id, t_bus_value value);
	void nameChanged(t_bus_id id, const QString& name);
	void tapped(t_bus_id id);
};

#endif
