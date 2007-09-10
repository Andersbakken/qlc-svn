/*
  Q Light Controller
  vcdockslider.cpp

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

#include <qslider.h>
#include <qlabel.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qevent.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qfontdialog.h>
#include <qpixmap.h>
#include <qcolor.h>
#include <qcolordialog.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qinputdialog.h>
#include <assert.h>

#include "vcdockslider.h"
#include "vcdocksliderproperties.h"
#include "vcframe.h"
#include "virtualconsole.h"
#include "sliderkeybind.h"
#include "assignsliderhotkey.h"
#include "bus.h"
#include "app.h"
#include "doc.h"
#include "functionconsumer.h"
#include "common/settings.h"
#include "configkeys.h"

#include "common/filehandler.h"
#include "common/outputplugin.h"
#include "common/inputplugin.h"
#include "common/minmax.h"
#include "common/qlcimagepreview.h"

extern App* _app;

VCDockSlider::VCDockSlider(QWidget* parent, bool isStatic)
	: UI_VCDockSlider(parent, "VCDockSlider"),
	  m_mode ( Speed ),
	  m_busID ( KBusIDDefaultFade ),
	  m_hasCustomForegroundColor ( false ),
	  m_hasCustomBackgroundColor ( false ),
	  m_hasCustomFont ( false ),
	  m_busLowLimit ( 0 ),
	  m_busHighLimit ( 5 ),
	  m_levelLowLimit ( 0 ),
	  m_levelHighLimit ( 255 ),
	  m_static ( isStatic ),
	  m_updateOnly ( false ),
	  m_sliderKeyBind( NULL )
{
}


VCDockSlider::~VCDockSlider()
{
	//
	// If mode is submaster, resign couplings before delete
	//
	if (m_mode == Submaster)
	{
		assignSubmasters(false);

		// Reset submasters to 100% if they are not occupied
		// anymore by another slider
		_app->resetSubmasters();
	}
}

void VCDockSlider::init()
{
	m_valueLabel->setBackgroundOrigin(ParentOrigin);
	m_slider->setBackgroundOrigin(ParentOrigin);

	setCaption("No Name");
	setMode(Speed);
	m_inputChannel = -1;
	m_channels.clear();

	m_slider->setPageStep(1);
	m_slider->setValue(0);

	connect(_app, SIGNAL(modeChanged()), this, SLOT(slotModeChanged()));
	connect(_app->virtualConsole(), SIGNAL(sendFeedBack()),
		this, SLOT(slotFeedBack()));

	connect(_app->virtualConsole(), SIGNAL(InpEvent(const int, const int, const int)), 
		this, SLOT(slotInputEvent(const int, const int, const int)));

	Q_ASSERT(m_sliderKeyBind == NULL);
	m_sliderKeyBind = new SliderKeyBind();

	connect(m_sliderKeyBind, SIGNAL(pressedUp()), this, SLOT(slotPressUp()));
	connect(m_sliderKeyBind, SIGNAL(pressedDown()), this, SLOT(slotPressDown()));
}

void VCDockSlider::destroy()
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

/*********************************************************************
 * Background image
 *********************************************************************/
void VCDockSlider::setBackgroundImage(const QString& path)
{
	m_hasCustomBackgroundColor = false;
	m_backgroundImage = path;
	setPaletteBackgroundPixmap(QPixmap(path));
}

const QString& VCDockSlider::backgroundImage()
{
	return m_backgroundImage;
}

void VCDockSlider::chooseBackgroundImage()
{
	QLCImagePreview* preview = new QLCImagePreview();
	QFileDialog* fd = new QFileDialog(this);
	fd->setContentsPreviewEnabled(true);
	fd->setContentsPreview(preview, preview);
	fd->setPreviewMode(QFileDialog::Contents);
	fd->setFilter("Images (*.png *.xpm *.jpg *.gif)");
	fd->setSelection(backgroundImage());
	
	if (fd->exec() == QDialog::Accepted)
		setBackgroundImage(fd->selectedFile());
	
	delete preview;
	delete fd;
}

/*********************************************************************
 * Background color
 *********************************************************************/
void VCDockSlider::setBackgroundColor(const QColor& color)
{
	m_hasCustomBackgroundColor = true;
	m_backgroundImage = QString::null;
	setPaletteBackgroundColor(color);
}

void VCDockSlider::resetBackgroundColor()
{
	QColor fg;

	m_hasCustomBackgroundColor = false;
	m_backgroundImage = QString::null;

	/* Store foreground color */
	if (m_hasCustomForegroundColor == true)
		fg = paletteForegroundColor();

	/* Reset the whole palette */
	unsetPalette();

	/* Restore foreground color */
	if (fg.isValid() == true)
		setPaletteForegroundColor(fg);

	_app->doc()->setModified();
}

void VCDockSlider::chooseBackgroundColor()
{
	QColor color;
	color = QColorDialog::getColor(backgroundColor());
	if (color.isValid())
		setBackgroundColor(color);
}

/*********************************************************************
 * Foreground color
 *********************************************************************/
void VCDockSlider::setForegroundColor(const QColor& color)
{
	m_hasCustomForegroundColor = true;
	setPaletteForegroundColor(color);
}

void VCDockSlider::resetForegroundColor()
{
	QColor bg;

	m_hasCustomForegroundColor = false;

	/* Store background color */
	if (m_hasCustomBackgroundColor == true)
		bg = paletteBackgroundColor();

	/* Reset the whole palette */
	unsetPalette();

	/* Restore foreground color */
	if (bg.isValid() == true)
		setPaletteBackgroundColor(bg);
	else if (m_backgroundImage.isEmpty() == false)
		setPaletteBackgroundPixmap(QPixmap(m_backgroundImage));

	_app->doc()->setModified();
}

void VCDockSlider::chooseForegroundColor()
{
	QColor color;
	color = QColorDialog::getColor(foregroundColor());
	if (color.isValid())
		setForegroundColor(color);
}

/*********************************************************************
 * Font
 *********************************************************************/

void VCDockSlider::setFont(const QFont& font)
{
	m_hasCustomFont = true;
	QWidget::setFont(font);
}

void VCDockSlider::resetFont()
{
	m_hasCustomFont = false;
	unsetFont();
	_app->doc()->setModified();
}

void VCDockSlider::chooseFont()
{
	bool ok = false;
	QFont f = QFontDialog::getFont(&ok, font());
	if (ok == true)
		setFont(f);
}

/*****************************************************************************
 * Caption
 *****************************************************************************/

void VCDockSlider::setCaption(const QString& text)
{
	m_infoLabel->setText(text);
	UI_VCDockSlider::setCaption(text);
}

void VCDockSlider::rename()
{
	QString text;
	bool ok = false;

	text = QInputDialog::getText("Rename slider",
				     "Set slider caption:", QLineEdit::Normal,
				     QString::null, &ok, this );
	if (ok == true && text.isEmpty() == false)
		setCaption(text);
}

/*****************************************************************************
 * Properties
 *****************************************************************************/
void VCDockSlider::editProperties()
{
	VCDockSliderProperties* sp = new VCDockSliderProperties(this);
	sp->init();
	if (sp->exec() == QDialog::Accepted)
		_app->doc()->setModified();
	
	delete sp;
}

/*****************************************************************************
 * Key binding
 *****************************************************************************/

void VCDockSlider::setSliderKeyBind(const SliderKeyBind* skb)
{
	Q_ASSERT(skb != NULL);

	if (m_sliderKeyBind)
		delete m_sliderKeyBind;

	m_sliderKeyBind = new SliderKeyBind(skb);

	connect(m_sliderKeyBind, SIGNAL(pressedUp()), this, SLOT(slotPressUp()));
	connect(m_sliderKeyBind, SIGNAL(pressedDown()), this, SLOT(slotPressDown()));
}

/*****************************************************************************
 * Slider Mode
 *****************************************************************************/

void VCDockSlider::setMode(SliderMode m)
{
	m_mode = m;

	switch (m)
	{
	case Speed:
	{
		t_bus_value value;
		Bus::value(m_busID, value);
		//
		// Set name label
		//
		QString name = Bus::name(m_busID);
		if (name == QString::null)
		{
			name.sprintf("%.2d", m_busID + 1);
		}

		m_tapInButton->setText(name);

		connect(Bus::emitter(), SIGNAL(nameChanged(t_bus_id, const QString&)),
			this, SLOT(slotBusNameChanged(t_bus_id, const QString&)));

		connect(Bus::emitter(), SIGNAL(valueChanged(t_bus_id, t_bus_value)),
			this, SLOT(slotBusValueChanged(t_bus_id, t_bus_value)));

		m_slider->setRange(m_busLowLimit * KFrequency,
				   m_busHighLimit * KFrequency);

		m_infoLabel->hide();
		m_tapInButton->show();

		m_time.start();

		slotBusValueChanged(m_busID, value);

	}
	break;

	case Level:
	{
		disconnect(Bus::emitter(),SIGNAL(nameChanged(t_bus_id,const QString&)),
			   this, SLOT(slotBusNameChanged(t_bus_id, const QString&)));

		disconnect(Bus::emitter(), SIGNAL(valueChanged(t_bus_id, t_bus_value)),
			   this, SLOT(slotBusValueChanged(t_bus_id, t_bus_value)));

		m_slider->setRange(m_levelLowLimit, m_levelHighLimit);
		slotSliderValueChanged(m_slider->value());

		m_infoLabel->show();
		m_tapInButton->hide();
	}
	break;

	case Submaster:
	{
		disconnect(Bus::emitter(),SIGNAL(nameChanged(t_bus_id,const QString&)),
			   this, SLOT(slotBusNameChanged(t_bus_id, const QString&)));

		disconnect(Bus::emitter(), SIGNAL(valueChanged(t_bus_id, t_bus_value)),
			   this, SLOT(slotBusValueChanged(t_bus_id, t_bus_value)));

		m_slider->setRange(0, 100);
		slotSliderValueChanged(m_slider->value());

		m_infoLabel->show();
		m_tapInButton->hide();
	}
	break;
	}
}

QString VCDockSlider::modeToString(SliderMode mode)
{
	switch(mode)
	{
	default:
	case Speed:
		return QString("Speed");
	case Level:
		return QString("Level");
	case Submaster:
		return QString("Submaster");
	}
}

VCDockSlider::SliderMode VCDockSlider::stringToMode(const QString& str)
{
	if (str == "Speed")
		return Speed;
	else if (str == "Level")
		return Level;
	else if (str == "Submaster")
		return Submaster;
	else
		return Speed;
}

/*****************************************************************************
 * Bus
 *****************************************************************************/

bool VCDockSlider::setBusID(t_bus_id id)
{
	if (id >= KBusIDMin && id < KBusCount)
	{
		m_busID = id;

		return true;
	}
	else
	{
		return false;
	}
}

void VCDockSlider::setBusRange(t_bus_value lo, t_bus_value hi)
{
	m_busLowLimit = lo;
	m_busHighLimit = hi;
}


void VCDockSlider::busRange(t_bus_value &lo, t_bus_value &hi)
{
	lo = m_busLowLimit;
	hi = m_busHighLimit;
}

void VCDockSlider::slotBusNameChanged(t_bus_id id, const QString &name)
{
	if (id == m_busID)
	{
		m_tapInButton->setText(name);
	}
}

void VCDockSlider::slotBusValueChanged(t_bus_id id, t_bus_value value)
{
	if (id == m_busID)
	{
		m_updateOnly = true;
		m_slider->setValue(value);
		m_updateOnly = false;
	}
}

/*****************************************************************************
 * Value & Submasters
 *****************************************************************************/

void VCDockSlider::assignSubmasters(bool assign)
{
	QValueList<t_channel>::iterator it;
	for (it = m_channels.begin(); it != m_channels.end(); ++it)
	{
		if (assign == true)
		{
			_app->assignSubmaster(*it);
		}
		else
		{
			_app->resignSubmaster(*it);
		}
	}
}

void VCDockSlider::setLevelRange(t_value lo, t_value hi)
{
	m_levelLowLimit = lo;
	m_levelHighLimit = hi;
}

void VCDockSlider::levelRange(t_value &lo, t_value &hi)
{
	lo = m_levelLowLimit;
	hi = m_levelHighLimit;
}

void VCDockSlider::appendChannel(t_channel channel)
{
	/* Append each channel only once to the list */
	if (m_channels.contains(channel) == 0)
		m_channels.append(channel);
}

void VCDockSlider::removeChannel(t_channel channel)
{
	m_channels.remove(m_channels.find(channel));
}

void VCDockSlider::clearChannels()
{
	m_channels.clear();
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCDockSlider::loader(QDomDocument* doc, QDomElement* root, QWidget* parent)
{
	VCDockSlider* slider = NULL;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);
	Q_ASSERT(parent != NULL);

	if (root->tagName() != KXMLQLCVCDockSlider)
	{
		qWarning("Slider node not found!");
		return false;
	}

	/* Create a new slider into its parent */
	slider = new VCDockSlider(parent);
	slider->init();
	slider->show();

	/* Continue loading */
	return slider->loadXML(doc, root);
}

bool VCDockSlider::loadXML(QDomDocument* doc, QDomElement* root)
{
	bool visible = false;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	
	QDomNode node;
	QDomElement tag;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCVCDockSlider)
	{
		qWarning("Slider node not found!");
		return false;
	}

	/* Caption */
	setCaption(root->attribute(KXMLQLCVCCaption));

	/* Slider mode */
	setMode(VCDockSlider::stringToMode(
			root->attribute(KXMLQLCVCDockSliderMode)));

	/* Slider value */
	str = root->attribute(KXMLQLCVCDockSliderValue);
	m_slider->setValue(str.toInt());

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
			/* TODO */
		}
		else if (tag.tagName() == KXMLQLCVCDockSliderBus)
		{
			t_bus_id busID = KBusIDDefaultFade;
			t_bus_value busLowLimit = 0;
			t_bus_value busHighLimit = 5;

			busID = tag.text().toInt();

			str = tag.attribute(KXMLQLCVCDockSliderBusLowLimit);
			busLowLimit = str.toInt();

			str = tag.attribute(KXMLQLCVCDockSliderBusHighLimit);
			busHighLimit = str.toInt();

			setBusRange(busLowLimit, busHighLimit);
			setBusID(busID);
		}
		else if (tag.tagName() == KXMLQLCVCDockSliderLevel)
		{
			t_value levelLowLimit = 0;
			t_value levelHighLimit = 255;
			
			str = tag.attribute(KXMLQLCVCDockSliderLevelLowLimit);
			levelLowLimit = str.toInt();

			str = tag.attribute(KXMLQLCVCDockSliderLevelHighLimit);
			levelHighLimit = str.toInt();

			setLevelRange(levelLowLimit, levelHighLimit);
		}
		else if (tag.tagName() == KXMLQLCVCDockSliderInputChannel)
		{
			int inputChannel = -1;
			inputChannel = tag.text().toInt();
			setInputChannel(inputChannel);
		}
		else if (tag.tagName() == KXMLQLCVCDockSliderDMXChannel)
		{
			t_channel channel = 0;
			channel = tag.text().toInt();
			appendChannel(channel);
		}
		else if (tag.tagName() == KXMLQLCSliderKeyBind)
		{
			SliderKeyBind* skb = NULL;
			skb = SliderKeyBind::loader(doc, &tag);
			setSliderKeyBind(skb);
			delete skb;
		}
		else
		{
			qWarning("Unknown slider tag: %s",
				 (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}

	return true;
}

bool VCDockSlider::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(vc_root != NULL);

	/* VC Slider entry */
	root = doc->createElement(KXMLQLCVCDockSlider);
	vc_root->appendChild(root);

	/* Caption */
	root.setAttribute(KXMLQLCVCCaption, caption());

	/* Slider Mode */
	root.setAttribute(KXMLQLCVCDockSliderMode,
			  VCDockSlider::modeToString(mode()));

	/* Value */
	str.setNum(m_slider->value());
	root.setAttribute(KXMLQLCVCDockSliderValue, str);

	/* Appearance */
	saveXMLAppearance(doc, &root);

	/* Bus & Bus ID */
	tag = doc->createElement(KXMLQLCVCDockSliderBus);
	root.appendChild(tag);
	str.setNum(busID());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Bus low limit */
	str.setNum(m_busLowLimit);
	tag.setAttribute(KXMLQLCVCDockSliderBusLowLimit, str);

	/* Bus high limit */
	str.setNum(m_busHighLimit);
	tag.setAttribute(KXMLQLCVCDockSliderBusHighLimit, str);

	/* Level */
	tag = doc->createElement(KXMLQLCVCDockSliderLevel);
	root.appendChild(tag);

	/* Level low limit */
	str.setNum(m_levelLowLimit);
	tag.setAttribute(KXMLQLCVCDockSliderLevelLowLimit, str);

	/* Level high limit */
	str.setNum(m_levelHighLimit);
	tag.setAttribute(KXMLQLCVCDockSliderLevelHighLimit, str);

	/* Key binding */
	m_sliderKeyBind->saveXML(doc, &root);

	/* Midi input */
	tag = doc->createElement(KXMLQLCVCDockSliderInputChannel);
	root.appendChild(tag);
	str.setNum(m_inputChannel);
	text = doc->createTextNode(str);
	tag.appendChild(text);

	QValueList<t_channel>::Iterator it;
	for (it = m_channels.begin(); it != m_channels.end(); ++it)
	{
		tag = doc->createElement(KXMLQLCVCDockSliderDMXChannel);
		root.appendChild(tag);
		str.setNum(*it);
		text = doc->createTextNode(str);
		tag.appendChild(text);
	}

	return FileHandler::saveXMLWindowState(doc, &root, this);
}

bool VCDockSlider::loadXMLAppearance(QDomDocument* doc, QDomElement* root)
{
	QDomNode node;
	QDomElement tag;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCVCAppearance)
	{
		qWarning("Appearance node not found!");
		return false;
	}

	/* Children */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCVCFrameStyle)
		{
			int style = 0;
			style = VirtualConsole::stringToFrameStyle(tag.text());
			setFrameStyle(style);
		}
		else if (tag.tagName() == KXMLQLCVCForegroundColor)
		{
			if (tag.text() != KXMLQLCVCColorDefault)
			{
				QColor color(tag.text().toInt());
				setForegroundColor(color);
			}
		}
		else if (tag.tagName() == KXMLQLCVCBackgroundColor)
		{
			if (tag.text() != KXMLQLCVCColorDefault)
				setBackgroundColor(QColor(tag.text().toInt()));
		}
		else if (tag.tagName() == KXMLQLCVCBackgroundImage)
		{
			if (tag.text() != KXMLQLCVCBackgroundImageNone)
				setBackgroundImage(tag.text());
		}
		else if (tag.tagName() == KXMLQLCVCFont)
		{
			if (tag.text() != KXMLQLCVCFontDefault)
			{
				QFont font;
				font.fromString(tag.text());
				setFont(font);
			}
		}
		else
		{
			qWarning("Unknown Appearance tag: %s",
				 (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}
}

bool VCDockSlider::saveXMLAppearance(QDomDocument* doc, QDomElement* slider_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(slider_root != NULL);

	/* VC Label entry */
	root = doc->createElement(KXMLQLCVCAppearance);
	slider_root->appendChild(root);

	/* Frame style */
	tag = doc->createElement(KXMLQLCVCFrameStyle);
	root.appendChild(tag);
	text = doc->createTextNode(VirtualConsole::frameStyleToString(frameStyle()));
	tag.appendChild(text);

	/* Foreground color */
	tag = doc->createElement(KXMLQLCVCForegroundColor);
	root.appendChild(tag);
	if (hasCustomForegroundColor() == true)
		str.setNum(paletteForegroundColor().rgb());
	else
		str = KXMLQLCVCColorDefault;
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Background color */
	tag = doc->createElement(KXMLQLCVCBackgroundColor);
	root.appendChild(tag);
	if (hasCustomBackgroundColor() == true)
		str.setNum(paletteBackgroundColor().rgb());
	else
		str = KXMLQLCVCColorDefault;
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Background image */
	tag = doc->createElement(KXMLQLCVCBackgroundImage);
	root.appendChild(tag);
	if (backgroundImage() != QString::null)
		str = m_backgroundImage;
	else
		str = KXMLQLCVCBackgroundImageNone;
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Font */
	tag = doc->createElement(KXMLQLCVCFont);
	root.appendChild(tag);
	if (hasCustomFont() == true)
		str = font().toString();
	else
		str = KXMLQLCVCFontDefault;
	text = doc->createTextNode(str);
	tag.appendChild(text);	

	return true;
}

/*****************************************************************************
 * QLC Mode change
 *****************************************************************************/

void VCDockSlider::slotModeChanged()
{
	repaint();
}

/*****************************************************************************
 * Slots
 *****************************************************************************/

//
// Slider up Key has been pressed
//
void VCDockSlider::slotPressUp()
{
	assert(m_sliderKeyBind);
	m_slider->setPageStep(5);
	m_slider->addStep();
}

//
// Slider down Key has been pressed
//
void VCDockSlider::slotPressDown()
{
	assert(m_sliderKeyBind);
	m_slider->setPageStep(5);
	m_slider->subtractStep();
}

//
// Slider has been moved
//
void VCDockSlider::slotSliderValueChanged(const int value)
{
	QString t;
	if (m_mode == Speed)
	{
		if (m_updateOnly == false)
		{
			if (Bus::setValue(m_busID, m_slider->value()) == false)
			{
				m_valueLabel->setText("ERROR");
				return;
			}
		}

		QString num;
		num.sprintf("%.2fs", ((float) value / (float) KFrequency));
		m_valueLabel->setText(num);
		int range = m_busHighLimit - m_busLowLimit;
		float f = ((float) value / (float) KFrequency);
		_app->inputPlugin()->feedBack(1, m_inputChannel,
					      127 - int((f * 127) / range));
	} 
	else if (m_mode == Level)
	{
		QString num;
		num.sprintf("%.3d", m_levelHighLimit - value + m_levelLowLimit);
		m_valueLabel->setText(num);

		QValueList<t_channel>::iterator it;
		for(it = m_channels.begin(); it != m_channels.end(); ++it)
		{
			_app->setValue(*it, m_levelHighLimit - 
				       value + m_levelLowLimit);
		}

		_app->inputPlugin()->feedBack(1, m_inputChannel, 
					      127 - (value * 127) / 255);
	}
	else
	{
		QString num;
		num.sprintf("%d%%", 100 - value);
		m_valueLabel->setText(num);

		QValueList<t_channel>::iterator it;
		for(it = m_channels.begin(); it != m_channels.end(); ++it)
		{
			_app->setSubmasterValue(*it, 100 - value);
		}

		_app->inputPlugin()->feedBack(1, m_inputChannel,
					      127 - (value * 127) / 100);
	}
}

//
// Calculate speed from button tap intervals
//
void VCDockSlider::slotTapInButtonClicked()
{
	int t = m_time.elapsed();
	m_slider->setValue(static_cast<int> (t * 0.001 * KFrequency));
	m_time.restart();
}

void VCDockSlider::slotInputEvent(const int id, const int channel, const int value)
{
	if (id == 1 && channel == m_inputChannel)
	{
		if (m_mode == Submaster)
		{
			m_slider->setValue(100 - value * 100 / 127);
		}
		if (m_mode == Speed)
		{
			int range = m_busHighLimit - m_busLowLimit;
			m_slider->setValue(m_busHighLimit * KFrequency - 
					   (range * value * KFrequency) / 127);
		}
		if (m_mode == Level)
		{
			int range = m_levelHighLimit - m_levelLowLimit;
			m_slider->setValue(255 - (m_levelLowLimit / 127 + 
						  (range * value + 1) / 127));
		}
	}
}

void VCDockSlider::slotFeedBack()
{
	int value = m_slider->value();
	if (m_mode == Speed)
	{
		t_bus_value range = m_busHighLimit - m_busLowLimit;
		float f = ((float) value / (float) KFrequency);
		_app->inputPlugin()->feedBack(1, m_inputChannel, 
					      127 - int((f * 127) / range));
	}
	else if (m_mode == Level)
	{
		_app->inputPlugin()->feedBack(1, m_inputChannel,
					      127 - (value * 127) / 255);
	}
	else
	{
		_app->inputPlugin()->feedBack(1, m_inputChannel,
					      127 - (value * 127) / 100);
	}
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

//
// Mouse button pressed inside the widget
//
void VCDockSlider::mousePressEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		if (m_static == false)
		{
			// Can't delete or edit default fade & hold sliders
			_app->virtualConsole()->setSelectedWidget(this);
		}

		if (m_resizeMode == true)
		{
			setMouseTracking(false);
			m_resizeMode = false;
		}

		if ((e->button() & LeftButton || e->button() & MidButton) &&
		    m_static == false)
		{
			if (e->x() > rect().width() - 10 &&
			    e->y() > rect().height() - 10)
			{
				m_resizeMode = true;
				setMouseTracking(true);
				setCursor(QCursor(SizeFDiagCursor));
			}
			else
			{
				m_mousePressPoint = QPoint(e->x(), e->y());
				setCursor(QCursor(SizeAllCursor));
			}
		}
		else if (e->button() & RightButton)
		{
			invokeMenu(mapToGlobal(e->pos()));
		}
	}
	else
	{
		QFrame::mousePressEvent(e);
	}
}


//
// QSlider passes this event thru, so grab it also
//
void VCDockSlider::contextMenuEvent(QContextMenuEvent* e)
{
	if (_app->mode() == App::Design && m_static == false)
	{
		// invokeMenu(mapToGlobal(e->pos()));
	}
}

/*****************************************************************************
 * Widget menu
 *****************************************************************************/

void VCDockSlider::invokeMenu(QPoint point)
{
	if (m_static == true)
		invokeStaticSliderMenu(point);
	else
		invokeDynamicSliderMenu(point);
}

void VCDockSlider::invokeStaticSliderMenu(QPoint point)
{
	QPopupMenu menu;
	menu.insertItem(QPixmap(QString(PIXMAPS) + QString("/configure.png")),
			QString("&Properties..."), KVCMenuEditProperties);
	
	if (menu.exec(point) == KVCMenuEditProperties)
	{
		bool ok = false;
		QString current;
		current.sprintf("%d-%d", m_busLowLimit, m_busHighLimit);
		
		QString text;
		text = QInputDialog::getText("Edit slider value range",
					     "Value range in seconds (for example 0-10):",
					     QLineEdit::Normal, current, &ok, this);
		
		if (ok == true && text.isEmpty() == false)
		{
			int dash = text.find('-');
			QString min = text.left(dash);
			QString max = text.mid(dash + 1);
			
			if (min.toInt() >= max.toInt())
			{
				QMessageBox::warning(this,
						     "Invalid value range",
						     "Minimum value cannot be " \
						     "greater than or equal to " \
						     "the maximum value");
			}
			else
			{
				setBusRange(min.toInt(), max.toInt());
				setMode(Speed);
				
				/* TODO: Save these to workspace, not settings!! */
				if (m_busID == KBusIDDefaultFade)
				{
					_app->settings()->set(KEY_DEFAULT_FADE_MIN,
							      min.toInt());
					_app->settings()->set(KEY_DEFAULT_FADE_MAX,
							      max.toInt());
				}
				else
				{
					_app->settings()->set(KEY_DEFAULT_HOLD_MIN,
							      min.toInt());
					_app->settings()->set(KEY_DEFAULT_HOLD_MAX,
							      max.toInt());
				}
			}
		}
	}
}

void VCDockSlider::invokeDynamicSliderMenu(QPoint point)
{
	// Foreground menu
	QPopupMenu* fgMenu = new QPopupMenu();
	fgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/color.png")),
			   "&Color...", KVCMenuForegroundColor);
	fgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/undo.png")),
			   "&Default", KVCMenuForegroundDefault);

	// Background menu
	QPopupMenu* bgMenu = new QPopupMenu();
	bgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/color.png")),
			   "&Color...", KVCMenuBackgroundColor);
	bgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/image.png")),
			   "&Image...", KVCMenuBackgroundImage);
	bgMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/undo.png")),
			   "&Default", KVCMenuBackgroundDefault);

	// Font menu
	QPopupMenu* fontMenu = new QPopupMenu();
	fontMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/fonts.png")),
			     "&Font...", KVCMenuFont);
	fontMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/undo.png")),
			   "&Default", KVCMenuFontDefault);

	// Frame menu
	QPopupMenu* frameMenu = new QPopupMenu();
	frameMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/framesunken.png")),
			      "&Sunken", KVCMenuFrameSunken);
	frameMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/frameraised.png")),
			      "&Raised", KVCMenuFrameRaised);
	frameMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/framenone.png")),
			      "&None", KVCMenuFrameNone);

	// Stacking order menu
	QPopupMenu* stackMenu = new QPopupMenu();
	stackMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/up.png")),
			      "Bring to &Front", KVCMenuStackingRaise);
	stackMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/down.png")),
			      "Send to &Back", KVCMenuStackingLower);

	// Edit menu
	QPopupMenu* editMenu = new QPopupMenu();
	editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editcut.png")),
			     "Cut", KVCMenuEditCut);
	editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editcopy.png")),
			     "Copy", KVCMenuEditCopy);
	editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editpaste.png")),
			     "Paste", KVCMenuEditPaste);
	editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editdelete.png")),
			     "Delete", KVCMenuEditDelete);

	editMenu->insertSeparator();

	editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/configure.png")),
			       "&Properties...", KVCMenuEditProperties);
	editMenu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editclear.png")),
			       "&Rename...", KVCMenuEditRename);

	editMenu->setItemEnabled(KVCMenuEditCut, false);
	editMenu->setItemEnabled(KVCMenuEditCopy, false);
	editMenu->setItemEnabled(KVCMenuEditPaste, false);

	editMenu->insertSeparator();

	editMenu->insertItem("Background", bgMenu, 0);
	editMenu->insertItem("Foreground", fgMenu, 0);
	editMenu->insertItem("Font", fontMenu, 0);
	editMenu->insertItem("Frame", frameMenu, 0);
	editMenu->insertItem("Stacking Order", stackMenu, 0);

	connect(editMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));
	connect(bgMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));
	connect(fgMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));
	connect(fontMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));
	connect(frameMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));
	connect(stackMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));
	
	editMenu->exec(point);
	delete editMenu;
	delete bgMenu;
	delete fgMenu;
	delete fontMenu;
	delete frameMenu;
	delete stackMenu;
}

void VCDockSlider::slotMenuCallback(int item)
{
	switch (item)
	{
	case KVCMenuEditCut:
		break;
	case KVCMenuEditCopy:
		break;
	case KVCMenuEditPaste:
		break;
	case KVCMenuEditDelete:
		destroy();
		break;

	case KVCMenuEditProperties:
		editProperties();
		break;

	case KVCMenuEditRename:
		rename();
		break;

	case KVCMenuForegroundColor:
		chooseForegroundColor();
		break;

	case KVCMenuForegroundDefault:
		resetForegroundColor();
		break;

	case KVCMenuBackgroundColor:
		chooseBackgroundColor();
		break;
	
	case KVCMenuBackgroundImage:
		chooseBackgroundImage();
		break;
	
	case KVCMenuBackgroundDefault:
		resetBackgroundColor();
		break;

	case KVCMenuFont:
		chooseFont();
		break;

	case KVCMenuFontDefault:
		resetFont();
		break;

	case KVCMenuFrameSunken:
		setFrameStyle(KFrameStyleSunken);
		break;

	case KVCMenuFrameRaised:
		setFrameStyle(KFrameStyleRaised);
		break;

	case KVCMenuFrameNone:
		setFrameStyle(KFrameStyleNone);
		break;

	case KVCMenuStackingRaise:
		raise();
		break;

	case KVCMenuStackingLower:
		lower();
		break;

	default:
		break;
	}
}

//
// Mouse button released inside the widget
//
void VCDockSlider::mouseReleaseEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design && m_static == false)
	{
		unsetCursor();
		m_resizeMode = false;
		setMouseTracking(false);
	}
	else
	{
		QFrame::mouseReleaseEvent(e);
	}
}


void VCDockSlider::paintEvent(QPaintEvent* e)
{
	QFrame::paintEvent(e);

	if (_app->mode() == App::Design &&
	    _app->virtualConsole()->selectedWidget() == this)
	{
		QPainter p(this);

		// Draw a dotted line around the widget
		QPen pen(DotLine);
		pen.setWidth(2);
		p.setPen(pen);
		p.drawRect(1, 1, rect().width() - 1, rect().height() - 1);

		// Draw a resize handle
		QBrush b(SolidPattern);
		p.fillRect(rect().width() - 10, rect().height() - 10, 10, 10, b);
	}
}

void VCDockSlider::mouseMoveEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design && m_static == false)
	{
		if (m_resizeMode == true)
		{
			QPoint p(QCursor::pos());
			resizeTo(mapFromGlobal(p));
			_app->doc()->setModified();
		}
		else if (e->state() & LeftButton || e->state() & MidButton)
		{
			QPoint p(parentWidget()->mapFromGlobal(QCursor::pos()));
			p.setX(p.x() - m_mousePressPoint.x());
			p.setY(p.y() - m_mousePressPoint.y());

			moveTo(p);
			_app->doc()->setModified();
		}
	}
	else
	{
		QFrame::mouseMoveEvent(e);
	}
}

void VCDockSlider::customEvent(QCustomEvent* e)
{
	if (e->type() == KVCMenuEvent)
	{
		// parseWidgetMenu(((VCMenuEvent*) e)->menuItem());
	}

}

void VCDockSlider::resizeTo(QPoint p)
{
	// Grid settings
	if (_app->virtualConsole()->isGridEnabled())
	{
		p.setX(p.x() - (p.x() % _app->virtualConsole()->gridX()));
		p.setY(p.y() - (p.y() % _app->virtualConsole()->gridY()));
	}

	// Map to parent coordinates so that they can be compared
	p = mapToParent(p);

	// Don't move beyond left or right
	if (p.x() < 0)
	{
		p.setX(0);
	}
	else if (p.x() > parentWidget()->width())
	{
		p.setX(parentWidget()->width());
	}

	// Don't move beyond top or bottom
	if (p.y() < 0)
	{
		p.setY(0);
	}
	else if (p.y() > parentWidget()->height())
	{
		p.setY(parentWidget()->height());
	}

	// Map back so that this can be resized
	p = mapFromParent(p);

	// Do the resize
	resize(p.x(), p.y());
}


void VCDockSlider::moveTo(QPoint p)
{
	// Grid settings
	if (_app->virtualConsole()->isGridEnabled())
	{
		p.setX(p.x() - (p.x() % _app->virtualConsole()->gridX()));
		p.setY(p.y() - (p.y() % _app->virtualConsole()->gridY()));
	}

	// Don't move beyond left or right
	if (p.x() < 0)
	{
		p.setX(0);
	}
	else if (p.x() + rect().width() > parentWidget()->width())
	{
		p.setX(parentWidget()->width() - rect().width());
	}

	// Don't move beyond top or bottom
	if (p.y() < 0)
	{
		p.setY(0);
	}
	else if (p.y() + rect().height() > parentWidget()->height())
	{
		p.setY(parentWidget()->height() - rect().height());
	}

	// Do the move
	move(p);
}

void VCDockSlider::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (_app->mode() == App::Design)
	{
		invokeMenu(mapToGlobal(e->pos()));
	}
	else
	{
		mousePressEvent(e);
	}
}
