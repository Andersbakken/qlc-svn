/*
  Q Light Controller
  qlcinputprofile.h

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

#ifndef QLCINPUTPROFILE_H
#define QLCINPUTPROFILE_H

#include <QStringList>
#include <QString>
#include <QHash>
#include <QMap>

#include <common/qlctypes.h>

class QLCInputChannel;
class QLCInputProfile;
class QDomDocument;
class QDomElement;

#define KXMLQLCInputProfile "InputProfile"
#define KXMLQLCInputProfileManufacturer "Manufacturer"
#define KXMLQLCInputProfileModel "Model"

#define KXMLQLCInputProfileMap "Map"
#define KXMLQLCInputProfileMapFrom "From"
#define KXMLQLCInputProfileMapTo "To"

class QLC_DECLSPEC QLCInputProfile
{
	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	/** Standard constructor */
	QLCInputProfile();

	/** Copy constructor */
	QLCInputProfile(const QLCInputProfile& profile);

	/** Destructor */
	virtual ~QLCInputProfile();

	/** Assignment operator */
	QLCInputProfile& operator=(const QLCInputProfile& profile);

	/********************************************************************
	 * Profile information
	 ********************************************************************/
public:
	void setManufacturer(const QString& manufacturer);
	QString manufacturer() const { return m_manufacturer; }

	void setModel(const QString& model);
	QString model() const { return m_model; }

	/** Get the profile name (manufacturer - model) */
	QString name() const;

	/** Get the path where the profile is stored in. Don't use
	    this as a unique ID since this varies between platforms. */
	QString path() const;

protected:
	QString m_manufacturer;
	QString m_model;
	QString m_path;

	/********************************************************************
	 * Channels
	 ********************************************************************/
public:
	/**
	 * Add a new channel to this profile and claim ownership of the channel.
	 *
	 * @param ich The input channel to add. The channel contains the channel
	 *            number to map to. Any existing mapping is cleared.
	 */
	void addChannel(QLCInputChannel* ich);

	/**
	 * Remove the given channel mapping from this profile. Does not delete
	 * the instance.
	 *
	 * @param ich The channel object to remove
	 */
	void removeChannel(QLCInputChannel* ich);

	/**
	 * Remove a channel from the given channel number from this profile.
	 * Also deletes the instance.
	 *
	 * @param channel The channel number to remove
	 */
	void removeChannel(t_input_channel channel);

	/**
	 * Get a channel object by a channel number.
	 *
	 * @param channel The number of the channel to get
	 * @return A QLCInputChannel* or NULL if not found
	 */
	QLCInputChannel* channel(t_input_channel channel) const;

	/**
	 * Get the name of the given channel.
	 *
	 * @param channel The number of the channel, whose name to get
	 * @return The name of the channel or QString::null if not found
	 */
	QString channelName(t_input_channel channel) const;

	/** Get available channels in a non-modifiable map */
	const QMap <t_input_channel, QLCInputChannel*> channels() const
		{ return m_channels; }

protected:
	/** Channel objects present in this profile. This is a QMap and not a
	    QList because not all channels might be present. */
	QMap <t_input_channel, QLCInputChannel*> m_channels;

	/********************************************************************
	 * Channel mapping
	 ********************************************************************/
public:
	/** Set up a real input channel to show up as a different channel.
	  * Remove mapping when to == KInputChannelInvalid. */
	void setMapping(t_input_channel from, t_input_channel to);

	/** Get the mapped channel for a real input channel */
	t_input_channel mapping(t_input_channel from) const;

	/** Get the real channel for a mapped channel (slow) */
	t_input_channel reverseMapping(t_input_channel to) const;

	/** Get a readily-mapped channel object for the given mapped channel */
	const QLCInputChannel* mappedChannel(t_input_channel to) const;
	
	/** Get the cross-channel mappings as a non-modifiable hash */
	const QHash <t_input_channel, t_input_channel> mapping() const
		{ return m_mapping; }

protected:
	/** Channel cross-mapping. From (real) = key, to (mapped) = value.
	    This is not an attribute of QLCInputChannel to prevent multiple
	    mappings to/from the same channel. */
	QHash <t_input_channel, t_input_channel> m_mapping;
	
	/********************************************************************
	 * Load & Save
	 ********************************************************************/
public:
	/** Load an input profile from the given path */
	static QLCInputProfile* loader(const QString& path);

	/** Save an input profile into a given file name */
	bool saveXML(const QString& fileName);

protected:
	/** Load an input profile from the given document */
	bool loadXML(const QDomDocument* doc);

	/** Save channel mappings */
	bool saveXMLMappings(QDomDocument* doc, QDomElement* root) const;
};

#endif
