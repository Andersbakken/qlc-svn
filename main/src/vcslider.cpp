/*
  Q Light Controller
  vcslider.cpp

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

#include <qstring.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qslider.h>
#include <qpushbutton.h>
#include <qdatetime.h>
#include <qmessagebox.h>
#include <qdatetime.h>
#include <qdom.h>
#include <math.h>

#include "app.h"
#include "doc.h"
#include "dmxmap.h"
#include "vcslider.h"
#include "vcsliderproperties.h"
#include "virtualconsole.h"

#include "common/filehandler.h"

extern App* _app;

static const t_bus_value KDefaultBusLowLimit ( 0 );
static const t_bus_value KDefaultBusHighLimit ( 5 );

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCSlider::VCSlider(QWidget* parent) : VCWidget(parent, "Slider")
{
	m_vbox = NULL;
	m_hbox = NULL;
	m_topLabel = NULL;
	m_slider = NULL;
	m_bottomLabel = NULL;
	m_tapButton = NULL;

	m_sliderMode = Bus;
	m_valueDisplayStyle = ExactValue;

	m_levelLowLimit = 0;
	m_levelHighLimit = 255;

	m_bus = -1;
	m_busLowLimit = KDefaultBusLowLimit;
	m_busHighLimit = KDefaultBusHighLimit;

	m_sliderValue = 0;
	m_sliderPressed = false;
	m_moveSliderOnly = false;
	m_trackSliderValue = true;

	m_time = NULL;

	m_feedbackChannel = -1;
}

VCSlider::~VCSlider()
{
	if (m_time != NULL)
		delete m_time;
	m_time = NULL;
}

void VCSlider::init()
{
	setCaption("Slider");

	/* Main VBox */
	m_vbox = new QVBoxLayout(this);
	m_vbox->setMargin(10);
	m_vbox->setSpacing(10);
	
	/* Top label */
	m_topLabel = new QLabel(this);
	m_vbox->addWidget(m_topLabel);
	m_topLabel->setAlignment(AlignCenter);

	/* Slider & its HBox */
	m_hbox = new QHBoxLayout(m_vbox);
	m_hbox->insertSpacing(-1, 10);

	m_slider = new QSlider(this);
	m_hbox->addWidget(m_slider);
	m_slider->setRange(KDefaultBusLowLimit * KFrequency, 
			   KDefaultBusHighLimit * KFrequency);
	m_slider->setPageStep(1);
	connect(m_slider, SIGNAL(sliderPressed()),
		this, SLOT(slotSliderPressed()));
	connect(m_slider, SIGNAL(valueChanged(int)),
		this, SLOT(slotSliderValueChanged(int)));
	connect(m_slider, SIGNAL(sliderReleased()),
		this, SLOT(slotSliderReleased()));
	
	m_hbox->insertSpacing(-1, 10);

	/* Tap button */
	m_tapButton = new QPushButton(this);
	m_vbox->addWidget(m_tapButton);
	connect(m_tapButton, SIGNAL(clicked()),
		this, SLOT(slotTapButtonClicked()));
	m_time = new QTime();

	/* Bottom label */
	m_bottomLabel = new QLabel(this);
	m_vbox->addWidget(m_bottomLabel);
	m_bottomLabel->setAlignment(AlignCenter);
	m_bottomLabel->hide();

	resize(QPoint(60, 220));

	/* Initialize to bus mode by default */
	setBus(KBusIDDefaultFade);
	setSliderMode(Bus);
	setSliderValue(0);
	slotSliderValueChanged(0);

	/* Catch QLC mode changes*/
	connect(_app, SIGNAL(modeChanged()), this, SLOT(slotQLCModeChanged()));

	/* Update the slider according to current mode */
	slotQLCModeChanged();
}

void VCSlider::scram()
{
	int result = QMessageBox::warning(this,
					  QString(caption()),
					  QString("Remove selected slider?"),
					  QMessageBox::Yes,
					  QMessageBox::No);

	if (result == QMessageBox::Yes)
	{
		_app->virtualConsole()->setSelectedWidget(NULL);
		_app->doc()->setModified();
		deleteLater();
	}
}

/*****************************************************************************
 * Caption
 *****************************************************************************/

void VCSlider::setCaption(const QString& text)
{
	VCWidget::setCaption(text);

	/* Bottom label has not been created yet during VCWidget
	   construction, so this would crash without this check */
	if (m_bottomLabel != NULL)
		setBottomLabelText(text);
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCSlider::editProperties()
{
	VCSliderProperties* prop = NULL;

	prop = new VCSliderProperties(_app, this);
	prop->init();
	prop->exec();

	delete prop;
}

/*****************************************************************************
 * QLC Mode
 *****************************************************************************/

void VCSlider::slotQLCModeChanged()
{
	if (_app->mode() == App::Operate)
	{
		m_topLabel->setEnabled(true);
		m_slider->setEnabled(true);
		m_bottomLabel->setEnabled(true);
		m_tapButton->setEnabled(true);
	}
	else
	{
		m_topLabel->setEnabled(false);
		m_slider->setEnabled(false);
		m_bottomLabel->setEnabled(false);
		m_tapButton->setEnabled(false);
	}
}

/*****************************************************************************
 * Value display style
 *****************************************************************************/

QString VCSlider::valueDisplayStyleToString(VCSlider::ValueDisplayStyle style)
{
	switch (style)
	{
	case ExactValue:
		return KXMLQLCVCSliderValueDisplayStyleExact;
	case PercentageValue:
		return KXMLQLCVCSliderValueDisplayStylePercentage;
	default:
		return QString("Unknown");
	};
}

VCSlider::ValueDisplayStyle VCSlider::stringToValueDisplayStyle(QString style)
{
	if (style == KXMLQLCVCSliderValueDisplayStyleExact)
		return ExactValue;
	else if (style == KXMLQLCVCSliderValueDisplayStylePercentage)
		return PercentageValue;
	else
		return ExactValue;
}

void VCSlider::setValueDisplayStyle(VCSlider::ValueDisplayStyle style)
{
	m_valueDisplayStyle = style;
}

VCSlider::ValueDisplayStyle VCSlider::valueDisplayStyle()
{
	return m_valueDisplayStyle;
}

/*****************************************************************************
 * Slider Mode
 *****************************************************************************/

QString VCSlider::sliderModeToString(SliderMode mode)
{
	switch (mode)
	{
	case Bus:
		return QString("Bus");
		break;

	case Level:
		return QString("Level");
		break;

	case Submaster:
		return QString("Submaster");
		break;

	default:
		return QString("Unknown");
		break;
	}
}

VCSlider::SliderMode VCSlider::stringToSliderMode(const QString& mode)
{
	if (mode == QString("Bus"))
		return Bus;
	else if (mode == QString("Level"))
		return Level;
	else if (mode == QString("Submaster"))
		return Submaster;
	else
		return Bus;
}

VCSlider::SliderMode VCSlider::sliderMode()
{
	return m_sliderMode;
}

void VCSlider::setSliderMode(SliderMode mode)
{
	Q_ASSERT(mode >= Bus && mode <= Submaster);

	/* Disconnect these to prevent double callbacks and non-essential
	   signals (with Level & Submaster modes) */
	disconnect(Bus::emitter(), SIGNAL(nameChanged(t_bus_id, const QString&)),
		   this, SLOT(slotBusNameChanged(t_bus_id, const QString&)));	
	disconnect(Bus::emitter(), SIGNAL(valueChanged(t_bus_id, t_bus_value)),
		   this, SLOT(slotBusValueChanged(t_bus_id, t_bus_value)));

	m_sliderMode = mode;

	if (mode == Bus)
	{
		/* Set the slider range */
		m_slider->setRange(busLowLimit() * KFrequency, 
				   busHighLimit() * KFrequency);

		/* Reconnect to bus emitter */
		connect(Bus::emitter(), SIGNAL(nameChanged(t_bus_id, const QString&)),
			this, SLOT(slotBusNameChanged(t_bus_id, const QString&)));
		connect(Bus::emitter(), SIGNAL(valueChanged(t_bus_id, t_bus_value)),
			this, SLOT(slotBusValueChanged(t_bus_id, t_bus_value)));

		m_bottomLabel->hide();
		m_tapButton->show();

		m_time->start();
	}
	else if (mode == Level)
	{
		/* Set the slider range */
		m_slider->setRange(levelLowLimit(), levelHighLimit());

		m_bottomLabel->show();
		m_tapButton->hide();
	}
	else if (mode == Submaster)
	{
		m_bottomLabel->show();
		m_tapButton->hide();
	}
}

/*****************************************************************************
 * Bus
 *****************************************************************************/

void VCSlider::setBus(t_bus_id bus)
{
	QString name;

	m_bus = bus;

	name = Bus::name(bus);
	if (name == QString::null)
		name.sprintf("%.2d", bus + 1);

	setCaption(name);
}

t_bus_id VCSlider::bus()
{
	return m_bus;
}

void VCSlider::setBusLowLimit(t_bus_value limit)
{
	m_busLowLimit = limit;
}

t_bus_value VCSlider::busLowLimit()
{
	return m_busLowLimit;
}

void VCSlider::setBusHighLimit(t_bus_value limit)
{
	m_busHighLimit = limit;
}

t_bus_value VCSlider::busHighLimit()
{
	return m_busHighLimit;
}

void VCSlider::setBusValue(int value)
{
	if (Bus::setValue(m_bus, value) == false)
		setTopLabelText("No Bus");

	sendFeedback(value);
}

void VCSlider::slotBusValueChanged(t_bus_id bus, t_bus_value value)
{
	if (bus == m_bus && m_sliderPressed == false)
		setSliderValue(value);
}

void VCSlider::slotBusNameChanged(t_bus_id bus, const QString& name)
{
	if (m_bus == bus)
		setTapButtonText(name);
}

/*****************************************************************************
 * Level
 *****************************************************************************/

int VCSlider::combineFixtureAndChannel(t_fixture_id fixture, t_channel channel)
{
	int combined = 0;

	/* Combine fixture id and channel to a single int. The channel
	   number takes the highest 9 bits, while the lowest 23 bits are
	   reserved for the fixture id */
	combined = channel;
	combined = (combined << 23) | fixture;

	return combined;
}

t_fixture_id VCSlider::splitCombinedValue(int combined,
					  t_fixture_id* fixture,
					  t_channel* channel)
{
	/* Split a combined value into fixture id and channel number */
	*channel = combined >> 23;
	*fixture = combined & 0x007FFFFF;
	
	return *fixture;
}

void VCSlider::addLevelChannel(t_fixture_id fixture, t_channel channel)
{
	int combined = combineFixtureAndChannel(fixture, channel);

	if (levelChannel(combined) == m_levelChannels.end())
	{
		m_levelChannels.append(combined);
		qHeapSort(m_levelChannels);
	}
	else
	{
		qWarning("Fixture %d and channel %d already in list",
			 fixture, channel);
	}
}

void VCSlider::removeLevelChannel(t_fixture_id fixture, t_channel channel)
{
	int combined = combineFixtureAndChannel(fixture, channel);

	if (m_levelChannels.remove(combined) == 0)
	{
		qWarning("Fixture %d and channel %d not found",
			 fixture, channel);
	}
}

void VCSlider::clearLevelChannels()
{
	m_levelChannels.clear();
}

QValueList<int> VCSlider::levelChannels()
{
	return m_levelChannels;
}

void VCSlider::setLevelLowLimit(t_value value)
{
	m_levelLowLimit = value;
}

t_value VCSlider::levelLowLimit()
{
	return m_levelLowLimit;
}

void VCSlider::setLevelHighLimit(t_value value)
{
	m_levelHighLimit = value;
}

t_value VCSlider::levelHighLimit()
{
	return m_levelHighLimit;
}

QValueList<int>::iterator VCSlider::levelChannel(int combined)
{
	return qFind(m_levelChannels.begin(), m_levelChannels.end(), combined);
}

QValueList<int>::iterator VCSlider::levelChannel(t_fixture_id fixture,
						 t_channel channel)
{
	int combined = combineFixtureAndChannel(fixture, channel);
	return levelChannel(combined);
}

void VCSlider::setLevelValue(t_value value)
{
	Fixture* fxi = NULL;
	t_fixture_id fxi_id = KNoID;
	t_channel ch = 0;
	int dmx_ch = 0;

	QValueList<int>::iterator it;
	for (it = m_levelChannels.begin(); it != m_levelChannels.end(); ++it)
	{
		splitCombinedValue(*it, &fxi_id, &ch);
		fxi = _app->doc()->fixture(fxi_id);
		if (fxi != NULL)
			dmx_ch = fxi->channelAddress(ch);
		
		_app->dmxMap()->setValue(dmx_ch,
					 m_levelHighLimit - value +
					 m_levelLowLimit);

	}

	// TODO
	// _app->inputPlugin()->feedBack(1, m_inputChannel, 
	//		      127 - (value * 127) / 255);
}

/*****************************************************************************
 * Top label
 *****************************************************************************/

void VCSlider::setTopLabelText(const QString& text)
{
	m_topLabel->setText(text);
}

QString VCSlider::topLabelText()
{
	return m_topLabel->text();
}

/*****************************************************************************
 * Slider
 *****************************************************************************/

void VCSlider::setSliderValue(int value)
{
	m_sliderValue = value;
	m_slider->setValue(value);
}

int VCSlider::sliderValue()
{
	return m_sliderValue;
}

void VCSlider::slotSliderPressed()
{
	m_sliderPressed = true;
}

void VCSlider::slotSliderValueChanged(int value)
{
	QString num;

	m_sliderValue = value;

	switch(sliderMode())
	{
	case Bus:
	{
		/* Set bus value only if this callback is a result of
		   user dragging the slider and slider tracking is on */
		if (m_sliderPressed == true && m_trackSliderValue == true)
			setBusValue(value);
		
		/* Set text for the top label */
		if (valueDisplayStyle() == ExactValue)
			num.sprintf("%.2fs", ((float) value / (float) KFrequency));
		else
			num.sprintf("%.3d%%", static_cast<int> 
				    (((float) ((m_busHighLimit * KFrequency) - value) / (float) ((m_busHighLimit - m_busLowLimit) * (float) KFrequency)) * 100.0));

		setTopLabelText(num);
	}
	break;
	
	case Level:
	{
		/* Set level value only if slider tracking is on */
		if (m_trackSliderValue == true)
			setLevelValue(value);
		
		/* Set text for the top label */
		if (valueDisplayStyle() == ExactValue)
			num.sprintf("%.3d", 
				    m_levelHighLimit - value + m_levelLowLimit);
		else
			num.sprintf("%.3d%%", static_cast<int>
				    ((((float) (m_levelHighLimit - value)) / (float) (m_levelHighLimit - m_levelLowLimit)) * 100.0));
		setTopLabelText(num);
	}
	break;
	
	case Submaster:
		break;
		
	default:
		break;
	}
}

void VCSlider::slotSliderReleased()
{
	m_sliderPressed = false;

	/* If slider tracking is off, set the value only when the
	   slider is released. */
	if (m_trackSliderValue == false)
		setLevelValue(sliderValue());
}

/*****************************************************************************
 * Bottom label
 *****************************************************************************/
void VCSlider::setBottomLabelText(const QString& text)
{
	m_bottomLabel->setText(text);
}

QString VCSlider::bottomLabelText()
{
	return m_bottomLabel->text();
}

/*****************************************************************************
 * Tap button
 *****************************************************************************/

void VCSlider::setTapButtonText(const QString& text)
{
	m_tapButton->setText(text);
}

QString VCSlider::tapButtonText()
{
	return m_tapButton->text();
}

void VCSlider::slotTapButtonClicked()
{
	int t = m_time->elapsed();
	setSliderValue(static_cast<int> (t * 0.001 * KFrequency));
	m_time->restart();
}

/*****************************************************************************
 * Feedback
 *****************************************************************************/

void VCSlider::setFeedbackChannel(int channel)
{
	m_feedbackChannel = channel;
}

int VCSlider::feedbackChannel()
{
	return m_feedbackChannel;
}

void VCSlider::sendFeedback(int value)
{
	/* TODO
	  int range = 0;
	  float f = 0;
	  
	  range = m_busHighLimit - m_busLowLimit;
	  f = ((float) value / (float) KFrequency);
	  
	  _app->inputPlugin()->feedBack(1, feedbackChannel(),
	  127 - int((f * 127) / range));
	*/
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCSlider::loader(QDomDocument* doc, QDomElement* root, QWidget* parent)
{
	VCSlider* slider = NULL;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);
	Q_ASSERT(parent != NULL);

	if (root->tagName() != KXMLQLCVCSlider)
	{
		qWarning("Slider node not found!");
		return false;
	}

	/* Create a new slider into its parent */
	slider = new VCSlider(parent);
	slider->init();
	slider->show();

	/* Continue loading */
	return slider->loadXML(doc, root);
}

bool VCSlider::loadXML(QDomDocument* doc, QDomElement* root)
{
	bool visible = false;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	
	QDomNode node;
	QDomElement tag;
	QString str;
	SliderMode sliderMode = Bus;
	ValueDisplayStyle valueDisplayStyle = ExactValue;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCVCSlider)
	{
		qWarning("Slider node not found!");
		return false;
	}

	/* Caption */
	setCaption(root->attribute(KXMLQLCVCCaption));

	/* Children */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCWindowState)
		{
			FileHandler::loadXMLWindowState(&tag, &x, &y, &w, &h,
							&visible);
			setGeometry(x, y, w, h);
		}
		else if (tag.tagName() == KXMLQLCVCAppearance)
		{
			loadXMLAppearance(doc, &tag);
		}
		else if (tag.tagName() == KXMLQLCVCSliderMode)
		{
			sliderMode = stringToSliderMode(tag.text());

			str = tag.attribute(KXMLQLCVCSliderValueDisplayStyle);
			setValueDisplayStyle(stringToValueDisplayStyle(str));
		}
		else if (tag.tagName() == KXMLQLCVCSliderBus)
		{
			str = tag.attribute(KXMLQLCVCSliderBusLowLimit);
			setBusLowLimit(str.toInt());

			str = tag.attribute(KXMLQLCVCSliderBusHighLimit);
			setBusHighLimit(str.toInt());
		}
		else if (tag.tagName() == KXMLQLCVCSliderLevel)
		{
			loadXMLLevel(doc, &tag);
		}
		else
		{
			qWarning("Unknown slider tag: %s",
				 (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}

	/* Set the mode last, after everything else has been set */
	setSliderMode(sliderMode);

	return true;
}

bool VCSlider::loadXMLLevel(QDomDocument* doc, QDomElement* level_root)
{
	QDomNode node;
	QDomElement tag;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(level_root != NULL);

	if (level_root->tagName() != KXMLQLCVCSliderLevel)
	{
		qWarning("Slider level node not found!");
		return false;
	}

	/* Level low limit */
	str = level_root->attribute(KXMLQLCVCSliderLevelLowLimit);
	setLevelLowLimit(str.toInt());

	/* Level high limit */
	str = level_root->attribute(KXMLQLCVCSliderLevelHighLimit);
	setLevelHighLimit(str.toInt());

	/* Children */
	node = level_root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCVCSliderChannel)
		{
			/* Fixture & channel */
			str = tag.attribute(KXMLQLCVCSliderChannelFixture);
			addLevelChannel(
				static_cast<t_fixture_id>(str.toInt()),
				static_cast<t_channel> (tag.text().toInt()));
		}
		else
		{
			qWarning("Unknown slider level tag: %s",
				 (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}

	return true;
}

bool VCSlider::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
	QDomElement root;
	QDomElement tag;
	QDomElement chtag;
	QDomText text;
	QString str;
	QValueList<int>::iterator it;
	t_fixture_id fxi_id = KNoID;
	t_channel ch = 0;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(vc_root != NULL);

	/* VC Slider entry */
	root = doc->createElement(KXMLQLCVCSlider);
	vc_root->appendChild(root);

	/* Caption */
	root.setAttribute(KXMLQLCVCCaption, caption());

	/* Window state */
	FileHandler::saveXMLWindowState(doc, &root, this);

	/* Appearance */
	saveXMLAppearance(doc, &root);

	/* Mode */
	tag = doc->createElement(KXMLQLCVCSliderMode);
	root.appendChild(tag);
	text = doc->createTextNode(sliderModeToString(m_sliderMode));
	tag.appendChild(text);

	/* Value display style */
	str = valueDisplayStyleToString(valueDisplayStyle());
	tag.setAttribute(KXMLQLCVCSliderValueDisplayStyle, str);

	/* Bus */
	tag = doc->createElement(KXMLQLCVCSliderBus);
	root.appendChild(tag);
	str.setNum(bus());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Bus low limit */
	str.setNum(busLowLimit());
	tag.setAttribute(KXMLQLCVCSliderBusLowLimit, str);

	/* Bus high limit */
	str.setNum(busHighLimit());
	tag.setAttribute(KXMLQLCVCSliderBusHighLimit, str);

	/* Level */
	tag = doc->createElement(KXMLQLCVCSliderLevel);
	root.appendChild(tag);

	/* Level low limit */
	str.setNum(levelLowLimit());
	tag.setAttribute(KXMLQLCVCSliderLevelLowLimit, str);

	/* Level high limit */
	str.setNum(levelHighLimit());
	tag.setAttribute(KXMLQLCVCSliderLevelHighLimit, str);

	/* Level channels */
	for (it = m_levelChannels.begin(); it != m_levelChannels.end(); ++it)
	{
		splitCombinedValue(*it, &fxi_id, &ch);

		chtag = doc->createElement(KXMLQLCVCSliderChannel);
		tag.appendChild(chtag);

		str.setNum(fxi_id);
		chtag.setAttribute(KXMLQLCVCSliderChannelFixture, str);

		str.setNum(ch);
		text = doc->createTextNode(str);
		chtag.appendChild(text);
	}

	return true;
}
