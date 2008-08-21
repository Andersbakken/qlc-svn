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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QPaintEvent>
#include <QPainter>
#include <iostream>
#include <QString>
#include <QSlider>
#include <QLabel>
#include <QTime>
#include <QSize>
#include <QtXml>
#include <QPen>

#include "vcsliderproperties.h"
#include "virtualconsole.h"
#include "vcslider.h"
#include "inputmap.h"
#include "dmxmap.h"
#include "app.h"
#include "doc.h"

#include "common/qlcfile.h"

using namespace std;

extern App* _app;

static const t_bus_value KDefaultBusLowLimit ( 0 );
static const t_bus_value KDefaultBusHighLimit ( 5 );

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCSlider::VCSlider(QWidget* parent) : VCWidget(parent)
{
	/* Set the class name "VCSlider" as the object name as well */
	setObjectName(VCSlider::staticMetaObject.className());

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

	m_time = NULL;

	setCaption(QString::null);

	/* Main VBox */
	new QVBoxLayout(this);
	layout()->setMargin(0);
	layout()->setSpacing(0);
	
	/* Top label */
	m_topLabel = new QLabel(this);
	layout()->addWidget(m_topLabel);
	m_topLabel->setAlignment(Qt::AlignCenter);

	/* Slider & its HBox */
	m_hbox = new QHBoxLayout();
	layout()->addItem(m_hbox);
	m_hbox->insertSpacing(-1, 10);

	m_slider = new QSlider(this);
	m_hbox->addWidget(m_slider);
	m_slider->setRange(KDefaultBusLowLimit * KFrequency, 
			   KDefaultBusHighLimit * KFrequency);
	m_slider->setPageStep(1);
	m_slider->setInvertedAppearance(true);
	connect(m_slider, SIGNAL(sliderPressed()),
		this, SLOT(slotSliderPressed()));
	connect(m_slider, SIGNAL(valueChanged(int)),
		this, SLOT(slotSliderValueChanged(int)));
	connect(m_slider, SIGNAL(sliderReleased()),
		this, SLOT(slotSliderReleased()));
	
	m_hbox->insertSpacing(-1, 10);

	/* Tap button */
	m_tapButton = new QPushButton(this);
	layout()->addWidget(m_tapButton);
	connect(m_tapButton, SIGNAL(clicked()),
		this, SLOT(slotTapButtonClicked()));
	m_time = new QTime();

	/* Bottom label */
	m_bottomLabel = new QLabel(this);
	layout()->addWidget(m_bottomLabel);
	m_bottomLabel->setAlignment(Qt::AlignCenter);
	m_bottomLabel->hide();

	resize(QPoint(60, 220));

	/* Initialize to bus mode by default */
	setBus(KBusIDDefaultFade);
	setSliderMode(Bus);
	setSliderValue(0);
	slotSliderValueChanged(0);

	/* Update the slider according to current mode */
	slotModeChanged(_app->mode());

	/* External input */
	m_inputUniverse = KInputUniverseInvalid;
	m_inputChannel = KInputChannelInvalid;
}

VCSlider::~VCSlider()
{
	if (m_time != NULL)
		delete m_time;
	m_time = NULL;
}

void VCSlider::slotDelete()
{
	QString msg;

	msg = "Do you wish to delete this slider?\n" + caption();
	int result = QMessageBox::question(this, "Delete", msg,
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

	if (m_bottomLabel != NULL)
		setBottomLabelText(text);

	if (m_tapButton != NULL)
		setTapButtonText(text);
}

void VCSlider::slotRename()
{
	QString text;

	if (m_sliderMode == Bus)
		QMessageBox::information(_app->virtualConsole(),
					 "Cannot rename a bus mode slider",
					 "A slider cannot be renamed when it " \
					 "is used to control a bus;\n" \
					 "The bus' name is used as the " \
					 "slider's name instead.");
	else
		VCWidget::slotRename();
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCSlider::slotProperties()
{
	VCSliderProperties prop(_app, this);
	prop.exec();
}

/*****************************************************************************
 * QLC Mode
 *****************************************************************************/

void VCSlider::slotModeChanged(App::Mode mode)
{
	if (mode == App::Operate)
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

	VCWidget::slotModeChanged(mode);
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
	if (name.simplified().isEmpty() == true)
		name.sprintf("Bus %.2d", bus + 1);

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
}

void VCSlider::slotBusValueChanged(t_bus_id bus, t_bus_value value)
{
	if (bus == m_bus && m_sliderPressed == false)
		setSliderValue(value);
}

void VCSlider::slotBusNameChanged(t_bus_id bus, const QString&)
{
	if (m_bus == bus)
		setBus(bus);
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

	if (m_levelChannels.contains(combined) == false)
	{
		m_levelChannels.append(combined);
		qSort(m_levelChannels.begin(), m_levelChannels.end());
	}
	else
	{
		cout << QString("Fixture %1 and channel %2 already in list")
			.arg(fixture).arg(channel).toStdString()
		     << endl;
	}
}

void VCSlider::removeLevelChannel(t_fixture_id fixture, t_channel channel)
{
	int combined = combineFixtureAndChannel(fixture, channel);

	if (m_levelChannels.removeAll(combined) == 0)
	{
		cout << QString("Fixture %1 and channel %2 not found")
			.arg(fixture).arg(channel).toStdString()
		     << endl;
	}
}

void VCSlider::clearLevelChannels()
{
	m_levelChannels.clear();
}

QList <int> VCSlider::levelChannels()
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

void VCSlider::setLevelValue(t_value value)
{
	Fixture* fxi = NULL;
	t_fixture_id fxi_id = KNoID;
	t_channel ch = 0;
	int dmx_ch = 0;

	QListIterator <int> it(m_levelChannels);
	while (it.hasNext() == true)
	{
		splitCombinedValue(it.next(), &fxi_id, &ch);
		fxi = _app->doc()->fixture(fxi_id);
		if (fxi != NULL)
			dmx_ch = fxi->channelAddress(ch);
		
		_app->dmxMap()->setValue(dmx_ch,
					 m_levelHighLimit - value +
					 m_levelLowLimit);

	}
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
		if (m_sliderPressed == true)
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
	slotSliderPressed();
	setSliderValue(static_cast<int> (t * 0.001 * KFrequency));
	slotSliderReleased();
	m_time->restart();
}

/*****************************************************************************
 * External input
 *****************************************************************************/

void VCSlider::setInputUniverse(t_input_universe uni)
{
	m_inputUniverse = uni;

	disconnect(_app->inputMap(),
		   SIGNAL(inputValueChanged(t_input_universe,
					    t_input_channel,
					    t_input_value)),
		   this, SLOT(slotInputValueChanged(t_input_universe,
						    t_input_channel,
						    t_input_value)));

	if (m_inputUniverse != KInputUniverseInvalid)
	{
		connect(_app->inputMap(),
			SIGNAL(inputValueChanged(t_input_universe,
						 t_input_channel,
						 t_input_value)),
			this,
			SLOT(slotInputValueChanged(t_input_universe,
						   t_input_channel,
						   t_input_value)));
	}
}

void VCSlider::setInputChannel(t_input_channel ch)
{
	m_inputChannel = ch;

	disconnect(_app->inputMap(),
		   SIGNAL(inputValueChanged(t_input_universe,
					    t_input_channel,
					    t_input_value)),
		   this,
		   SLOT(slotInputValueChanged(t_input_universe,
					      t_input_channel,
					      t_input_value)));
	
	if (m_inputChannel != KInputChannelInvalid)
	{
		connect(_app->inputMap(),
			SIGNAL(inputValueChanged(t_input_universe,
						 t_input_channel,
						 t_input_value)),
			this,
			SLOT(slotInputValueChanged(t_input_universe,
						   t_input_channel,
						   t_input_value)));
	}
}

void VCSlider::slotInputValueChanged(t_input_universe universe,
				     t_input_channel channel,
				     t_input_value value)
{
	qDebug() << universe << channel << value;
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

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCVCSlider)
	{
		cout << "Slider node not found!" << endl;
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
			QLCFile::loadXMLWindowState(&tag, &x, &y, &w, &h,
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

			setBus(tag.text().toInt());
		}
		else if (tag.tagName() == KXMLQLCVCSliderLevel)
		{
			loadXMLLevel(doc, &tag);
		}
		else if (tag.tagName() == KXMLQLCVCSliderInputUniverse)
		{
			setInputUniverse(tag.text().toInt());
		}
		else if (tag.tagName() == KXMLQLCVCSliderInputChannel)
		{
			setInputChannel(tag.text().toInt());
		}
		else
		{
			cout << "Unknown slider tag: "
			     << tag.tagName().toStdString()
			     << endl;
		}
		
		node = node.nextSibling();
	}

	/* Set the mode last, after everything else has been set */
	setSliderMode(sliderMode);

	return true;
}

bool VCSlider::loadXMLLevel(QDomDocument*, QDomElement* level_root)
{
	QDomNode node;
	QDomElement tag;
	QString str;

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
			cout << "Unknown slider level tag: "
			     << tag.tagName().toStdString()
			     << endl;
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
	QLCFile::saveXMLWindowState(doc, &root, this);

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

	/* External input universe */
	tag = doc->createElement(KXMLQLCVCSliderInputUniverse);
	root.appendChild(tag);
	text = doc->createTextNode(str.setNum(m_inputUniverse));
	tag.appendChild(text);

	/* External input channel */
	tag = doc->createElement(KXMLQLCVCSliderInputChannel);
	root.appendChild(tag);
	text = doc->createTextNode(str.setNum(m_inputChannel));
	tag.appendChild(text);

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
	QListIterator <int> it(m_levelChannels);
	while (it.hasNext() == true)
	{
		splitCombinedValue(it.next(), &fxi_id, &ch);

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

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void VCSlider::paintEvent(QPaintEvent* e)
{
	// First paint whatever QFrame wants to
	QFrame::paintEvent(e);

	QPainter painter(this);

	/* This paint event handler must be here because VCWidget would draw
	   the caption into its background, which is not what we want for
	   a slider */

	/* Draw selection frame */
	if (_app->mode() == App::Design &&
	    _app->virtualConsole()->selectedWidget() == this)
	{
		// Draw a dotted line around the widget
		QPen pen(Qt::DotLine);
		pen.setWidth(2);
		painter.setPen(pen);
		painter.drawRect(1, 1, rect().width() - 1, rect().height() - 1);

		// Draw a resize handle
		QBrush b(Qt::SolidPattern);
		painter.fillRect(rect().width() - 10,
				 rect().height() - 10, 10, 10, b);
	}
}
