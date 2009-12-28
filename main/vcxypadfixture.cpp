/*
  Q Light Controller
  vcxypadfixture.cpp

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

#include <QStringList>
#include <QVariant>
#include <QString>
#include <QtXml>

#include "qlcfixturemode.h"
#include "qlcchannel.h"
#include "qlcfile.h"

#include "vcxypadfixture.h"
#include "outputmap.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCXYPadFixture::VCXYPadFixture()
{
	m_fixture = Fixture::invalidId();

	m_xMin = 0;
	m_xMax = 1;
	m_xReverse = false;

	m_yMin = 0;
	m_yMax = 1;
	m_yReverse = false;

	m_xLSB = KChannelInvalid;
	m_xMSB = KChannelInvalid;
	m_yLSB = KChannelInvalid;
	m_yMSB = KChannelInvalid;
}

VCXYPadFixture::VCXYPadFixture(const VCXYPadFixture& vc_fxi)
{
	*this = vc_fxi;
}

VCXYPadFixture::VCXYPadFixture(const QVariant& variant)
{
	if (variant.canConvert(QVariant::StringList) == true)
	{
		QStringList list(variant.toStringList());
		if (list.count() == 7)
		{
			m_fixture = list.at(0).toInt();

			m_xMin = list.at(1).toDouble();
			m_xMax = list.at(2).toDouble();
			m_xReverse = list.at(3).toInt();

			m_yMin = list.at(4).toDouble();
			m_yMax = list.at(5).toDouble();
			m_yReverse = list.at(6).toInt();
		}
		else
		{
			/* Construct empty fixture */
			*this = VCXYPadFixture();
		}
	}
	else
	{
		/* Construct empty fixture */
		*this = VCXYPadFixture();
	}

	m_xMSB = KChannelInvalid;
	m_xLSB = KChannelInvalid;

	m_yMSB = KChannelInvalid;
	m_yLSB = KChannelInvalid;
}

VCXYPadFixture::~VCXYPadFixture()
{
}
	
VCXYPadFixture& VCXYPadFixture::operator=(const VCXYPadFixture& fxi)
{
	m_fixture = fxi.m_fixture;

	m_xMin = fxi.m_xMin;
	m_xMax = fxi.m_xMax;
	m_xReverse = fxi.m_xReverse;

	m_yMin = fxi.m_yMin;
	m_yMax = fxi.m_yMax;
	m_yReverse = fxi.m_yReverse;

	m_xMSB = fxi.m_xMSB;
	m_xLSB = fxi.m_xLSB;

	m_yMSB = fxi.m_yMSB;
	m_yLSB = fxi.m_yLSB;

	return *this;
}

bool VCXYPadFixture::operator==(const VCXYPadFixture& fxi)
{
	if (m_fixture == fxi.m_fixture)
		return true;
	else
		return false;
}

VCXYPadFixture::operator QVariant() const
{
	QStringList list;

	list << QString("%1").arg(m_fixture);

	list << QString("%1").arg(m_xMin);
	list << QString("%1").arg(m_xMax);
	list << QString("%1").arg(m_xReverse);

	list << QString("%1").arg(m_yMin);
	list << QString("%1").arg(m_yMax);
	list << QString("%1").arg(m_yReverse);

	return QVariant(list);
}

/****************************************************************************
 * Fixture
 ****************************************************************************/

void VCXYPadFixture::setFixture(t_fixture_id fxi_id)
{
	m_fixture = fxi_id;
}

QString VCXYPadFixture::name() const
{
	static QString null = QString::null;
	if (m_fixture == Fixture::invalidId())
		return null;

	Fixture* fxi = _app->doc()->fixture(m_fixture);
	if (fxi != NULL)
		return fxi->name();
	else
		return null;
}

void VCXYPadFixture::arm()
{
	Fixture* fxi = NULL;

	fxi = _app->doc()->fixture(m_fixture);
	if (fxi == NULL)
	{
		m_xLSB = KChannelInvalid;
		m_xMSB = KChannelInvalid;
		m_yLSB = KChannelInvalid;
		m_yMSB = KChannelInvalid;
	}
	else
	{
		const QLCFixtureMode* mode = NULL;
		QLCChannel* ch = NULL;

		/* If this fixture has no mode, it's a generic dimmer that
		   can't do pan&tilt anyway. */
		mode = fxi->fixtureMode();
		if (mode == NULL)
		{
			m_xLSB = KChannelInvalid;
			m_xMSB = KChannelInvalid;
			m_yLSB = KChannelInvalid;
			m_yMSB = KChannelInvalid;
			
			return;
		}

		/* Find exact channel numbers for MSB/LSB pan and tilt */
		for (t_channel i = 0; i < mode->channels().size(); i++)
		{
			ch = mode->channel(i);
			Q_ASSERT(ch != NULL);

			if (ch->group() == KQLCChannelGroupPan)
			{
				if (ch->controlByte() == 0)
					m_xMSB = fxi->universeAddress() + i;
				else if (ch->controlByte() == 1)
					m_xLSB = fxi->universeAddress() + i;
			}
			else if (ch->group() == KQLCChannelGroupTilt)
			{
				if (ch->controlByte() == 0)
					m_yMSB = fxi->universeAddress() + i;
				else if (ch->controlByte() == 1)
					m_yLSB = fxi->universeAddress() + i;
			}
		}
	}
}

void VCXYPadFixture::disarm()
{
	m_xLSB = KChannelInvalid;
	m_xMSB = KChannelInvalid;
	m_yLSB = KChannelInvalid;
	m_yMSB = KChannelInvalid;
}

void VCXYPadFixture::outputDMX(double xmul, double ymul)
{
	if (m_xMSB == KChannelInvalid || m_yMSB == KChannelInvalid)
		return;

	double xMSB = ((m_xMax - m_xMin) * xmul) + m_xMin;
	double yMSB = ((m_yMax - m_yMin) * ymul) + m_yMin;

	if (m_xReverse == true)
		xMSB = m_xMax - xMSB;
	if (m_yReverse == true)
		yMSB = m_yMax - yMSB;

	_app->outputMap()->setValue(m_xMSB, t_value(xMSB * 255));
	_app->outputMap()->setValue(m_yMSB, t_value(yMSB * 255));

	if (m_xLSB != KChannelInvalid && m_yLSB != KChannelInvalid)
	{
		/* Leave only the fraction part from the value */
		double xLSB = (xMSB * 255.0) - floor(xMSB * 255.0);
		double yLSB = (yMSB * 255.0) - floor(yMSB * 255.0);

		_app->outputMap()->setValue(m_xLSB, t_value(xLSB * 255));
		_app->outputMap()->setValue(m_yLSB, t_value(yLSB * 255));
	}
}

/****************************************************************************
 * X-Axis
 ****************************************************************************/

void VCXYPadFixture::setX(double min, double max, bool reverse)
{
	m_xMin = CLAMP(min, 0.0, 1.0);
	m_xMax = CLAMP(max, 0.0, 1.0);
	m_xReverse = reverse;
}

QString VCXYPadFixture::xBrief() const
{
	if (m_xReverse == false)
		return QString("%1 - %2%").arg(m_xMin * 100).arg(m_xMax * 100);
	else
		return QString("%1 - %2%").arg(m_xMax * 100).arg(m_xMin * 100);
}

/****************************************************************************
 * Y-Axis
 ****************************************************************************/

void VCXYPadFixture::setY(double min, double max, bool reverse)
{
	m_yMin = CLAMP(min, 0.0, 1.0);
	m_yMax = CLAMP(max, 0.0, 1.0);
	m_yReverse = reverse;
}

QString VCXYPadFixture::yBrief() const
{
	if (m_yReverse == false)
		return QString("%1 - %2%").arg(m_yMin * 100).arg(m_yMax * 100);
	else
		return QString("%1 - %2%").arg(m_yMax * 100).arg(m_yMin * 100);
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool VCXYPadFixture::loadXML(const QDomElement* root)
{
	QDomNode node;
	QDomElement tag;
	QString axis;
	QString min;
	QString max;
	QString rev;

	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCVCXYPadFixture)
	{
		qWarning() << "XYPad Fixture node not found";
		return false;
	}

	/* Fixture ID */
	setFixture(root->attribute(KXMLQLCVCXYPadFixtureID).toInt());

	/* Children */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCVCXYPadFixtureAxis)
		{
			axis = tag.attribute(KXMLQLCVCXYPadFixtureAxisID);
			min = tag.attribute(KXMLQLCVCXYPadFixtureAxisLowLimit);
			max = tag.attribute(KXMLQLCVCXYPadFixtureAxisHighLimit);
			rev = tag.attribute(KXMLQLCVCXYPadFixtureAxisReverse);

			if (axis == KXMLQLCVCXYPadFixtureAxisX)
			{
				if (rev == KXMLQLCTrue)
					setX(min.toDouble(), max.toDouble(), true);
				else
					setX(min.toDouble(), max.toDouble(), false);
			}
			else if (axis == KXMLQLCVCXYPadFixtureAxisY)
			{
				if (rev == KXMLQLCTrue)
					setY(min.toDouble(), max.toDouble(), true);
				else
					setY(min.toDouble(), max.toDouble(), false);
			}
			else
			{
				qWarning() << "Unknown XYPad axis" << axis;
			}
		}
		else
		{
			qDebug() << "Unknown XY Pad tag:" << tag.tagName();
		}

		node = node.nextSibling();
	}

	return true;
}

bool VCXYPadFixture::saveXML(QDomDocument* doc, QDomElement* pad_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(pad_root != NULL);

	/* VCXYPad Fixture */
	root = doc->createElement(KXMLQLCVCXYPadFixture);
	root.setAttribute(KXMLQLCVCXYPadFixtureID,
			  QString("%1").arg(m_fixture));
	root.appendChild(text);
	pad_root->appendChild(root);

	/* X-Axis */
	tag = doc->createElement(KXMLQLCVCXYPadFixtureAxis);
	root.appendChild(tag);
	tag.setAttribute(KXMLQLCVCXYPadFixtureAxisID,
			 KXMLQLCVCXYPadFixtureAxisX);
	tag.setAttribute(KXMLQLCVCXYPadFixtureAxisLowLimit,
			 QString("%1").arg(m_xMin));
	tag.setAttribute(KXMLQLCVCXYPadFixtureAxisHighLimit,
			 QString("%1").arg(m_xMax));
	if (m_xReverse == true)
	{
		tag.setAttribute(KXMLQLCVCXYPadFixtureAxisReverse,
				 KXMLQLCTrue);
	}
	else
	{
		tag.setAttribute(KXMLQLCVCXYPadFixtureAxisReverse,
				 KXMLQLCFalse);
	}

	/* Y-Axis */
	tag = doc->createElement(KXMLQLCVCXYPadFixtureAxis);
	root.appendChild(tag);
	tag.setAttribute(KXMLQLCVCXYPadFixtureAxisID,
			 KXMLQLCVCXYPadFixtureAxisY);
	tag.setAttribute(KXMLQLCVCXYPadFixtureAxisLowLimit,
			 QString("%1").arg(m_yMin));
	tag.setAttribute(KXMLQLCVCXYPadFixtureAxisHighLimit,
			 QString("%1").arg(m_yMax));
	if (m_yReverse == true)
	{
		tag.setAttribute(KXMLQLCVCXYPadFixtureAxisReverse,
				 KXMLQLCTrue);
	}
	else
	{
		tag.setAttribute(KXMLQLCVCXYPadFixtureAxisReverse,
				 KXMLQLCFalse);
	}

	return true;
}
