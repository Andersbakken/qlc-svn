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
#define KXMLQLCVCDockSliderBusLowLimit "Low"
#define KXMLQLCVCDockSliderBusHighLimit "High"
#define KXMLQLCVCDockSliderInput "Input"
#define KXMLQLCVCDockSliderInputUniverse "Universe"
#define KXMLQLCVCDockSliderInputChannel "Channel"

class VCDockSlider : public QFrame, public Ui_VCDockSlider
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	VCDockSlider(QWidget* parent, t_bus_id bus);
	~VCDockSlider();

private:
	Q_DISABLE_COPY(VCDockSlider)

	/*********************************************************************
	 * Bus
	 *********************************************************************/
public:
	/** Set the bus that this slider controls */
	void setBus(t_bus_id bus);

	/** Get the bus that this slider controls */
	t_bus_id bus() const { return m_bus; }

	/** Set bus value low & high limits i.e. range */
	void setBusRange(t_bus_value lo, t_bus_value hi);

	/** Get the slider's low value limit */
	t_bus_value busLowLimit() const { return m_busLowLimit; }

	/** Get the slider's high value limit */
	t_bus_value busHighLimit() const { return m_busHighLimit; }

public slots:
	void slotBusNameChanged(t_bus_id bus, const QString& name);
	void slotBusValueChanged(t_bus_id bus, t_bus_value value);

protected:
	t_bus_id m_bus;
	t_bus_value m_busLowLimit;
	t_bus_value m_busHighLimit;

	/*********************************************************************
	 * User input slots
	 *********************************************************************/
protected slots:
	void slotSliderPressed();
	void slotSliderValueChanged(int value);
	void slotSliderReleased();

protected:
	/** Separates manual user input (true) from internal data (false) */
	bool m_sliderPressed;

	/*********************************************************************
	 * Tap button
	 *********************************************************************/
protected slots:
	void slotTapButtonClicked();

protected:
	/** Time object to get the elapsed time between tap button clicks */
	QTime m_time;

	/*********************************************************************
	 * External input
	 *********************************************************************/
public:
	/** Set the input universe & channel that this slider listens to */
	void setInputSource(t_input_universe uni, t_input_channel ch);

	/** Get the input universe that this slider listens to */
	t_input_universe inputUniverse() const { return m_inputUniverse; }

	/** Get the input channel number that this slider listens to */
	t_input_channel inputChannel() const { return m_inputChannel; }

protected slots:
	/** Slot for external input value change signals */
	void slotInputValueChanged(t_input_universe universe,
				   t_input_channel channel,
				   t_input_value value);

protected:
	t_input_universe m_inputUniverse;
	t_input_channel m_inputChannel;

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	/** Load this slider's settings */
	bool loadXML(QDomDocument* doc, QDomElement* root);

	/** Save this slider's settings */
	bool saveXML(QDomDocument* doc, QDomElement* da_root);

};

#endif

