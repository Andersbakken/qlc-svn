/*
  Q Light Controller
  qlcchannel.h
  
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

#ifndef QLC_CHANNEL_H
#define QLC_CHANNEL_H

#include <qstring.h>
#include <qptrlist.h>
#include "common/types.h"

#define KXMLQLCChannel          "Channel"
#define KXMLQLCChannelNumber    "Number"
#define KXMLQLCChannelName      "Name"
#define KXMLQLCChannelGroup     "Group"
#define KXMLQLCChannelGroupByte "Byte"

#define KIntensityGroup "Intensity"
#define KColourGroup    "Colour"
#define KGoboGroup      "Gobo"
#define KSpeedGroup     "Speed"
#define KEffectGroup    "Effect"
#define KBeamGroup      "Beam"
#define KPanGroup       "Pan"
#define KTiltGroup      "Tilt"

class QFile;
class QDomDocument;
class QDomElement;
class QStringList;
class LogicalChannel;
class QLCCapability;
class QLCChannel;

class QLCChannel
{
 public:
	/** Standard constructor */
	QLCChannel();
	
	/** Copy constructor */
	QLCChannel(QLCChannel* lc);

	/** Create contents from an XML tag */
	QLCChannel(QDomElement* tag);

	/** Create contents from an old LogicalChannel */
	QLCChannel(LogicalChannel* lch);
 
	/** Destructor */
	~QLCChannel();

	/** Assignment operator */
	QLCChannel& operator=(QLCChannel& lc);

	/*********************************************************************
	 * Properties
	 *********************************************************************/

	/** Get the channel's name */
	QString name() const { return m_name; }

	/** Set the channel's name */
	void setName(const QString &name) { m_name = name; }

	/** Get the channel's group */
	QString group() const { return m_group; }

	/** Set the channel's group */
	void setGroup(const QString& group) { m_group = group; }

	/** Set the channel's control byte */
	void setControlByte(t_controlbyte byte) { m_controlByte = byte; }

	/** Get the channel's control byte */
	t_controlbyte controlByte() const { return m_controlByte; }

	/*********************************************************************
	 * Capabilities
	 *********************************************************************/

	/** Get a list of channel's capabilities */
	QPtrList <QLCCapability> *capabilities() { return &m_capabilities; }

	/** Search for a particular capability by its channel value */
	QLCCapability* searchCapability(t_value value);

	/** Search for a particular capability by its name */
	QLCCapability* searchCapability(QString name);

	/** Add a new capability to the channel */
	void addCapability(QLCCapability* cap);

	/** Remove a capability from the channel */
	bool removeCapability(QLCCapability* cap);

	/*********************************************************************
	 * File operations
	 *********************************************************************/

	/** Save the channel to a QDomDocument, under the given element */
	void saveXML(QDomDocument* doc, QDomElement* root);

	/** Load channel contents from an XML element */
	bool loadXML(QDomElement* tag);

 private:
	/** Name */
	QString m_name;

	/** List of channel's capabilities */
	QPtrList <QLCCapability> m_capabilities;

	/** Channel's group */
	QString m_group;

	t_controlbyte m_controlByte;

 public:
	/** Get all available channel groups into a list (cleared first) */
	static void groups(QStringList &list);
};

#endif
