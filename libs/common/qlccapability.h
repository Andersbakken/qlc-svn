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

/**
 * QLCCapability represents one value range with a special meaning in a
 * QLCChannel. For example, a sunburst gobo might be set on a "gobo" channel
 * with any DMX value between 15 and 25. This is represented as a
 * QLCCapability, whose min == 15, max == 25 and name == "Sunburst". Single
 * values can be represented by setting the same value to both, for example:
 * min == 15 and max == 15.
 */
class QLCCapability
{
	/********************************************************************
	 * Initialization
	 ********************************************************************/
public:
	/** Default constructor */
	QLCCapability(t_value min = KChannelValueMin,
		      t_value max = KChannelValueMax,
		      const QString& name = QString::null);

	/** Copy constructor */
	QLCCapability(const QLCCapability* cap);

	/** Destructor */
	~QLCCapability();

	/** Assignment operator */
	QLCCapability& operator=(const QLCCapability& capability);

	/** Comparing operator for qSort */
	bool operator<(const QLCCapability& capability) const;

	/********************************************************************
	 * Properties
	 ********************************************************************/
public:
	t_value min() const { return m_min; }
	void setMin(t_value value) { m_min = value; }

	t_value max() const { return m_max; }
	void setMax(t_value value) { m_max = value; }

	t_value middle() const { return int((m_max + m_min) / 2); }

	QString name() const { return m_name; }
	void setName(const QString& name) { m_name = name; }

	/** Check, whether the given capability overlaps with this */
	bool overlaps(const QLCCapability& cap);

protected:
	t_value m_min;
	t_value m_max;
	QString m_name;

	/********************************************************************
	 * Load & Save
	 ********************************************************************/
public:
	/** Save the capability to a QDomDocument, under the given element */
	bool saveXML(QDomDocument* doc, QDomElement* root);

	/** Load capability contents from an XML element */
	bool loadXML(const QDomElement* root);
};

#endif
