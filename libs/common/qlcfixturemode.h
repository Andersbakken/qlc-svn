/*
  Q Light Controller
  qlcfixturemode.h

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

#ifndef QLCFIXTUREMODE_H
#define QLCFIXTUREMODE_H

#include <QString>
#include <QList>

#include "qlcfixturedef.h"
#include "qlcphysical.h"
#include "qlcchannel.h"
#include "qlctypes.h"

#define KXMLQLCFixtureMode              "Mode"
#define KXMLQLCFixtureModeName          "Name"
#define KXMLQLCFixtureModeChannel       "Channel"
#define KXMLQLCFixtureModeChannelNumber "Number"

class QDomDocument;
class QDomElement;
class QLCFixtureMode;
class QLCFixtureDef;
class QLCPhysical;
class QLCChannel;

class QLC_DECLSPEC QLCFixtureMode
{
public:
	/** Default constructor */
	QLCFixtureMode(QLCFixtureDef* fixtureDef);

	/** Copy constructor */
	QLCFixtureMode(const QLCFixtureMode* mode);

	/** Create contents from an XML tag */
	QLCFixtureMode(QLCFixtureDef* fixtureDef, const QDomElement* tag);

	/** Destructor */
	~QLCFixtureMode();

	/** Assignment operator */
	QLCFixtureMode& operator=(const QLCFixtureMode& mode);

	/*********************************************************************
	 * Name
	 *********************************************************************/
public:
	void setName(const QString &name) { m_name = name; }
	QString name() const { return m_name; }

protected:
	QString m_name;

	/*********************************************************************
	 * Fixture definition
	 *********************************************************************/
public:
	/** Get the fixture that this mode is associated to */
	QLCFixtureDef* fixtureDef() const { return m_fixtureDef; }

protected:
	QLCFixtureDef* m_fixtureDef;

	/*********************************************************************
	 * Channels
	 *********************************************************************/
public:
	/** Insert a channel at the given position */
	void insertChannel(QLCChannel* channel, t_channel index);

	/** Remove a channel (doesn't delete it) */
	bool removeChannel(QLCChannel* channel);

	/** Get a channel by its name */
	QLCChannel* channel(const QString& name);

	/** Get a channel by its index */
	QLCChannel* channel(t_channel ch);

	/** Get the number of channels */
	t_channel channels() { return m_channels.count(); }

	/** Get a channel's index */
	t_channel channelNumber(QLCChannel* channel);

protected:
	QList <QLCChannel*> m_channels;

	/*********************************************************************
	 * Physical
	 *********************************************************************/
public:
	/** Set physical properties */
	void setPhysical(const QLCPhysical &physical);

	/** Get physical properties */
	const QLCPhysical physical();

	/** Load from an XML tag */
	bool loadXML(const QDomElement* root);

	/** Save to an XML document */
	bool saveXML(QDomDocument* doc, QDomElement* root);

protected:
	QLCPhysical m_physical;
};

#endif
