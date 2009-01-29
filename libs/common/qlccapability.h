/*
  Q Light Controller
  qlccapability.h

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

#ifndef QLCCAPABILITY_H
#define QLCCAPABILITY_H

#include <QList>

#include "qlctypes.h"

#define KXMLQLCCapability    "Capability"
#define KXMLQLCCapabilityMin "Min"
#define KXMLQLCCapabilityMax "Max"

class QString;
class QFile;
class QDomDocument;
class QDomElement;
class QLCCapability;

class QLC_DECLSPEC QLCCapability
{
public:
	/** Default constructor */
	QLCCapability(t_value min = KChannelValueMin,
		      t_value max = KChannelValueMax,
		      QString name = QString::null);

	/** Copy constructor */
	QLCCapability(QLCCapability* cap);

	/** Create contents from an XML tag */
	QLCCapability(QDomElement* tag);

	/** Destructor */
	virtual ~QLCCapability();

	/** Assignment operator */
	QLCCapability& operator=(QLCCapability& capability);

	t_value min() { return m_min; }
	t_value max() { return m_max; }
	QString name() { return m_name; }

	void setMin(const t_value &value) { m_min = value; }
	void setMax(const t_value &value) { m_max = value; }
	void setName(const QString &name) { m_name = name; }

	/** Save the capability to a QDomDocument, under the given element */
	int saveXML(QDomDocument* doc, QDomElement* root);

	/** Load capability contents from an XML element */
	virtual bool loadXML(QDomElement* root);

protected:
	t_value m_min;
	t_value m_max;
	QString m_name;
};

#endif
