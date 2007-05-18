/*
  Q Light Controller
  qlcphysical.h
  
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

#ifndef QLC_PHYSICAL_H
#define QLC_PHYSICAL_H

class QString;
class QDomElement;
class QDomDocument;

#define KXMLQLCPhysical "Physical"

#define KXMLQLCPhysicalBulb "Bulb"
#define KXMLQLCPhysicalBulbType "Type"
#define KXMLQLCPhysicalBulbLumens "Lumens"
#define KXMLQLCPhysicalBulbColourTemperature "ColourTemperature"

#define KXMLQLCPhysicalDimensions "Dimensions"
#define KXMLQLCPhysicalDimensionsWeight "Weight"
#define KXMLQLCPhysicalDimensionsWidth "Width"
#define KXMLQLCPhysicalDimensionsHeight "Height"
#define KXMLQLCPhysicalDimensionsDepth "Depth"

#define KXMLQLCPhysicalLens "Lens"
#define KXMLQLCPhysicalLensName "Name"
#define KXMLQLCPhysicalLensDegreesMin "DegreesMin"
#define KXMLQLCPhysicalLensDegreesMax "DegreesMax"

#define KXMLQLCPhysicalFocus "Focus"
#define KXMLQLCPhysicalFocusType "Type"
#define KXMLQLCPhysicalFocusPanMax "PanMax"
#define KXMLQLCPhysicalFocusTiltMax "TiltMax"

class QLCPhysical
{
public:
	/** Assignment operator */
	QLCPhysical& operator=(const QLCPhysical& physical);

public:
	void setBulbType(const QString& type) { m_bulbType = type; }
	QString bulbType() const { return m_bulbType; }
	
	void setBulbLumens(const int lumens) { m_bulbLumens = lumens; }
	int bulbLumens() const { return m_bulbLumens; }
	
	void setBulbColourTemperature(const int temp) { m_bulbColourTemperature = temp; }
	int bulbColourTemperature() const { return m_bulbColourTemperature; }
	
	void setWeight(const int weight) { m_weight = weight; }
	int weight() const { return m_weight; }

	void setWidth(const int width) { m_width = width; }
	int width() const { return m_width; }

	void setHeight(const int height) { m_height = height; }
	int height() const { return m_height; }

	void setDepth(const int depth) { m_depth = depth; }
	int depth() const { return m_depth; }
	
	void setLensName(const QString& name) { m_lensName = name; }
	QString lensName() const { return m_lensName; }
	
	void setLensDegreesMin(const int degrees) { m_lensDegreesMin = degrees; }
	int lensDegreesMin() const { return m_lensDegreesMin; }

	void setLensDegreesMax(const int degrees) { m_lensDegreesMax = degrees; }
	int lensDegreesMax() const { return m_lensDegreesMax; }

	void setFocusType(const QString& type) { m_focusType = type; }
	QString focusType() const { return m_focusType; }
	
	void setFocusPanMax(const int pan) { m_focusPanMax = pan; }
	int focusPanMax() const { return m_focusPanMax; }

	void setFocusTiltMax(const int tilt) { m_focusTiltMax = tilt; }
	int focusTiltMax() const { return m_focusTiltMax; }

	bool loadXML(QDomElement* root);
	bool saveXML(QDomDocument* doc, QDomElement* root);
	
private:
	QString m_bulbType;
	int m_bulbLumens;
	int m_bulbColourTemperature;
 
	int m_weight;
	int m_width;
	int m_height;
	int m_depth;
 
	QString m_lensName;
	int m_lensDegreesMin;
	int m_lensDegreesMax;
 
	QString m_focusType;
	int m_focusPanMax;
	int m_focusTiltMax;
};

#endif
