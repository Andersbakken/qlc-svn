/*
  Q Light Controller
  qlcinputdevice.h

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

#ifndef QLCINPUTDEVICE_H
#define QLCINPUTDEVICE_H

#include <QStringList>
#include <QString>
#include <QObject>
#include <QMap>

#include <common/qlctypes.h>

class QLCInputChannel;
class QDomDocument;
class QDomElement;

#define KXMLQLCInputTemplate "InputTemplate"
#define KXMLQLCInputTemplateManufacturer "Manufacturer"
#define KXMLQLCInputTemplateModel "Model"

class QLC_DECLSPEC QLCInputDevice : public QObject
{
	Q_OBJECT

	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	/** Standard constructor */
	QLCInputDevice(QObject* parent);

	/** Copy constructor */
	QLCInputDevice(const QLCInputDevice& inputDevice);

	/** Destructor */
	virtual ~QLCInputDevice();

	/** Assignment operator */
	QLCInputDevice& operator=(const QLCInputDevice& inputDevice);

	/********************************************************************
	 * Device information
	 ********************************************************************/
public:
	void setManufacturer(const QString& manufacturer);
	QString manufacturer() const { return m_manufacturer; }

	void setModel(const QString& model);
	QString model() const { return m_model; }

	/** Get the device name (manufacturer - model) */
	QString name() const;

	/** Get the path where the device template is stored in. Don't use
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
	 * Add a new channel to this device and claim ownership of the channel.
	 *
	 * @param ich The input channel to add. The channel contains the channel
	 *            number to map to. Any existing mapping is cleared.
	 */
	void addChannel(QLCInputChannel* ich);

	/**
	 * Remove the given channel mapping from this device. Does not delete
	 * the instance.
	 *
	 * @param ich The channel object to remove
	 */
	void removeChannel(QLCInputChannel* ich);

	/**
	 * Remove a channel from the given channel number from this device.
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
	QLCInputChannel* channel(t_input_channel channel);

	/**
	 * Get the name of the given channel.
	 *
	 * @param channel The number of the channel, whose name to get
	 * @return The name of the channel or QString::null if not found
	 */
	QString channelName(t_input_channel channel);

	/**
	 * Get available channels in a non-modifiable map
	 */
	const QMap <t_input_channel, QLCInputChannel*> channels()
		{ return m_channels; }

protected:
	/** Channel objects present in this device. This is a QMap and not a
	    QList because not all channels might be present. */
	QMap <t_input_channel, QLCInputChannel*> m_channels;

	/********************************************************************
	 * Load & Save
	 ********************************************************************/
public:
	/** Load an input template from the given path */
	static QLCInputDevice* loader(QObject* parent, const QString& path);

	/** Save an input template into a given file name */
	bool saveXML(const QString& fileName);

protected:
	/** Load an input template from the given document */
	bool loadXML(QDomDocument* doc);
};

#endif
