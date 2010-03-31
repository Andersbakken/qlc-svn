/*
  Q Light Controller
  vcslider.h

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

#ifndef VCSLIDER_H
#define VCSLIDER_H

#include <QMutex>
#include <QList>

#include "dmxsource.h"
#include "vcwidget.h"
#include "qlctypes.h"

class QDomDocument;
class QDomElement;
class QPushButton;
class QHBoxLayout;
class QSlider;
class QLabel;
class QTime;

class VCSliderProperties;

#define KXMLQLCVCSlider "Slider"
#define KXMLQLCVCSliderMode "SliderMode"

#define KXMLQLCVCSliderValueDisplayStyle "ValueDisplayStyle"
#define KXMLQLCVCSliderValueDisplayStyleExact "Exact"
#define KXMLQLCVCSliderValueDisplayStylePercentage "Percentage"

#define KXMLQLCVCSliderInvertedAppearance "InvertedAppearance"

#define KXMLQLCVCSliderBus "Bus"
#define KXMLQLCVCSliderBusLowLimit "LowLimit"
#define KXMLQLCVCSliderBusHighLimit "HighLimit"

#define KXMLQLCVCSliderLevel "Level"
#define KXMLQLCVCSliderLevelLowLimit "LowLimit"
#define KXMLQLCVCSliderLevelHighLimit "HighLimit"

#define KXMLQLCVCSliderChannel "Channel"
#define KXMLQLCVCSliderChannelFixture "Fixture"

class VCSlider : public VCWidget, public DMXSource
{
	Q_OBJECT

	friend class VCSliderProperties;

public:
	static const QSize defaultSize;

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/** Normal constructor */
	VCSlider(QWidget* parent);

	/** Destructor */
	~VCSlider();

private:
	/** Prevent copying thru operator= or copy constructor since QObject's
	    parental properties get confused when copied. */
	Q_DISABLE_COPY(VCSlider)

	/*********************************************************************
	 * Clipboard
	 *********************************************************************/
public:
	/** Create a copy of this widget into the given parent */
	VCWidget* createCopy(VCWidget* parent);

protected:
	/** Copy the contents for this widget from another widget */
	bool copyFrom(VCWidget* widget);

	/*********************************************************************
	 * Caption
	 *********************************************************************/
public:
	void setCaption(const QString& text);

	/*********************************************************************
	 * Properties
	 *********************************************************************/
public:
	/** Edit this widget's properties */
	void editProperties();

	/*********************************************************************
	 * QLC Mode
	 *********************************************************************/
public slots:
	void slotModeChanged(Doc::Mode mode);

	/*********************************************************************
	 * Slider Mode
	 *********************************************************************/
public:
	enum SliderMode
	{
		Bus,
		Level,
		Submaster
	};

public:
	/**
	 * Convert a SliderMode enum to a string that can be saved into
	 * an XML file.
	 *
	 * @param mode The mode to convert
	 * @return A string
	 */
	static QString sliderModeToString(SliderMode mode);

	/**
	 * Convert a string into a SliderMode enum.
	 *
	 * @param mode The string to convert
	 * @return SliderMode
	 */
	static SliderMode stringToSliderMode(const QString& mode);

	/**
	 * Get the slider's current SliderMode
	 */
	SliderMode sliderMode();

	/**
	 * Change the slider's current SliderMode
	 */
	void setSliderMode(SliderMode mode);

protected:
	SliderMode m_sliderMode;

	/*********************************************************************
	 * Value display style
	 *********************************************************************/
public:
	enum ValueDisplayStyle
	{
		ExactValue,
		PercentageValue
	};

	static QString valueDisplayStyleToString(ValueDisplayStyle style);
	static ValueDisplayStyle stringToValueDisplayStyle(QString style);

	void setValueDisplayStyle(ValueDisplayStyle style);
	ValueDisplayStyle valueDisplayStyle();

protected:
	ValueDisplayStyle m_valueDisplayStyle;

	/*********************************************************************
	 * Inverted appearance
	 *********************************************************************/
public:
	bool invertedAppearance() const;
	void setInvertedAppearance(bool invert);

	/*********************************************************************
	 * Bus
	 *********************************************************************/
public:
	/**
	 * Set the ID of the bus to control (when in Bus mode)
	 *
	 * @param bus A bus id
	 */
	void setBus(quint32 bus);

	/**
	 * Get the ID of the controlled bus
	 *
	 */
	quint32 bus();

	/**
	 * Set the low limit for bus values set thru the slider
	 */
	void setBusLowLimit(quint32 limit);

	/**
	 * Get the low limit for bus values set thru the slider
	 */
	quint32 busLowLimit();

	/**
	 * Set the high limit for bus values set thru the slider
	 */
	void setBusHighLimit(quint32 limit);

	/**
	 * Get the high limit for bus values set thru the slider
	 */
	quint32 busHighLimit();

protected:
	/**
	 * Set the current slider value to the assigned bus
	 */
	void setBusValue(int value);

public slots:
	/**
	 * Callback for bus value changes
	 */
	void slotBusValueChanged(quint32 bus, quint32 value);

	/**
	 * Callback for bus name changes
	 */
	void slotBusNameChanged(quint32 bus, const QString& name);

protected:
	quint32 m_bus;
	quint32 m_busLowLimit;
	quint32 m_busHighLimit;

	/*********************************************************************
	 * Level channels
	 *********************************************************************/
public:
	/**
	 * Combine a fixture id and channel number into a single int.
	 * The highest 9 bits are used for the channel, while the
	 * lowest 23 bits remain for the fixture id.
	 *
	 * @param fixture A fixture id
	 * @param channel A channel from the fixture
	 * @return An integer containing the both numbers
	 */
	static int combineFixtureAndChannel(t_fixture_id fixture,
					    t_channel channel);

	/**
	 * Split a combined integer into a fixture id and channel number.
	 * 
	 * @param combined The combined integer
	 * @param fixture Fixture ID
	 * @param channel Channel from the fixture
	 * @return Fixture ID
	 */
	static t_fixture_id splitCombinedValue(int combined,
					       t_fixture_id* fixture,
					       t_channel* channel);

	/**
	 * Add a channel from a fixture into the slider's list of
	 * level channels.
	 *
	 * @param fixture Fixture ID
	 * @param channel A channel from the fixture
	 */
	void addLevelChannel(t_fixture_id fixture, t_channel channel);

	/**
	 * Remove a fixture & channel from the slider's list of
	 * level channels.
	 *
	 * @param fixture Fixture ID
	 * @param channel A channel from the fixture
	 */
	void removeLevelChannel(t_fixture_id fixture, t_channel channel);

	/**
	 * Clear the list of level channels
	 *
	 */
	void clearLevelChannels();

	/**
	 * Get the list of channels that this slider controls
	 *
	 */
	QList <int> levelChannels();

	/**
	 * Set low limit for levels set thru the slider
	 *
	 * @param value Low limit
	 */
	void setLevelLowLimit(t_value value);

	/**
	 * Get low limit for levels set thru the slider
	 *
	 */
	t_value levelLowLimit();

	/**
	 * Set high limit for levels set thru the slider
	 *
	 * @param value High limit
	 */
	void setLevelHighLimit(t_value value);

	/**
	 * Get high limit for levels set thru the slider
	 *
	 */
	t_value levelHighLimit();

protected:
	/**
	 * Set the level to all channels that have been assigned to
	 * the slider.
	 *
	 * @param value DMX value
	 */
	void setLevelValue(t_value value);

protected:
	QList <int> m_levelChannels;
	t_value m_levelLowLimit;
	t_value m_levelHighLimit;

	/*********************************************************************
	 * DMXSource
	 *********************************************************************/
public:
	/** @reimpl */
	void writeDMX(MasterTimer* timer, QByteArray* universes);

protected:
	QMutex m_levelValueMutex;
	char m_levelValue;

	/*********************************************************************
	 * Top label
	 *********************************************************************/
public:
	/**
	 * Set the text for the top label
	 */
	void setTopLabelText(const QString& text);

	/**
	 * Get the text in the top label
	 */
	QString topLabelText();

protected:
	QLabel* m_topLabel;

	/*********************************************************************
	 * Slider
	 *********************************************************************/
public:
	/**
	 * Set the current slider value
	 */
	void setSliderValue(int value);

	/**
	 * Get the current slider value
	 */
	int sliderValue();

public slots:
	void slotSliderMoved(int value);

protected:
	QHBoxLayout* m_hbox;
	QSlider* m_slider;

	int m_sliderValue;

	/*********************************************************************
	 * Bottom label
	 *********************************************************************/
public:
	/**
	 * Set the text for the bottom label
	 */
	void setBottomLabelText(const QString& text);

	/**
	 * Get the text in the top label
	 */
	QString bottomLabelText();

protected:
	QLabel* m_bottomLabel;

	/*********************************************************************
	 * Tap button
	 *********************************************************************/
public:
	/**
	 * Set the text for the tap button
	 */
	void setTapButtonText(const QString& text);

	/**
	 * Get the text in the tap button
	 */
	QString tapButtonText();

public slots:
	/**
	 * Callback for tap button clicks
	 */
	void slotTapButtonClicked();

protected:
	QPushButton* m_tapButton;
	QTime* m_time;

	/*********************************************************************
	 * External input
	 *********************************************************************/
protected slots:
	void slotInputValueChanged(t_input_universe universe,
				   t_input_channel channel,
				   t_input_value value);

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	static bool loader(const QDomElement* root, QWidget* parent);
	bool loadXML(const QDomElement* root);
	bool loadXMLLevel(const QDomElement* level_root);

	bool saveXML(QDomDocument* doc, QDomElement* vc_root);
};

#endif
