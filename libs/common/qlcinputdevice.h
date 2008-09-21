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
protected:
	 QLCInputDevice(QObject* parent);

public:
	virtual ~QLCInputDevice();
	
	/********************************************************************
	 * Device information
	 ********************************************************************/
public:
	void setManufacturer(const QString& manufacturer);
	QString manufacturer() const { return m_manufacturer; }

	void setModel(const QString& model);
	QString model() const { return m_model; }

	QString name() const;

protected:
	QString m_manufacturer;
	QString m_model;

	/********************************************************************
	 * Channels
	 ********************************************************************/
public:
	/**
	 * Add a new channel to this device.
	 *
	 * @param channel The channel number that ich is attached to
	 * @param ich The input channel to attach to the channel number
	 */
	void addChannel(t_input_channel channel, QLCInputChannel* ich);
	
	/**
	 * Remove the given channel from this device
	 *
	 * @param ich The channel object to remove
	 */
	void removeChannel(QLCInputChannel* ich);

	/**
	 * Remove a channel from the given channel number from this device
	 *
	 * @param channel The channel number to remove
	 */
	void removeChannel(t_input_channel channel);

	/**
	 * Get the number of channels present in this device
	 */
	t_input_channel channels() const { return m_channels.count(); }

	/**
	 * Get the channel object from the given channel number
	 *
	 * @param channel The channel number of the object
	 */
	QLCInputChannel* channel(t_input_channel channel);

protected:
	/** Channel objects present in this device */
	QMap <t_input_channel, QLCInputChannel*> m_channels;

	/********************************************************************
	 * Load & Save
	 ********************************************************************/
public:
	/** Get all available input templates' names and file paths */
	static void availableTemplates(QStringList* names, QStringList* paths);

	/** Load an input template file just enough to get its name */
	static QString loadXMLName(const QString& path);

	/** Load an input template from the given path */
	static QLCInputDevice* load(QObject* parent, const QString& path);

	/** Save an input template into the given file path */
	bool saveXML(const QString& path);

protected:
	/** Load an input template from the given document */
	bool loadXML(QDomDocument* doc);
};

#endif
