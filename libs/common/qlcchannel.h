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

#include <QList>

#include "qlctypes.h"

#define KXMLQLCChannel          QString("Channel")
#define KXMLQLCChannelNumber    QString("Number")
#define KXMLQLCChannelName      QString("Name")
#define KXMLQLCChannelGroup     QString("Group")
#define KXMLQLCChannelGroupByte QString("Byte")

#define KQLCChannelGroupIntensity   QString("Intensity")
#define KQLCChannelGroupColour      QString("Colour")
#define KQLCChannelGroupGobo        QString("Gobo")
#define KQLCChannelGroupPrism       QString("Prism")
#define KQLCChannelGroupShutter     QString("Shutter")
#define KQLCChannelGroupBeam        QString("Beam")
#define KQLCChannelGroupSpeed       QString("Speed")
#define KQLCChannelGroupEffect      QString("Effect")
#define KQLCChannelGroupPan         QString("Pan")
#define KQLCChannelGroupTilt        QString("Tilt")
#define KQLCChannelGroupMaintenance QString("Maintenance")
#define KQLCChannelGroupNothing     QString("Nothing")

class QFile;
class QString;
class QDomDocument;
class QDomElement;
class QStringList;
class QLCCapability;
class QLCChannel;

class QLC_DECLSPEC QLCChannel
{
public:
	/** Standard constructor */
	QLCChannel();

	/** Copy constructor */
	QLCChannel(const QLCChannel* channel);

	/** Create contents from an XML tag */
	QLCChannel(const QDomElement* tag);

	/** Destructor */
	~QLCChannel();

	/** Assignment operator */
	QLCChannel& operator=(const QLCChannel& lc);

	/*********************************************************************
	 * Channel groups
	 *********************************************************************/
public:
	/** Get a list of possible channel groups */
	static QStringList groupList();

	/** Convert a group name to an index, to be used with menus etc. */
	static int groupToIndex(QString group);

	/** Convert an index to a group name, to be used with menus etc. */
	static QString indexToGroup(int index);

	/*********************************************************************
	 * Properties
	 *********************************************************************/
public:
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
public:
	/** Get a list of channel's capabilities */
	const QList <QLCCapability*> capabilities() const { return m_capabilities; }

	/** Search for a particular capability by its channel value */
	QLCCapability* searchCapability(t_value value) const;

	/** Search for a particular capability by its name */
	QLCCapability* searchCapability(QString name) const;

	/** Add a new capability to the channel */
	bool addCapability(QLCCapability* cap);

	/** Remove a capability from the channel */
	bool removeCapability(QLCCapability* cap);

protected:
	void sortCapabilities();

	/*********************************************************************
	 * File operations
	 *********************************************************************/
public:
	/** Save the channel to a QDomDocument, under the given element */
	void saveXML(QDomDocument* doc, QDomElement* root);

	/** Load channel contents from an XML element */
	bool loadXML(const QDomElement* tag);

protected:
	/** Name */
	QString m_name;

	/** List of channel's capabilities */
	QList <QLCCapability*> m_capabilities;

	/** Channel's group */
	QString m_group;

	t_controlbyte m_controlByte;
};

#endif
