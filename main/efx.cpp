/*
  Q Light Controller
  efx.cpp

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

#include <QApplication>
#include <QByteArray>
#include <QPolygon>
#include <QDebug>
#include <QList>
#include <QtXml>

#include <math.h>

#include "common/qlcfixturemode.h"
#include "common/qlcfixturedef.h"
#include "common/qlcchannel.h"
#include "common/qlcfile.h"

#include "mastertimer.h"
#include "outputmap.h"
#include "fixture.h"
#include "scene.h"
#include "doc.h"
#include "efx.h"
#include "bus.h"

/* Supported EFX algorithms */
static const char* KCircleAlgorithmName    ( "Circle" );
static const char* KEightAlgorithmName     ( "Eight" );
static const char* KLineAlgorithmName      ( "Line" );
static const char* KDiamondAlgorithmName   ( "Diamond" );
static const char* KTriangleAlgorithmName  ( "Triangle" );
static const char* KLissajousAlgorithmName ( "Lissajous" );

/*****************************************************************************
 * Initialization
 *****************************************************************************/

EFX::EFX(QObject* parent) : Function(parent)
{
	pointFunc = NULL;

	m_width = 127;
	m_height = 127;
	m_xOffset = 127;
	m_yOffset = 127;
	m_rotation = 0;

	m_xFrequency = 2;
	m_yFrequency = 3;
	m_xPhase = M_PI / 2.0;
	m_yPhase = 0;

	m_propagationMode = Parallel;

	m_startSceneID = KNoID;
	m_startSceneEnabled = false;

	m_stopSceneID = KNoID;
	m_stopSceneEnabled = false;

	m_algorithm = KCircleAlgorithmName;

	m_stepSize = 0;

	setName(tr("New EFX"));

	/* Set Default Fade as the speed bus */
	setBus(Bus::defaultFade());
	connect(Bus::instance(), SIGNAL(valueChanged(quint32,quint32)),
		this, SLOT(slotBusValueChanged(quint32,quint32)));

	Doc* doc = qobject_cast <Doc*> (parent);
	if (doc != NULL)
	{
		/* Listen to function removals so that they can be removed from
		   this efx as well. Parent might not always be Doc, but an
		   editor dialog, for example. Such efx's cannot be run,
		   though. */
		connect(doc, SIGNAL(functionRemoved(t_function_id)),
			this, SLOT(slotFunctionRemoved(t_function_id)));
	}
}

EFX::~EFX()
{
	while (m_fixtures.isEmpty() == false)
		delete m_fixtures.takeFirst();
}

/*****************************************************************************
 * Function type
 *****************************************************************************/

Function::Type EFX::type() const
{
	return Function::EFX;
}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* EFX::createCopy(Doc* doc)
{
	Q_ASSERT(doc != NULL);

	Function* copy = new EFX(doc);
	Q_ASSERT(copy != NULL);
	if (copy->copyFrom(this) == false)
	{
		delete copy;
		copy = NULL;
	}
	else if (doc->addFunction(copy) == false)
	{
		delete copy;
		copy = NULL;
	}
	else
	{
		copy->setName(tr("Copy of %1").arg(name()));
	}

	return copy;
}

bool EFX::copyFrom(const Function* function)
{
	const EFX* efx = qobject_cast<const EFX*> (function);
	if (efx == NULL)
		return false;

	while (m_fixtures.isEmpty() == false)
		delete m_fixtures.takeFirst();

	QListIterator <EFXFixture*> it(efx->m_fixtures);
	while (it.hasNext() == true)
	{
		EFXFixture* ef = new EFXFixture(this);
		ef->copyFrom(it.next());
		m_fixtures.append(ef);
	}

	m_propagationMode = efx->m_propagationMode;

	m_width = efx->m_width;
	m_height = efx->m_height;
	m_xOffset = efx->m_xOffset;
	m_yOffset = efx->m_yOffset;
	m_rotation = efx->m_rotation;

	m_xFrequency = efx->m_xFrequency;
	m_yFrequency = efx->m_yFrequency;
	m_xPhase = efx->m_xPhase;
	m_yPhase = efx->m_yPhase;

	m_startSceneID = efx->m_startSceneID;
	m_startSceneEnabled = efx->m_startSceneEnabled;

	m_stopSceneID = efx->m_stopSceneID;
	m_stopSceneEnabled = efx->m_stopSceneEnabled;

	m_algorithm = efx->m_algorithm;

	bool result = Function::copyFrom(function);

	emit changed(m_id);

	return result;
}

/*****************************************************************************
 * Preview
 *****************************************************************************/

bool EFX::preview(QPolygon* polygon)
{
	if (polygon == NULL)
		return false;

	bool retval = true;
	int stepCount = 128;
	int step = 0;
	qreal stepSize = (qreal)(1) / ((qreal)(stepCount) / (M_PI * 2.0));

	qreal i = 0;
	qreal x = 0;
	qreal y = 0;

	/* Resize the array to contain stepCount points */
	polygon->resize(stepCount);

	/* Since algorithm is identified by a string, we don't want to do N
	   string comparisons on each for loop increment. So, it's a bit faster
	   to check the algorithm only once and then do the looping. */
	if (m_algorithm == KCircleAlgorithmName)
	{
		/* Draw a preview of a circle */
		for (i = 0; i < (M_PI * 2.0); i += stepSize)
		{
			circlePoint(this, i, &x, &y);
			polygon->setPoint(step++, int(x), int(y));
		}
	}
	else if (m_algorithm == KEightAlgorithmName)
	{
		/* Draw a preview of a eight */
		for (i = 0; i < (M_PI * 2.0); i += stepSize)
		{
			eightPoint(this, i, &x, &y);
			polygon->setPoint(step++, int(x), int(y));
		}
	}
	else if (m_algorithm == KLineAlgorithmName)
	{
		/* Draw a preview of a line */
		for (i = 0; i < (M_PI * 2.0); i += stepSize)
		{
			linePoint(this, i, &x, &y);
			polygon->setPoint(step++, int(x), int(y));
		}
	}
	else if (m_algorithm == KDiamondAlgorithmName)
	{
		/* Draw a preview of a diamond */
		for (i = 0; i < (M_PI * 2.0); i += stepSize)
		{
			diamondPoint(this, i, &x, &y);
			polygon->setPoint(step++, int(x), int(y));
		}
	}
	else if (m_algorithm == KTriangleAlgorithmName)
	{
		/* Draw a preview of a triangle */
		for (i = 0; i < (M_PI * 2.0); i += stepSize)
		{
			trianglePoint(this, i, &x, &y);
			polygon->setPoint(step++, int(x), int(y));
		}
	}
	else if (m_algorithm == KLissajousAlgorithmName)
	{
		/* Draw a preview of a lissajous */
		for (i = 0; i < (M_PI * 2.0); i += stepSize)
		{
			lissajousPoint(this, i, &x, &y);
			polygon->setPoint(step++, int(x), int(y));
		}
	}
	else
	{
		polygon->resize(0);
		retval = false;
	}

	return retval;
}

/*****************************************************************************
 * Algorithm
 *****************************************************************************/

QStringList EFX::algorithmList()
{
	QStringList list;
	list.append(KCircleAlgorithmName);
	list.append(KEightAlgorithmName);
	list.append(KLineAlgorithmName);
	list.append(KDiamondAlgorithmName);
	/* list.append(KTriangleAlgorithmName); */
	list.append(KLissajousAlgorithmName);

	return list;
}

QString EFX::algorithm() const
{
	return m_algorithm;
}

void EFX::setAlgorithm(const QString& algorithm)
{
	if (algorithmList().contains(algorithm) == true)
		m_algorithm = QString(algorithm);
	else
		m_algorithm = KCircleAlgorithmName;

	emit changed(m_id);
}

/*****************************************************************************
 * Width
 *****************************************************************************/

void EFX::setWidth(int width)
{
	m_width = static_cast<double> (CLAMP(width, 0, 127));
	emit changed(m_id);
}

int EFX::width() const
{
	return static_cast<int> (m_width);
}

/*****************************************************************************
 * Height
 *****************************************************************************/

void EFX::setHeight(int height)
{
	m_height = static_cast<double> (CLAMP(height, 0, 127));
	emit changed(m_id);
}

int EFX::height() const
{
	return static_cast<int> (m_height);
}

/*****************************************************************************
 * Rotation
 *****************************************************************************/

void EFX::setRotation(int rot)
{
	m_rotation = static_cast<int> (CLAMP(rot, 0, 359));
	emit changed(m_id);
}

int EFX::rotation() const
{
	return static_cast<int> (m_rotation);
}

/*****************************************************************************
 * Offset
 *****************************************************************************/

void EFX::setXOffset(int offset)
{
	m_xOffset = static_cast<double> (CLAMP(offset, 0, 255));
	emit changed(m_id);
}

int EFX::xOffset() const
{
	return static_cast<int> (m_xOffset);
}

void EFX::setYOffset(int offset)
{
	m_yOffset = static_cast<double> (CLAMP(offset, 0, 255));
	emit changed(m_id);
}

int EFX::yOffset() const
{
	return static_cast<int> (m_yOffset);
}

/*****************************************************************************
 * Frequency
 *****************************************************************************/

void EFX::setXFrequency(int freq)
{
	m_xFrequency = static_cast<qreal> (CLAMP(freq, 0, 5));
	emit changed(m_id);
}

int EFX::xFrequency() const
{
	return static_cast<int> (m_xFrequency);
}

void EFX::setYFrequency(int freq)
{
	m_yFrequency = static_cast<qreal> (CLAMP(freq, 0, 5));
	emit changed(m_id);
}

int EFX::yFrequency() const
{
	return static_cast<int> (m_yFrequency);
}

bool EFX::isFrequencyEnabled()
{
	if (m_algorithm == KLissajousAlgorithmName)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*****************************************************************************
 * Phase
 *****************************************************************************/

void EFX::setXPhase(int phase)
{
	m_xPhase = static_cast<qreal> (CLAMP(phase, 0, 359)) * M_PI / 180.0;
	emit changed(m_id);
}

int EFX::xPhase() const
{
	return static_cast<int> (floor((m_xPhase * 180.0 / M_PI) + 0.5));
}

void EFX::setYPhase(int phase)
{
	m_yPhase = static_cast<qreal> (CLAMP(phase, 0, 359)) * M_PI / 180.0;
	emit changed(m_id);
}

int EFX::yPhase() const
{
	return static_cast<int> (floor((m_yPhase * 180.0 / M_PI) + 0.5));
}

bool EFX::isPhaseEnabled() const
{
	if (m_algorithm == KLissajousAlgorithmName)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*****************************************************************************
 * Fixtures
 *****************************************************************************/

bool EFX::addFixture(EFXFixture* ef)
{
	Q_ASSERT(ef != NULL);

	/* Search for an existing fixture with the same ID to prevent multiple
	   entries of the same fixture. */
	QListIterator <EFXFixture*> it(m_fixtures);
	while (it.hasNext() == true)
	{
		/* Found the same fixture. Don't add the new one. */
		if (it.next()->fixture() == ef->fixture())
			return false;
	}

	/* Put the EFXFixture object into our list */
	m_fixtures.append(ef);

	emit changed(m_id);

	return true;
}

bool EFX::removeFixture(EFXFixture* ef)
{
	Q_ASSERT(ef != NULL);

	if (m_fixtures.removeAll(ef) > 0)
	{
		emit changed(m_id);
		return true;
	}
	else
	{
		return false;
	}
}

bool EFX::raiseFixture(EFXFixture* ef)
{
	Q_ASSERT(ef != NULL);

	int index = m_fixtures.indexOf(ef);
	if (index > 0)
	{
		m_fixtures.move(index, index - 1);
		emit changed(m_id);
		return true;
	}
	else
	{
		return false;
	}
}

bool EFX::lowerFixture(EFXFixture* ef)
{
	int index = m_fixtures.indexOf(ef);
	if (index < (m_fixtures.count() - 1))
	{
		m_fixtures.move(index, index + 1);
		emit changed(m_id);
		return true;
	}
	else
	{
		return false;
	}
}

void EFX::slotFixtureRemoved(t_fixture_id fxi_id)
{
	/* Remove the destroyed fixture from our list */
	QMutableListIterator <EFXFixture*> it(m_fixtures);
	while (it.hasNext() == true)
	{
		it.next();

		if (it.value()->fixture() == fxi_id)
		{
			delete it.value();
			it.remove();
			break;
		}
	}
}

/*****************************************************************************
 * Fixture propagation mode
 *****************************************************************************/

void EFX::setPropagationMode(PropagationMode mode)
{
	m_propagationMode = mode;
	emit changed(m_id);
}

QString EFX::propagationModeToString(PropagationMode mode)
{
	if (mode == Serial)
		return QString(KXMLQLCEFXPropagationModeSerial);
	else
		return QString(KXMLQLCEFXPropagationModeParallel);
}

EFX::PropagationMode EFX::stringToPropagationMode(QString str)
{
	if (str == QString(KXMLQLCEFXPropagationModeSerial))
		return Serial;
	else
		return Parallel;
}

/*****************************************************************************
 * Start & Stop scenes
 *****************************************************************************/

void EFX::setStartScene(t_function_id scene)
{
	if (scene > KNoID && scene < KFunctionArraySize)
	{
		m_startSceneID = scene;
	}
	else
	{
		m_startSceneID = Function::invalidId();
		m_startSceneEnabled = false;
	}

	emit changed(m_id);
}

t_function_id EFX::startScene() const
{
	return m_startSceneID;
}

void EFX::setStartSceneEnabled(bool set)
{
	m_startSceneEnabled = set;
	emit changed(m_id);
}

bool EFX::startSceneEnabled() const
{
	return m_startSceneEnabled;
}

void EFX::setStopScene(t_function_id scene)
{
	if (scene > KNoID && scene < KFunctionArraySize)
	{
		m_stopSceneID = scene;
	}
	else
	{
		m_stopSceneID = Function::invalidId();
		m_stopSceneEnabled = false;
	}

	emit changed(m_id);
}

t_function_id EFX::stopScene() const
{
	return m_stopSceneID;
}

void EFX::setStopSceneEnabled(bool set)
{
	m_stopSceneEnabled = set;
	emit changed(m_id);
}

bool EFX::stopSceneEnabled() const
{
	return m_stopSceneEnabled;
}

void EFX::slotFunctionRemoved(t_function_id id)
{
	if (id == m_startSceneID)
	{
		m_startSceneID = KNoID;
		m_startSceneEnabled = false;
		emit changed(m_id);
	}

	/* No "else" because the same function might be both start & stop */
	if (id == m_stopSceneID)
	{
		m_stopSceneID = KNoID;
		m_stopSceneEnabled = false;
		emit changed(m_id);
	}
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool EFX::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
	QDomElement root;
	QDomElement tag;
	QDomElement subtag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(wksp_root != NULL);

	/* Function tag */
	root = doc->createElement(KXMLQLCFunction);
	wksp_root->appendChild(root);

	root.setAttribute(KXMLQLCFunctionID, id());
	root.setAttribute(KXMLQLCFunctionType, Function::typeToString(type()));
	root.setAttribute(KXMLQLCFunctionName, name());

	/* Fixtures */
	QListIterator <EFXFixture*> it(m_fixtures);
	while (it.hasNext() == true)
		it.next()->saveXML(doc, &root);

	/* Propagation mode */
	tag = doc->createElement(KXMLQLCEFXPropagationMode);
	root.appendChild(tag);
	text = doc->createTextNode(propagationModeToString(m_propagationMode));
	tag.appendChild(text);

	/* Speed bus */
	tag = doc->createElement(KXMLQLCBus);
	root.appendChild(tag);
	tag.setAttribute(KXMLQLCBusRole, KXMLQLCBusFade);
	str.setNum(busID());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Direction */
	tag = doc->createElement(KXMLQLCFunctionDirection);
	root.appendChild(tag);
	text = doc->createTextNode(Function::directionToString(m_direction));
	tag.appendChild(text);

	/* Run order */
	tag = doc->createElement(KXMLQLCFunctionRunOrder);
	root.appendChild(tag);
	text = doc->createTextNode(Function::runOrderToString(m_runOrder));
	tag.appendChild(text);

	/* Algorithm */
	tag = doc->createElement(KXMLQLCEFXAlgorithm);
	root.appendChild(tag);
	text = doc->createTextNode(algorithm());
	tag.appendChild(text);

	/* Width */
	tag = doc->createElement(KXMLQLCEFXWidth);
	root.appendChild(tag);
	str.setNum(width());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Height */
	tag = doc->createElement(KXMLQLCEFXHeight);
	root.appendChild(tag);
	str.setNum(height());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Rotation */
	tag = doc->createElement(KXMLQLCEFXRotation);
	root.appendChild(tag);
	str.setNum(rotation());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Start function */
	tag = doc->createElement(KXMLQLCEFXStartScene);
	root.appendChild(tag);
	str.setNum(startScene());
	text = doc->createTextNode(str);
	tag.appendChild(text);
	if (startSceneEnabled() == true)
		tag.setAttribute(KXMLQLCFunctionEnabled, KXMLQLCTrue);
	else
		tag.setAttribute(KXMLQLCFunctionEnabled, KXMLQLCFalse);

	/* Stop function */
	tag = doc->createElement(KXMLQLCEFXStopScene);
	root.appendChild(tag);
	str.setNum(stopScene());
	text = doc->createTextNode(str);
	tag.appendChild(text);
	if (stopSceneEnabled() == true)
		tag.setAttribute(KXMLQLCFunctionEnabled, KXMLQLCTrue);
	else
		tag.setAttribute(KXMLQLCFunctionEnabled, KXMLQLCFalse);

	/********************************************
	 * X-Axis 
	 ********************************************/
	tag = doc->createElement(KXMLQLCEFXAxis);
	root.appendChild(tag);
	tag.setAttribute(KXMLQLCFunctionName, KXMLQLCEFXX);

	/* Offset */
	subtag = doc->createElement(KXMLQLCEFXOffset);
	tag.appendChild(subtag);
	str.setNum(xOffset());
	text = doc->createTextNode(str);
	subtag.appendChild(text);

        /* Frequency */
	subtag = doc->createElement(KXMLQLCEFXFrequency);
	tag.appendChild(subtag);
	str.setNum(xFrequency());
	text = doc->createTextNode(str);
	subtag.appendChild(text);

        /* Phase */
	subtag = doc->createElement(KXMLQLCEFXPhase);
	tag.appendChild(subtag);
	str.setNum(xPhase());
	text = doc->createTextNode(str);
	subtag.appendChild(text);

	/********************************************
	 * Y-Axis 
	 ********************************************/
	tag = doc->createElement(KXMLQLCEFXAxis);
	root.appendChild(tag);
	tag.setAttribute(KXMLQLCFunctionName, KXMLQLCEFXY);

	/* Offset */
	subtag = doc->createElement(KXMLQLCEFXOffset);
	tag.appendChild(subtag);
	str.setNum(yOffset());
	text = doc->createTextNode(str);
	subtag.appendChild(text);

	/* Frequency */
	subtag = doc->createElement(KXMLQLCEFXFrequency);
	tag.appendChild(subtag);
	str.setNum(yFrequency());
	text = doc->createTextNode(str);
	subtag.appendChild(text);

        /* Phase */
	subtag = doc->createElement(KXMLQLCEFXPhase);
	tag.appendChild(subtag);
	str.setNum(yPhase());
	text = doc->createTextNode(str);
	subtag.appendChild(text);

	return true;
}

bool EFX::loadXML(const QDomElement* root)
{
	QString str;
	QDomNode node;
	QDomElement tag;

	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCFunction)
	{
		qWarning() << "Function node not found!";
		return false;
	}

	if (root->attribute(KXMLQLCFunctionType) != typeToString(Function::EFX))
	{
		qWarning("Function is not an EFX!");
		return false;
	}

	/* Load EFX contents */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();

		if (tag.tagName() == KXMLQLCBus)
		{
			/* Bus */
			str = tag.attribute(KXMLQLCBusRole);
			setBus(tag.text().toUInt());
		}
		else if (tag.tagName() == KXMLQLCEFXFixture)
		{
			EFXFixture* ef = new EFXFixture(this);
			ef->loadXML(&tag);
			if (ef->fixture() != KNoID)
			{
				if (addFixture(ef) == false)
					delete ef;
			}
		}
		else if (tag.tagName() == KXMLQLCEFXPropagationMode)
		{
			/* Propagation mode */
			setPropagationMode(stringToPropagationMode(tag.text()));
		}
		else if (tag.tagName() == KXMLQLCEFXAlgorithm)
		{
			/* Algorithm */
			setAlgorithm(tag.text());
		}
		else if (tag.tagName() == KXMLQLCFunctionDirection)
		{
			/* Direction */
			setDirection(Function::stringToDirection(tag.text()));
		}
		else if (tag.tagName() == KXMLQLCFunctionRunOrder)
		{
			/* Run Order */
			setRunOrder(Function::stringToRunOrder(tag.text()));
		}
		else if (tag.tagName() == KXMLQLCEFXWidth)
		{
			/* Width */
			setWidth(tag.text().toInt());
		}
		else if (tag.tagName() == KXMLQLCEFXHeight)
		{
			/* Height */
			setHeight(tag.text().toInt());
		}
		else if (tag.tagName() == KXMLQLCEFXRotation)
		{
			/* Rotation */
			setRotation(tag.text().toInt());
		}
		else if (tag.tagName() == KXMLQLCEFXStartScene)
		{
			/* Start scene */
			setStartScene(tag.text().toInt());

			if (tag.attribute(KXMLQLCFunctionEnabled) ==
			    KXMLQLCTrue)
				setStartSceneEnabled(true);
			else
				setStartSceneEnabled(false);
		}
		else if (tag.tagName() == KXMLQLCEFXStopScene)
		{
			/* Stop scene */
			setStopScene(tag.text().toInt());

			if (tag.attribute(KXMLQLCFunctionEnabled) ==
			    KXMLQLCTrue)
				setStopSceneEnabled(true);
			else
				setStopSceneEnabled(false);
		}
		else if (tag.tagName() == KXMLQLCEFXAxis)
		{
			/* Axes */
			loadXMLAxis(&tag);
		}
		else
		{
			qWarning() << "Unknown EFX tag:" << tag.tagName();
		}

		node = node.nextSibling();
	}

	return true;
}

bool EFX::loadXMLAxis(const QDomElement* root)
{
	int frequency = 0;
	int offset = 0;
	int phase = 0;
	QString axis;

	QDomNode node;
	QDomElement tag;

	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCEFXAxis)
	{
		qWarning() << "EFX axis node not found!";
		return false;
	}

	/* Get the axis name */
	axis = root->attribute(KXMLQLCFunctionName);

	/* Load axis contents */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();

		if (tag.tagName() == KXMLQLCEFXOffset)
		{
			offset = tag.text().toInt();
		}
		else if (tag.tagName() == KXMLQLCEFXFrequency)
		{
			frequency = tag.text().toInt();
		}
		else if (tag.tagName() == KXMLQLCEFXPhase)
		{
			phase = tag.text().toInt();
		}
		else
		{
			qWarning() << "Unknown EFX axis tag: "
				   << tag.tagName();
		}

		node = node.nextSibling();
	}

	if (axis == KXMLQLCEFXY)
	{
		setYOffset(offset);
		setYFrequency(frequency);
		setYPhase(phase);

		return true;
	}
	else if (axis == KXMLQLCEFXX)
	{
		setXOffset(offset);
		setXFrequency(frequency);
		setXPhase(phase);

		return true;
	}
	else
	{
		qWarning() << "Unknown EFX axis:" << axis;

		return false;
	}
}

/*****************************************************************************
 * Bus
 *****************************************************************************/

void EFX::slotBusValueChanged(quint32 id, quint32 value)
{
	if (id != m_busID)
		return;

	/* Size of one step */
	m_stepSize = qreal(1) / (qreal(value) / qreal(M_PI * 2));
}

/*****************************************************************************
 * Point calculation functions
 *****************************************************************************/

void EFX::circlePoint(EFX* efx, qreal iterator, qreal* x, qreal* y)
{
	*x = cos(iterator + M_PI_2);
	*y = cos(iterator);

	rotateAndScale(x, y, efx->m_width, efx->m_height,
		       efx->m_xOffset, efx->m_yOffset, efx->m_rotation);
}

void EFX::eightPoint(EFX* efx, qreal iterator, qreal* x, qreal* y)
{
	*x = cos((iterator * 2) + M_PI_2);
	*y = cos(iterator);

	rotateAndScale(x, y, efx->m_width, efx->m_height,
		       efx->m_xOffset, efx->m_yOffset, efx->m_rotation);
}

void EFX::linePoint(EFX* efx, qreal iterator, qreal* x, qreal* y)
{
	/* TODO: It's a simple line, I don't think we need cos() :) */
	*x = cos(iterator);
	*y = cos(iterator);

	rotateAndScale(x, y, efx->m_width, efx->m_height,
		       efx->m_xOffset, efx->m_yOffset, efx->m_rotation);
}

void EFX::trianglePoint(EFX* efx, qreal iterator, qreal* x, qreal* y)
{
	/* TODO !!! */
	*x = cos(iterator);
	*y = sin(iterator);

	rotateAndScale(x, y, efx->m_width, efx->m_height,
		       efx->m_xOffset, efx->m_yOffset, efx->m_rotation);
}

void EFX::diamondPoint(EFX* efx, qreal iterator, qreal* x, qreal* y)
{
	*x = pow(cos(iterator - M_PI_2), 3);
	*y = pow(cos(iterator), 3);

	rotateAndScale(x, y, efx->m_width, efx->m_height,
		       efx->m_xOffset, efx->m_yOffset, efx->m_rotation);
}

void EFX::lissajousPoint(EFX* efx, qreal iterator, qreal* x, qreal* y)
{
	*x = cos((efx->m_xFrequency * iterator) - efx->m_xPhase);
	*y = cos((efx->m_yFrequency * iterator) - efx->m_yPhase);

	rotateAndScale(x, y, efx->m_width, efx->m_height,
		       efx->m_xOffset, efx->m_yOffset, efx->m_rotation);
}

void EFX::rotateAndScale(qreal* x, qreal* y, qreal w, qreal h,
			 qreal xOff, qreal yOff, qreal rotation)
{
	qreal xx;
	qreal yy;
	qreal r;

	xx = *x;
	yy = *y;

	r = M_PI/180 * rotation;
	*x = xOff + (xx * cos(r) + yy  * sin(r)) * w;
	*y = yOff + (-xx * sin(r) + yy * cos(r))  * h;
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void EFX::arm()
{
	class Scene* startScene = NULL;
	class Scene* stopScene = NULL;
	int serialNumber = 0;

	Doc* doc = qobject_cast <Doc*> (parent());
	Q_ASSERT(doc != NULL);

	/* Initialization scene */
	if (m_startSceneID != KNoID && m_startSceneEnabled == true)
		startScene = static_cast <class Scene*>
			(doc->function(m_startSceneID));

	/* De-initialization scene */
	if (m_stopSceneID != KNoID && m_stopSceneEnabled == true)
		stopScene = static_cast <class Scene*>
			(doc->function(m_stopSceneID));

	QListIterator <EFXFixture*> it(m_fixtures);
	while (it.hasNext() == true)
	{
		EFXFixture* ef = it.next();
		Q_ASSERT(ef != NULL);

		ef->setSerialNumber(serialNumber++);
		ef->setStartScene(startScene);
		ef->setStopScene(stopScene);

		/* If fxi == NULL, the fixture has been destroyed */
		Fixture* fxi = doc->fixture(ef->fixture());
		if (fxi == NULL)
			continue;

		/* If this fixture has no mode, it's a generic dimmer that
		   can't do pan&tilt anyway. */
		const QLCFixtureMode* mode = fxi->fixtureMode();
		if (mode == NULL)
			continue;

		/* Find exact channel numbers for MSB/LSB pan and tilt */
		for (t_channel i = 0; i < mode->channels().size(); i++)
		{
			QLCChannel* ch = mode->channel(i);
			Q_ASSERT(ch != NULL);

			if (ch->group() == KQLCChannelGroupPan)
			{
				if (ch->controlByte() == 0)
				{
					ef->setMsbPanChannel(
						fxi->universeAddress() + i);
				}
				else if (ch->controlByte() == 1)
				{
					ef->setLsbPanChannel(
						fxi->universeAddress() + i);
				}
			}
			else if (ch->group() == KQLCChannelGroupTilt)
			{
				if (ch->controlByte() == 0)
				{
					ef->setMsbTiltChannel(
						fxi->universeAddress() + i);
				}
				else if (ch->controlByte() == 1)
				{
					ef->setLsbTiltChannel(
						fxi->universeAddress() + i);
				}
			}
		}
	}

	/* Choose a point calculation function depending on the algorithm */
	if (m_algorithm == KCircleAlgorithmName)
		pointFunc = circlePoint;
	else if (m_algorithm == KEightAlgorithmName)
		pointFunc = eightPoint;
	else if (m_algorithm == KLineAlgorithmName)
		pointFunc = linePoint;
	else if (m_algorithm == KTriangleAlgorithmName)
		pointFunc = trianglePoint;
	else if (m_algorithm == KDiamondAlgorithmName)
		pointFunc = diamondPoint;
	else if (m_algorithm == KLissajousAlgorithmName)
		pointFunc = lissajousPoint;
	else
		pointFunc = NULL;
}

void EFX::disarm()
{
	/* Just a precaution: invalidate the algorithm worker function */
	pointFunc = NULL;
}

void EFX::start(MasterTimer* timer)
{
	Q_ASSERT(timer != NULL);

	/* Set initial speed */
	slotBusValueChanged(m_busID, Bus::instance()->value(m_busID));
	Function::start(timer);
}

bool EFX::write(QByteArray* universes)
{
	int ready = 0;

	Q_ASSERT(universes != NULL);

	/* Check that a valid point function is present and there's at least
	   one fixture to control. */
	if (pointFunc == NULL || m_fixtures.isEmpty() == true)
		return false;

	QListIterator <EFXFixture*> it(m_fixtures);
	while (it.hasNext() == true)
	{
		EFXFixture* ef = it.next();
		if (ef->isReady() == false)
			ef->nextStep(universes);
		else
			ready++;
	}

	/* Check for stop condition */
	if (ready == m_fixtures.count())
		return false;
	else
		return true;
}

void EFX::stop(MasterTimer* timer)
{
	Q_ASSERT(timer != NULL);

	Function::stop(timer);

	/* Reset all fixtures */
	QListIterator <EFXFixture*> it(m_fixtures);
	while (it.hasNext() == true)
	{
		EFXFixture* ef(it.next());

		/* WARNING: Potential place for a deadlock if
		   OutputMap::claimUniverses() and ::releaseUniverses() is ever
		   changed to use a mutex.
		   
		   This runs the EFX's stop scene for Loop & PingPong modes.
		   It's not exactly the best solution, but this way the change
		   is limited only to this one place. TODO: Maybe this can be
		   done the right way when HTP is implemented...? */
		if (m_runOrder != SingleShot)
		{
			QByteArray* universes;
			universes = timer->outputMap()->claimUniverses();
			ef->stop(universes);
			timer->outputMap()->releaseUniverses();
		}

		ef->reset();
	}
}

