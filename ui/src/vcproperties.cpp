/*
  Q Light Controller
  vcproperties.cpp

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

#include <QCheckBox>
#include <QSpinBox>
#include <QtXml>

#include "qlcinputprofile.h"
#include "qlcinputchannel.h"
#include "qlcfile.h"

#include "selectinputchannel.h"
#include "virtualconsole.h"
#include "vcproperties.h"
#include "inputpatch.h"
#include "inputmap.h"
#include "vcframe.h"
#include "app.h"
#include "bus.h"

extern App* _app;

/*****************************************************************************
 * Properties Initialization
 *****************************************************************************/

VCProperties::VCProperties() : VCWidgetProperties()
{
	m_contents = NULL;

	m_width = 640;
	m_height = 480;

	m_gridEnabled = true;
	m_gridX = 10;
	m_gridY = 10;

	m_keyRepeatOff = true;
	m_grabKeyboard = true;

	m_slidersVisible = true;

	m_fadeLowLimit = 0;
	m_fadeHighLimit = 10;
	m_holdLowLimit = 0;
	m_holdHighLimit = 10;

	m_fadeInputUniverse = KInputUniverseInvalid;
	m_fadeInputChannel = KInputChannelInvalid;
	m_holdInputUniverse = KInputUniverseInvalid;
	m_holdInputChannel = KInputChannelInvalid;
}

VCProperties::VCProperties(const VCProperties& properties)
	: VCWidgetProperties(properties)
{
	*this = properties;
}

VCProperties::~VCProperties()
{
	/* Don't delete m_contents because there might be several copies of
	   this class in memory and deleting just one of them would delete
	   the contents for others as well. */
}

VCProperties& VCProperties::operator=(const VCProperties& properties)
{
	/* The contents of m_contents cannot be copied. Instead, only the
	   pointer is copied. */
	m_contents = properties.m_contents;

	m_gridEnabled = properties.m_gridEnabled;
	m_gridX = properties.m_gridX;
	m_gridY = properties.m_gridY;

	m_keyRepeatOff = properties.m_keyRepeatOff;
	m_grabKeyboard = properties.m_grabKeyboard;

	m_slidersVisible = properties.m_slidersVisible;

	m_fadeLowLimit = properties.m_fadeLowLimit;
	m_fadeHighLimit = properties.m_fadeHighLimit;
	m_holdLowLimit = properties.m_holdLowLimit;
	m_holdHighLimit = properties.m_holdHighLimit;

	m_fadeInputUniverse = properties.m_fadeInputUniverse;
	m_fadeInputChannel = properties.m_fadeInputChannel;
	m_holdInputUniverse = properties.m_holdInputUniverse;
	m_holdInputChannel = properties.m_holdInputChannel;

	return *this;
}

/*****************************************************************************
 * VC Contents
 *****************************************************************************/

void VCProperties::resetContents()
{
	/* Get rid of any existing contents */
	if (m_contents != NULL)
		delete m_contents;

	/* Create new contents */
	m_contents = new VCFrame(_app);
}

/*****************************************************************************
 * Properties Load & Save
 *****************************************************************************/

bool VCProperties::loadXML(const QDomElement* vc_root)
{
	QDomNode node;
	QDomElement tag;
	QString str;

	Q_ASSERT(vc_root != NULL);

	if (vc_root->tagName() != KXMLQLCVirtualConsole)
	{
		qDebug() << "Virtual Console node not found!";
		return false;
	}

	node = vc_root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();

		if (tag.tagName() == KXMLQLCVCProperties)
		{
			/* Properties */
			loadProperties(&tag);
		}
		else if (tag.tagName() == KXMLQLCVCFrame)
		{
			/* Contents */
			if (m_contents == NULL)
				m_contents = new VCFrame(_app);
			Q_ASSERT(m_contents != NULL);
			m_contents->loadXML(&tag);
		}
		else
		{
			qDebug() << "Unknown Virtual Console tag"
				 << tag.tagName();
		}

		/* Next node */
		node = node.nextSibling();
	}

	return true;
}

bool VCProperties::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
	QDomElement prop_root;
	QDomElement vc_root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(wksp_root != NULL);

	/* Virtual Console entry */
	vc_root = doc->createElement(KXMLQLCVirtualConsole);
	wksp_root->appendChild(vc_root);

	/* Contents */
	if (m_contents != NULL)
		m_contents->saveXML(doc, &vc_root);

	/* Properties entry */
	prop_root = doc->createElement(KXMLQLCVCProperties);
	vc_root.appendChild(prop_root);

	/* Grid */
	tag = doc->createElement(KXMLQLCVCPropertiesGrid);
	prop_root.appendChild(tag);
	if (m_gridEnabled == true)
		tag.setAttribute(KXMLQLCVCPropertiesGridEnabled, KXMLQLCTrue);
	else
		tag.setAttribute(KXMLQLCVCPropertiesGridEnabled, KXMLQLCFalse);

	tag.setAttribute(KXMLQLCVCPropertiesGridXResolution,
			 QString("%1").arg(m_gridX));
	tag.setAttribute(KXMLQLCVCPropertiesGridYResolution,
			 QString("%1").arg(m_gridY));

	/* Keyboard settings */
	tag = doc->createElement(KXMLQLCVCPropertiesKeyboard);
	prop_root.appendChild(tag);

	/* Grab keyboard */
	if (m_grabKeyboard == true)
		tag.setAttribute(KXMLQLCVCPropertiesKeyboardGrab, KXMLQLCTrue);
	else
		tag.setAttribute(KXMLQLCVCPropertiesKeyboardGrab, KXMLQLCFalse);

	/* Key repeat off */
	if (m_keyRepeatOff == true)
		tag.setAttribute(KXMLQLCVCPropertiesKeyboardRepeatOff,
				 KXMLQLCTrue);
	else
		tag.setAttribute(KXMLQLCVCPropertiesKeyboardRepeatOff,
				 KXMLQLCFalse);

	/* Fade slider */
	tag = doc->createElement(KXMLQLCVCPropertiesDefaultSlider);
	prop_root.appendChild(tag);
	tag.setAttribute(KXMLQLCBusRole, KXMLQLCBusFade);

	/* Sliders' visibility state is stored in fade slider */
	if (m_slidersVisible == true)
		tag.setAttribute(KXMLQLCVCPropertiesDefaultSliderVisible,
				 KXMLQLCTrue);
	else
		tag.setAttribute(KXMLQLCVCPropertiesDefaultSliderVisible,
				 KXMLQLCFalse);

	/* Fade slider limits */
	tag.setAttribute(KXMLQLCVCPropertiesLowLimit,
			 QString("%1").arg(m_fadeLowLimit));
	tag.setAttribute(KXMLQLCVCPropertiesHighLimit,
			 QString("%1").arg(m_fadeHighLimit));

	/* Fade slider external input */
	if (m_fadeInputUniverse != KInputUniverseInvalid &&
	    m_fadeInputChannel != KInputChannelInvalid)
	{
		QDomElement subtag;
		subtag = doc->createElement(KXMLQLCVCPropertiesInput);
		tag.appendChild(subtag);
		subtag.setAttribute(KXMLQLCVCPropertiesInputUniverse,
				    QString("%1").arg(m_fadeInputUniverse));
		subtag.setAttribute(KXMLQLCVCPropertiesInputChannel,
				    QString("%1").arg(m_fadeInputChannel));
        }

	/* Hold slider */
	tag = doc->createElement(KXMLQLCVCPropertiesDefaultSlider);
	prop_root.appendChild(tag);
	tag.setAttribute(KXMLQLCBusRole, KXMLQLCBusHold);

	/* Hold slider limits */
	tag.setAttribute(KXMLQLCVCPropertiesLowLimit,
			 QString("%1").arg(m_holdLowLimit));
	tag.setAttribute(KXMLQLCVCPropertiesHighLimit,
			 QString("%1").arg(m_holdHighLimit));

	/* Fade slider external input */
	if (m_holdInputUniverse != KInputUniverseInvalid &&
	    m_holdInputChannel != KInputChannelInvalid)
	{
		QDomElement subtag;
		subtag = doc->createElement(KXMLQLCVCPropertiesInput);
		tag.appendChild(subtag);
		subtag.setAttribute(KXMLQLCVCPropertiesInputUniverse,
				    QString("%1").arg(m_holdInputUniverse));
		subtag.setAttribute(KXMLQLCVCPropertiesInputChannel,
				    QString("%1").arg(m_holdInputChannel));
        }

	/* Save widget properties */
	return VCWidgetProperties::saveXML(doc, &prop_root);
}

bool VCProperties::loadProperties(const QDomElement* root)
{
	QDomElement tag;
	QDomNode node;
	QString str;

	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCVCProperties)
	{
		qDebug() << "Virtual console properties node not found!";
		return false;
	}

	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCVCPropertiesGrid)
		{
			/* Grid X resolution */
			str = tag.attribute(KXMLQLCVCPropertiesGridXResolution);
			setGridX(str.toInt());

			/* Grid Y resolution */
			str = tag.attribute(KXMLQLCVCPropertiesGridYResolution);
			setGridY(str.toInt());

			/* Grid enabled */
			str = tag.attribute(KXMLQLCVCPropertiesGridEnabled);
			if (str == KXMLQLCTrue)
				setGridEnabled(true);
			else
				setGridEnabled(false);
		}
		else if (tag.tagName() == KXMLQLCVCPropertiesKeyboard)
		{
			/* Keyboard grab */
			str = tag.attribute(KXMLQLCVCPropertiesKeyboardGrab);
			if (str == KXMLQLCTrue)
				setGrabKeyboard(true);
			else
				setGrabKeyboard(false);

			/* Key repeat */
			str = tag.attribute(KXMLQLCVCPropertiesKeyboardRepeatOff);
			if (str == KXMLQLCTrue)
				setKeyRepeatOff(true);
			else
				setKeyRepeatOff(false);
		}
		else if (tag.tagName() == KXMLQLCVCPropertiesDefaultSlider)
		{
			quint32 low = 0;
			quint32 high = 10;
			t_input_universe universe = KInputUniverseInvalid;
			t_input_channel channel = KInputChannelInvalid;
			QDomElement subtag;

			/* Bus low limit */
			str = tag.attribute(KXMLQLCVCPropertiesLowLimit);
			if (str.isNull() == false)
				low = quint32(str.toUInt());

			/* Bus high limit */
			str = tag.attribute(KXMLQLCVCPropertiesHighLimit);
			if (str.isNull() == false)
				high = quint32(str.toUInt());

			/* Sliders' visibility (on by default) */
			str = tag.attribute(KXMLQLCVCPropertiesDefaultSliderVisible);
			if (str == KXMLQLCFalse)
				setSlidersVisible(false);
			else
				setSlidersVisible(true);

			/* External input */
			subtag = tag.firstChild().toElement();
			if (subtag.isNull() == false &&
			    subtag.tagName() == KXMLQLCVCPropertiesInput)
			{
				/* Universe */
				str = subtag.attribute(KXMLQLCVCPropertiesInputUniverse);
				if (str.isNull() == false)
					universe = str.toInt();

				/* Channel */
				str = subtag.attribute(KXMLQLCVCPropertiesInputChannel);
				if (str.isNull() == false)
					channel = str.toInt();
			}

			/* Set the gathered properties to the correct slider */
			if (tag.attribute(KXMLQLCBusRole) == KXMLQLCBusFade)
			{
				setFadeLimits(low, high);
				setFadeInputSource(universe, channel);
			}
			else
			{
				setHoldLimits(low, high);
				setHoldInputSource(universe, channel);
			}
		}
		else if (tag.tagName() == KXMLQLCWidgetProperties)
		{
			VCWidgetProperties::loadXML(&tag);
		}
		else
		{
			qDebug() << "Unknown virtual console property tag:"
				 << tag.tagName();
		}

		/* Next node */
		node = node.nextSibling();
	}

	return true;
}

/*****************************************************************************
 * VCPropertiesEditor Initialization
 *****************************************************************************/

VCPropertiesEditor::VCPropertiesEditor(QWidget* parent,
	const VCProperties& properties) : QDialog(parent)
{
	setupUi(this);

	connect(m_chooseFadeInputButton, SIGNAL(clicked()),
		this, SLOT(slotChooseFadeInputClicked()));
	connect(m_chooseHoldInputButton, SIGNAL(clicked()),
		this, SLOT(slotChooseHoldInputClicked()));

	m_properties = properties;

	/* Grid */
	m_gridGroup->setChecked(properties.isGridEnabled());
	m_gridXSpin->setValue(properties.gridX());
	m_gridYSpin->setValue(properties.gridY());

	/* Keyboard */
	m_grabKeyboardCheck->setChecked(properties.isGrabKeyboard());
	m_keyRepeatOffCheck->setChecked(properties.isKeyRepeatOff());

	/* Slider limits */
	m_fadeLowSpin->setValue(properties.fadeLowLimit());
	m_fadeHighSpin->setValue(properties.fadeHighLimit());
	m_holdLowSpin->setValue(properties.holdLowLimit());
	m_holdHighSpin->setValue(properties.holdHighLimit());

	/* External input */
	updateFadeInputSource();
	updateHoldInputSource();
}

VCPropertiesEditor::~VCPropertiesEditor()
{
}

/*****************************************************************************
 * Layout page
 *****************************************************************************/

void VCPropertiesEditor::slotGrabKeyboardClicked()
{
	m_properties.setGrabKeyboard(m_grabKeyboardCheck->isChecked());
}

void VCPropertiesEditor::slotKeyRepeatOffClicked()
{
	m_properties.setKeyRepeatOff(m_keyRepeatOffCheck->isChecked());
}

void VCPropertiesEditor::slotGridClicked()
{
	m_properties.setGridEnabled(m_gridGroup->isChecked());
}

void VCPropertiesEditor::slotGridXChanged(int value)
{
	m_properties.setGridX(value);
}

void VCPropertiesEditor::slotGridYChanged(int value)
{
	m_properties.setGridY(value);
}

/*****************************************************************************
 * Sliders page
 *****************************************************************************/

void VCPropertiesEditor::slotFadeLimitsChanged()
{
	m_properties.setFadeLimits(m_fadeLowSpin->value(),
				   m_fadeHighSpin->value());
}

void VCPropertiesEditor::slotHoldLimitsChanged()
{
	m_properties.setHoldLimits(m_holdLowSpin->value(),
				   m_holdHighSpin->value());
}

void VCPropertiesEditor::slotChooseFadeInputClicked()
{
	SelectInputChannel sic(this);
	if (sic.exec() == QDialog::Accepted)
	{
		m_properties.setFadeInputSource(sic.universe(),
						sic.channel());
		updateFadeInputSource();
	}
}

void VCPropertiesEditor::slotChooseHoldInputClicked()
{
	SelectInputChannel sic(this);
	if (sic.exec() == QDialog::Accepted)
	{
		m_properties.setHoldInputSource(sic.universe(),
						sic.channel());
		updateHoldInputSource();
	}
}

void VCPropertiesEditor::updateFadeInputSource()
{
	QLCInputProfile* profile;
	InputPatch* patch;
	QString uniName;
	QString chName;

	if (m_properties.fadeInputUniverse() == KInputUniverseInvalid ||
	    m_properties.fadeInputChannel() == KInputChannelInvalid)
	{
		/* Nothing selected for input universe and/or channel */
		uniName = KInputNone;
		chName = KInputNone;

		/* Display the gathered information */
		m_fadeInputUniverseEdit->setText(uniName);
		m_fadeInputChannelEdit->setText(chName);
		
		return;
	}

	patch = _app->inputMap()->patch(m_properties.fadeInputUniverse());
	if (patch == NULL || patch->plugin() == NULL)
	{
		/* There is no patch for the given universe */
		uniName = KInputNone;
		chName = KInputNone;

		/* Display the gathered information */
		m_fadeInputUniverseEdit->setText(uniName);
		m_fadeInputChannelEdit->setText(chName);
		
		return;
	}

	profile = patch->profile();
	if (profile == NULL)
	{
		/* There is no profile. Display plugin name and channel number.
		   Boring. */
		uniName = patch->plugin()->name();
		chName = tr("%1: Unknown")
				.arg(m_properties.fadeInputChannel() + 1);
	}
	else
	{
		QLCInputChannel* ich;
		QString name;

		/* Display profile name for universe */
		uniName = QString("%1: %2")
			.arg(m_properties.fadeInputUniverse() + 1)
			.arg(profile->name());

		/* User can input the channel number by hand, so put something
		   rational to the channel name in those cases as well. */
		ich = profile->channel(m_properties.fadeInputChannel());
		if (ich != NULL)
			name = ich->name();
		else
			name = tr("Unknown");

		/* Display channel name */
		chName = QString("%1: %2")
			.arg(m_properties.fadeInputChannel() + 1).arg(name);
	}

	/* Display the gathered information */
	m_fadeInputUniverseEdit->setText(uniName);
	m_fadeInputChannelEdit->setText(chName);
}

void VCPropertiesEditor::updateHoldInputSource()
{
	QLCInputProfile* profile;
	InputPatch* patch;
	QString uniName;
	QString chName;

	if (m_properties.holdInputUniverse() == KInputUniverseInvalid ||
	    m_properties.holdInputChannel() == KInputChannelInvalid)
	{
		/* Nothing selected for input universe and/or channel */
		uniName = KInputNone;
		chName = KInputNone;

		/* Display the gathered information */
		m_holdInputUniverseEdit->setText(uniName);
		m_holdInputChannelEdit->setText(chName);
		
		return;
	}

	patch = _app->inputMap()->patch(m_properties.holdInputUniverse());
	if (patch == NULL || patch->plugin() == NULL)
	{
		/* There is no patch for the given universe */
		uniName = KInputNone;
		chName = KInputNone;

		/* Display the gathered information */
		m_holdInputUniverseEdit->setText(uniName);
		m_holdInputChannelEdit->setText(chName);
		
		return;
	}

	profile = patch->profile();
	if (profile == NULL)
	{
		/* There is no profile. Display plugin name and channel number.
		   Boring. */
		uniName = patch->plugin()->name();
		chName = tr("%1: Unknown")
				.arg(m_properties.holdInputChannel() + 1);
	}
	else
	{
		QLCInputChannel* ich;
		QString name;

		/* Display profile name for universe */
		uniName = QString("%1: %2")
				.arg(m_properties.holdInputUniverse() + 1)
				.arg(profile->name());
		/* User can input the channel number by hand, so put something
		   rational to the channel name in those cases as well. */
		ich = profile->channel(m_properties.holdInputChannel());
		if (ich != NULL)
			name = ich->name();
		else
			name = tr("Unknown");

		/* Display channel name */
		chName = QString("%1: %2")
			.arg(m_properties.holdInputChannel() + 1).arg(name);
	}

	/* Display the gathered information */
	m_holdInputUniverseEdit->setText(uniName);
	m_holdInputChannelEdit->setText(chName);
}
