/*
  Q Light Controller
  qlcfixture.h
  
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

#ifndef QLC_FIXTURE_H
#define QLC_FIXTURE_H

#include <qobject.h>
#include <qptrlist.h>
#include <qstring.h>

#include "types.h"

// Fixture document type
#define KXMLQLCFixtureDocument "Fixture"

// Fixture definition XML tags
#define KXMLQLCFixture "Fixture"
#define KXMLQLCFixtureManufacturer "Manufacturer"
#define KXMLQLCFixtureModel "Model"
#define KXMLQLCFixtureType "Type"

// Fixture instance XML tags
#define KXMLQLCFixtureName "Name"
#define KXMLQLCFixtureID "ID"
#define KXMLQLCFixtureUniverse "Universe"
#define KXMLQLCFixtureAddress "Address"

class QDomDocument;
class QLCChannel;
class QLCFixtureMode;
class QLCFixture;
class DeviceClass;

class QLCFixture
{
 public:
	/** Default constructor */
	QLCFixture();

	/** Copy constructor */
	QLCFixture(QLCFixture *dc);

	/** Create contents from an XML file */
	QLCFixture(const QString &fileName);

	/** Create contents from an old DeviceClass */
	QLCFixture(DeviceClass* dc);
 
	/** Destructor */
	~QLCFixture();
	
	/** Assignment operator */
	QLCFixture& operator=(QLCFixture& fixture);
 
	/** Get the fixture's name string (=="manufacturer model") */
	QString name() { return m_manufacturer + QString(" ") + m_model; }
	
	/** Set the fixture's manufacturer string */
	void setManufacturer(const QString mfg);

	/** Set the fixture's manufacturer string */
	QString manufacturer() const { return m_manufacturer; }
	
	/** Set the fixture's model string */
	void setModel(const QString model);

	/** Get the fixture's model string */
	QString model() const { return m_model; }
	
	/** Set the fixture's type string */
	void setType(const QString &type);

	/** Get the fixture's type string */
	QString type() const { return m_type; }
	
	/** Add a new channel to this fixture */
	void addChannel(QLCChannel* channel);

	/** Remove a certain channel from this fixture */
	bool removeChannel(QLCChannel* channel);

	/** Search for a channel by its name */
	QLCChannel* searchChannel(const QString &name);
	
	/** Get all channels */
	QPtrList <QLCChannel> *channels() { return &m_channels; }
	
	/** Add a new mode to this fixture */
	void addMode(QLCFixtureMode* mode);

	/** Remove a certain mode from this fixture */
	bool removeMode(QLCFixtureMode* mode);

	/** Search for a certain mode by its name */
	QLCFixtureMode* searchMode(const QString& name);
	
	/** Get all modes */
	QPtrList <QLCFixtureMode> *modes() { return &m_modes; }
	
	/** Save the fixture into an XML file */
	bool saveXML(const QString &fileName);
	
	/** Load fixture contents from an XML document */
	bool loadXML(QDomDocument* doc); 

 private:
	QString m_manufacturer;
	QString m_model;
	QString m_type;

	/** Available channels */
	QPtrList <QLCChannel> m_channels;

	/** Modes (i.e. particular collections of channels) */
	QPtrList <QLCFixtureMode> m_modes;
};

#endif
