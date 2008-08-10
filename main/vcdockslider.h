/*
  Q Light Controller
  vcdockslider.h

  Copyright (c) Heikki Junnila, Stefan Krumm

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

#ifndef VCDOCKSLIDER_H
#define VCDOCKSLIDER_H

#include <QWidget>
#include <QTime>

#include "common/qlctypes.h"
#include "ui_vcdockslider.h"

class QDomDocument;
class QDomElement;
class QMouseEvent;
class QPoint;
class QFile;

#define KXMLQLCVCDockSlider "Slider"
#define KXMLQLCVCDockSliderBus "Bus"
#define KXMLQLCVCDockSliderBusLowLimit "LowLimit"
#define KXMLQLCVCDockSliderBusHighLimit "HighLimit"

class VCDockSlider : public QFrame, public Ui_VCDockSlider
{
	Q_OBJECT
    
	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	VCDockSlider(QWidget* parent, t_bus_id bus);
	~VCDockSlider();

	/*********************************************************************
	 * Bus
	 *********************************************************************/
public:
	/**
	 * Set the bus that this slider controls
	 *
	 * @param id The ID of the bus to control
	 */
	void setBusID(t_bus_id id);

	/**
	 * Get the bus that this slider controls
	 *
	 * @return The ID of the bus that this slider controls
	 */
	t_bus_id busID() const { return m_busID; }

	/**
	 * Set the value range that can be set thru this slider
	 *
	 * @param lo The lowest value
	 * @param hi The highest value
	 */
	void setBusRange(t_bus_value lo, t_bus_value hi);

	/**
	 * Get the value range that can be set thru this slider
	 *
	 * @param lo The lowest value
	 * @param hi The highest value
	 */
	void busRange(t_bus_value &lo, t_bus_value &hi);

public slots:
	/**
	 * Slot for bus name changes
	 *
	 * @param id The ID of the bus, whose name has changed
	 * @param name The new name of the bus
	 */
	void slotBusNameChanged(t_bus_id id, const QString& name);

	/**
	 * Slot for bus value changes
	 *
	 * @param id The ID of the bus, whose value has changed
	 * @param value The new value of the bus
	 */
	void slotBusValueChanged(t_bus_id id, t_bus_value value);

protected:
	/** The ID of the controlled bus */
	t_bus_id m_busID;

	/** The lowest value that can be set thru this slider */
	t_bus_value m_busLowLimit;

	/** The highest value that can be set thru this slider */
	t_bus_value m_busHighLimit;

	/** */
	bool m_updateOnly;

	/** Time object to get the elapsed time between tap button clicks */
	QTime m_time;

	/*********************************************************************
	 * User input slots
	 *********************************************************************/
protected slots:
	/**
	 * Slot for slider value changes (user input)
	 *
	 * @param value The slider's value
	 */
	void slotSliderValueChanged(int value);

	/**
	 * Slot for tap button clicks
	 */
	void slotTapButtonClicked();

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	/**
	 * Load this slider's settings
	 *
	 * @param doc An XML document to load from
	 * @param root A VCDockSlider XML root node to load from
	 */
	bool loadXML(QDomDocument* doc, QDomElement* root);

	/**
	 * Save this slider's settings
	 *
	 * @param doc An XML document to save to
	 * @param da_root VCDockArea XML node to save to
	 */
	bool saveXML(QDomDocument* doc, QDomElement* da_root);

};

#endif

