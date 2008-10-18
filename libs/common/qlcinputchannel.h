/*
  Q Light Controller
  qlcinputchannel.h
  
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

#ifndef QLCINPUTCHANNEL_H
#define QLCINPUTCHANNEL_H

#include <QObject>

#include <common/qlctypes.h>

class QLCInputDevice;
class QDomDocument;
class QDomElement;
class QString;

#define KXMLQLCInputChannel "Channel"
#define KXMLQLCInputChannelName "Name"
#define KXMLQLCInputChannelType "Type"
#define KXMLQLCInputChannelNumber "Number"
#define KXMLQLCInputChannelAbsoluteFader "AbsoluteFader"
#define KXMLQLCInputChannelRelativeFader "RelativeFader"
#define KXMLQLCInputChannelButton "Button"

class QLC_DECLSPEC QLCInputChannel : public QObject
{
	Q_OBJECT

	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	QLCInputChannel(QLCInputDevice* parent);
	QLCInputChannel(const QLCInputChannel& channel);
	virtual ~QLCInputChannel();

	QLCInputChannel& operator=(QLCInputChannel& channel);
	
	/********************************************************************
	 * Channel number
	 ********************************************************************/
public:
	void setChannel(t_input_channel channel);
	t_input_channel channel() const { return m_channel; }

protected:
	t_input_channel m_channel;

	/********************************************************************
	 * Type
	 ********************************************************************/
public:
	enum Type
	{
		AbsoluteFader,
		RelativeFader,
		Button
	};

	void setType(Type type);
	Type type() const { return m_type; }

	static QString typeToString(Type type);
	static Type stringToType(const QString& type);
	
protected:
	Type m_type;

	/********************************************************************
	 * Name
	 ********************************************************************/
public:
	void setName(const QString& name);
	QString name() const { return m_name; }
	
protected:
	QString m_name;
	
	/********************************************************************
	* Load & Save
	********************************************************************/
public:
	bool loadXML(QDomDocument* doc, QDomElement* root);
	bool saveXML(QDomDocument* doc, QDomElement* root) const;
};

#endif
