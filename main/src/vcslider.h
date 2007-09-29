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

#include <qvaluelist.h>
#include "vcwidget.h"
#include "common/types.h"

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QSlider;
class QPushButton;
class QDomDocument;
class QDomElement;
class QTime;

#define KXMLQLCVCSlider QString("Slider")
#define KXMLQLCVCSliderMode QString("SliderMode")

#define KXMLQLCVCSliderValueDisplayStyle QString("ValueDisplayStyle")
#define KXMLQLCVCSliderValueDisplayStyleExact QString("Exact")
#define KXMLQLCVCSliderValueDisplayStylePercentage QString("Percentage")

#define KXMLQLCVCSliderBus QString("Bus")
#define KXMLQLCVCSliderBusLowLimit QString("LowLimit")
#define KXMLQLCVCSliderBusHighLimit QString("HighLimit")

#define KXMLQLCVCSliderLevel QString("Level")
#define KXMLQLCVCSliderLevelLowLimit QString("LowLimit")
#define KXMLQLCVCSliderLevelHighLimit QString("HighLimit")

#define KXMLQLCVCSliderChannel QString("Channel")
#define KXMLQLCVCSliderChannelFixture QString("Fixture")

class VCSlider : public VCWidget
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/** Normal constructor */
	VCSlider(QWidget* parent);

	/** Destructor */
	~VCSlider();

	/* Second stage construction */
	void init();

	/** Destroy and delete were already taken, so... */
	void scram();

	/*********************************************************************
	 * Caption
	 *********************************************************************/
public:
	void setCaption(const QString& text);

	/*********************************************************************
	 * Properties
	 *********************************************************************/
public:
	void editProperties();

	/*********************************************************************
	 * QLC Mode
	 *********************************************************************/
public slots:
	void slotQLCModeChanged();

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
	 * Bus
	 *********************************************************************/
public:
	/**
	 * Set the ID of the bus to control (when in Bus mode)
	 *
	 * @param bus A bus id
	 */
	void setBus(t_bus_id bus);

	/**
	 * Get the ID of the controlled bus
	 *
	 */
	t_bus_id bus();

	/**
	 * Set the low limit for bus values set thru the slider
	 */
	void setBusLowLimit(t_bus_value limit);

	/**
	 * Get the low limit for bus values set thru the slider
	 */
	t_bus_value busLowLimit();

	/**
	 * Set the high limit for bus values set thru the slider
	 */
	void setBusHighLimit(t_bus_value limit);

	/**
	 * Get the high limit for bus values set thru the slider
	 */
	t_bus_value busHighLimit();

protected:
	/**
	 * Set the current slider value to the assigned bus
	 */
	void setBusValue(int value);

public slots:
	/**
	 * Callback for bus value changes
	 */
	void slotBusValueChanged(t_bus_id bus, t_bus_value value);

	/**
	 * Callback for bus name changes
	 */
	void slotBusNameChanged(t_bus_id bus, const QString& name);

protected:
	t_bus_id m_bus;
	t_bus_value m_busLowLimit;
	t_bus_value m_busHighLimit;

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
	QValueList<int> levelChannels();

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
	 * Get an iterator to a level channel.
	 *
	 * @param combined A combined integer containing fixture & channel
	 */
	QValueList<int>::iterator levelChannel(int combined);

	/**
	 * Get an iterator to a level channel.
	 *
	 * @param fixture A fixture ID
	 * @param channel A channel from the fixture
	 */
	QValueList<int>::iterator levelChannel(t_fixture_id fixture,
					       t_channel channel);

	/**
	 * Set the level to all channels that have been assigned to
	 * the slider.
	 *
	 * @param value DMX value
	 */
	void setLevelValue(t_value value);

protected:
	QValueList <int> m_levelChannels;
	t_value m_levelLowLimit;
	t_value m_levelHighLimit;

	/*********************************************************************
	 * Layout
	 *********************************************************************/
protected:
	QVBoxLayout* m_vbox;
	QHBoxLayout* m_hbox;

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
	void slotSliderPressed();
	void slotSliderValueChanged(int value);
	void slotSliderReleased();

protected:
	QSlider* m_slider;
	int m_sliderValue;
	bool m_sliderPressed;
	bool m_moveSliderOnly;
	bool m_trackSliderValue;

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
	 * Feedback
	 *********************************************************************/
public:
	void setFeedbackChannel(int channel);
	int feedbackChannel();

protected:
	void sendFeedback(int value);

protected:
	int m_feedbackChannel;

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	static bool loader(QDomDocument* doc, QDomElement* root, QWidget* parent);

	bool loadXML(QDomDocument* doc, QDomElement* root);
	bool loadXMLLevel(QDomDocument* doc, QDomElement* level_root);

	bool saveXML(QDomDocument* doc, QDomElement* vc_root);
};

#endif
