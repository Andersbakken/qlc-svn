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

#include <qptrlist.h>
#include <qobject.h>

#include "common/types.h"

class QFile;
class QString;
class QDomDocument;
class QDomElement;
class Function;
class BusEmitter;

#define KXMLQLCBus "Bus"
#define KXMLQLCBusID "ID"
#define KXMLQLCBusName "Name"
#define KXMLQLCBusValue "Value"

#define KXMLQLCBusRole "Role"
#define KXMLQLCBusHold "Hold"
#define KXMLQLCBusFade "Fade"

class Bus
{
 private:
	/** Constructor */
	Bus();
	
	/** Destructor */
	~Bus();
	
 public:
	/** Initialize buses */
	static void init();

	/** Get the BusEmitter singleton */
	static const BusEmitter* emitter() { return s_busEmitter; }

	/** Get the value of a bus */
	static const bool value(t_bus_id, t_bus_value&);

	/** Set the value of a bus (emits the value as well) */
	static bool setValue(t_bus_id, t_bus_value);

	/** Get the name of a bus */
	static const QString name(t_bus_id);

	/** Set the name of a bus */
	static bool setName(t_bus_id, QString);

	/** Add a new listener for bus value changes */
	static bool addListener(t_bus_id, Function*);

	/** Remove a bus listener */
	static bool removeListener(t_bus_id, Function*);

	/** Load all buses from an XML document */
	static bool loadXML(QDomDocument* doc, QDomElement* root);

	/** Save all buses to an XML document */
	static bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

 private:
	t_bus_id m_id;
	t_bus_value m_value;
	QString m_name;

	QPtrList <Function> m_listeners;

 private:
	static Bus* s_busArray;
	static t_bus_id s_nextID;
	static BusEmitter* s_busEmitter;
};

//
// Bus Emitter class to handle signal emission
//
class BusEmitter : public QObject
{
	Q_OBJECT

		friend class Bus;

 public:
	BusEmitter() {};
	~BusEmitter() {};

 protected:
	void emitValueChanged(t_bus_id id, t_bus_value value)
		{ emit valueChanged(id, value); }

	void emitNameChanged(t_bus_id id, QString name)
		{ emit nameChanged(id, name); }

 signals:
	void valueChanged(t_bus_id, t_bus_value);
	void nameChanged(t_bus_id, const QString&);
};

#endif
