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

class QLC_DECLSPEC QLCInputChannel : public QObject
{
	Q_OBJECT

	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	QLCInputChannel(QLCInputDevice* parent);
	virtual ~QLCInputChannel();

	QLCInputChannel& operator=(QLCInputChannel& channel);
	
	/********************************************************************
	 * Type
	 ********************************************************************/
public:
	enum Type
	{
		AbsoluteFader,
		Button
	};

	void setType(Type type);
	Type type() const { return m_type; }
	
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
	void loadXML(QDomDocument* doc, QDomElement* root);
	void saveXML(QDomDocument* doc, QDomElement* dev_root);
};

#endif
