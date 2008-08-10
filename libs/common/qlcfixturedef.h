/*
  Q Light Controller
  qlcfixturedef.h
  
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

#ifndef QLCFIXTUREDEF_H
#define QLCFIXTUREDEF_H

#include <QString>
#include <QList>

#include "qlctypes.h"

// Fixture document type
#define KXMLQLCFixtureDefDocument "FixtureDefinition"

// Fixture definition XML tags
#define KXMLQLCFixtureDef "FixtureDefinition"
#define KXMLQLCFixtureDefManufacturer "Manufacturer"
#define KXMLQLCFixtureDefModel "Model"
#define KXMLQLCFixtureDefType "Type"

// Fixture instance XML tags
#define KXMLQLCFixtureName "Name"
#define KXMLQLCFixtureID "ID"
#define KXMLQLCFixtureUniverse "Universe"
#define KXMLQLCFixtureAddress "Address"

class QDomDocument;
class QDomElement;
class QLCChannel;
class QLCFixtureMode;
class QLCFixtureDef;

class QLC_DECLSPEC QLCFixtureDef
{
public:
	/** Default constructor */
	QLCFixtureDef();

	/** Copy constructor */
	QLCFixtureDef(QLCFixtureDef *dc);

	/** Create contents from an XML file */
	QLCFixtureDef(const QString &fileName);

	/** Destructor */
	virtual ~QLCFixtureDef();

	/** Assignment operator */
	QLCFixtureDef& operator=(QLCFixtureDef& fixture);
 
	/*********************************************************************
	 * Fixture information
	 *********************************************************************/
public:
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

protected:
	QString m_manufacturer;
	QString m_model;
	QString m_type;

	/*********************************************************************
	 * Channels
	 *********************************************************************/
public:
	/** Add a new channel to this fixture */
	void addChannel(QLCChannel* channel);

	/** Remove a certain channel from this fixture */
	bool removeChannel(QLCChannel* channel);

	/** Search for a channel by its name */
	QLCChannel* channel(const QString &name);
	
	/** Get all channels */
	QList <QLCChannel*> *channels() { return &m_channels; }
	
protected:
	/** Available channels */
	QList <QLCChannel*> m_channels;

	/*********************************************************************
	 * Modes
	 *********************************************************************/
public:
	/** Add a new mode to this fixture */
	void addMode(QLCFixtureMode* mode);

	/** Remove a certain mode from this fixture */
	bool removeMode(QLCFixtureMode* mode);

	/** Get a certain mode by its name */
	QLCFixtureMode* mode(const QString& name);
	
	/** Get all modes */
	QList <QLCFixtureMode*> *modes() { return &m_modes; }
	
	/** Save the fixture into an XML file */
	bool saveXML(const QString &fileName);
	
	/** Load fixture contents from an XML document */
	virtual bool loadXML(QDomDocument* doc); 

protected:
	/** Modes (i.e. particular collections of channels) */
	QList <QLCFixtureMode*> m_modes;
};

#endif
