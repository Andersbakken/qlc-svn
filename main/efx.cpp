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
#include <QPolygon>
#include <QDebug>
#include <QList>
#include <QtXml>

#include <math.h>

#include "common/qlcfixturemode.h"
#include "common/qlcfixturedef.h"
#include "common/qlcchannel.h"
#include "common/qlcfile.h"

#include "functionconsumer.h"
#include "eventbuffer.h"
#include "fixture.h"
#include "scene.h"
#include "app.h"
#include "doc.h"
#include "efx.h"
#include "bus.h"

extern App* _app;

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

EFX::EFX(QObject* parent) : Function(parent, Function::EFX)
{
	pointFunc = NULL;

	m_width = 127;
	m_height = 127;
	m_xOffset = 127;
	m_yOffset = 127;
	m_rotation = 0;

	m_xFrequency = 2;
	m_yFrequency = 3;
	m_xPhase = 1.5707963267;
	m_yPhase = 0;

	m_propagationMode = Parallel;

	m_runOrder = EFX::Loop;
	m_direction = EFX::Forward;

	m_startSceneID = KNoID;
	m_startSceneEnabled = false;

	m_stopSceneID = KNoID;
	m_stopSceneEnabled = false;

	m_previewPointArray = NULL;

	m_algorithm = KCircleAlgorithmName;

	m_stepSize = 0;

	m_channelData = NULL;

	/* Set Default Fade as the speed bus */
	setBus(KBusIDDefaultFade);
}

/**
 * Standard destructor
 */
EFX::~EFX()
{
}

/**
 * Copy function contents from another function
 *
 * @param efx EFX function from which to copy contents to this function
 * @param to The new parent fixture instance for this function
 */
bool EFX::copyFrom(EFX* efx)
{
	Q_ASSERT(efx != NULL);

	Function::setName(efx->name());
	Function::setBus(efx->busID());

	m_fixtures.clear();
	m_fixtures = efx->m_fixtures;
	m_propagationMode = efx->m_propagationMode;

	m_width = efx->width();
	m_height = efx->height();
	m_xOffset = efx->xOffset();
	m_yOffset = efx->yOffset();
	m_rotation = efx->rotation();

	m_xFrequency = efx->xFrequency();
	m_yFrequency = efx->yFrequency();
	m_xPhase = efx->xPhase();
	m_yPhase = efx->yPhase();

	m_runOrder = efx->runOrder();
	m_direction = efx->direction();

	m_startSceneID = efx->startScene();
	m_startSceneEnabled = efx->startSceneEnabled();

	m_stopSceneID = efx->stopScene();
	m_stopSceneEnabled = efx->stopSceneEnabled();

	m_previewPointArray = NULL;

	m_algorithm = QString(efx->algorithm());

	m_stepSize = 0;

	m_channelData = NULL;

	return true;
}

/*****************************************************************************
 * Preview
 *****************************************************************************/

/**
 * Set a pointer to a point array for updating the
 * changes when editing the function.
 *
 * @note Call this function with NULL after editing is finished!
 *
 * @param array The array to save the preview points to
 */
void EFX::setPreviewPointArray(QPolygon* array)
{
	m_previewPointArray = array;
}

/**
 * Updates the preview points (if necessary)
 *
 */
void EFX::updatePreview()
{
	if (m_previewPointArray == NULL)
		return;

	int stepCount = 128;
	int step = 0;
	float stepSize = (float)(1) / ((float)(stepCount) / (M_PI * 2.0));

	float i = 0;
	float *x = new float;
	float *y = new float;

	/* Resize the array to contain stepCount points */
	m_previewPointArray->resize(stepCount);

	if (m_algorithm == KCircleAlgorithmName)
	{
		/* Draw a preview of a circle */
		for (i = 0; i < (M_PI * 2.0); i += stepSize)
		{
			circlePoint(this, i, x, y);
			m_previewPointArray->setPoint(step++,
						      static_cast<int> (*x),
						      static_cast<int> (*y));
		}
	}
	else if (m_algorithm == KEightAlgorithmName)
	{
		/* Draw a preview of a eight */
		for (i = 0; i < (M_PI * 2.0); i += stepSize)
		{
			eightPoint(this, i, x, y);
			m_previewPointArray->setPoint(step++,
						      static_cast<int> (*x),
						      static_cast<int> (*y));
		}
	}
	else if (m_algorithm == KLineAlgorithmName)
	{
		/* Draw a preview of a line */
		for (i = 0; i < (M_PI * 2.0); i += stepSize)
		{
			linePoint(this, i, x, y);
			m_previewPointArray->setPoint(step++,
						      static_cast<int> (*x),
						      static_cast<int> (*y));
		}
	}
	else if (m_algorithm == KDiamondAlgorithmName)
	{
		/* Draw a preview of a diamond */
		for (i = 0; i < (M_PI * 2.0); i += stepSize)
		{
			diamondPoint(this, i, x, y);
			m_previewPointArray->setPoint(step++,
						      static_cast<int> (*x),
						      static_cast<int> (*y));
		}
	}
	else if (m_algorithm == KTriangleAlgorithmName)
	{
		/* Draw a preview of a triangle */
		for (i = 0; i < (M_PI * 2.0); i += stepSize)
		{
			trianglePoint(this, i, x, y);
			m_previewPointArray->setPoint(step++,
						      static_cast<int> (*x),
						      static_cast<int> (*y));
		}
	}
	else if (m_algorithm == KLissajousAlgorithmName)
	{
		/* Draw a preview of a lissajous */
		for (i = 0; i < (M_PI * 2.0); i += stepSize)
		{
			lissajousPoint(this, i, x, y);
			m_previewPointArray->setPoint(step++,
						      static_cast<int> (*x),
						      static_cast<int> (*y));
		}
	}
	else
	{
		m_previewPointArray->resize(0);
	}

	delete x;
	delete y;
}

/*****************************************************************************
 * Algorithm
 *****************************************************************************/

/**
 * Get the supported algorithms as a string list
 *
 * @note This is a static function
 *
 * @param algorithms A QStrList that shall contain the algorithms
 */
void EFX::algorithmList(QStringList& list)
{
	list.clear();
	list.append(KCircleAlgorithmName);
	list.append(KEightAlgorithmName);
	list.append(KLineAlgorithmName);
	list.append(KDiamondAlgorithmName);
	/* list.append(KTriangleAlgorithmName); */
	list.append(KLissajousAlgorithmName);
}

/**
 * Get the current algorithm
 *
 * @return Name of the current algorithm. See @ref algorithmList
 */
QString EFX::algorithm()
{
	return m_algorithm;
}

/**
 * Set the current algorithm
 *
 * @param algorithm One of the strings returned by @ref algorithmList
 */
void EFX::setAlgorithm(QString algorithm)
{
	QStringList list;
	EFX::algorithmList(list);
	if (list.contains(algorithm) == true)
		m_algorithm = QString(algorithm);
	else
		m_algorithm = KCircleAlgorithmName;

	updatePreview();
}

/*****************************************************************************
 * Width
 *****************************************************************************/

/**
 * Set the pattern width
 *
 * @param width Pattern width (0-255)
 */
void EFX::setWidth(int width)
{
	m_width = static_cast<double> (width);
	updatePreview();
}

/**
 * Get the pattern width
 *
 * @return Pattern width (0-255)
 */
int EFX::width()
{
	return static_cast<int> (m_width);
}

/*****************************************************************************
 * Height
 *****************************************************************************/

/**
 * Set the pattern height
 *
 * @param height Pattern height (0-255)
 */
void EFX::setHeight(int height)
{
	m_height = static_cast<double> (height);
	updatePreview();
}

/**
 * Get the pattern height
 *
 * @return Pattern height (0-255)
 */
int EFX::height()
{
	return static_cast<int> (m_height);
}

/*****************************************************************************
 * Rotation
 *****************************************************************************/

/**
 * Set the pattern rotation
 *
 * @param rot Pattern rotation (0-359)
 */
void EFX::setRotation(int rot)
{
	m_rotation = static_cast<int> (rot);
	updatePreview();
}

/**
 * Get the pattern rotation
 *
 * @return Pattern rotation (0-359)
 */
int EFX::rotation()
{
	return static_cast<int> (m_rotation);
}

/*****************************************************************************
 * Offset
 *****************************************************************************/

/**
 * Set the pattern offset on the X-axis
 *
 * @param offset Pattern offset (0-255; 127 is middle)
 */
void EFX::setXOffset(int offset)
{
	m_xOffset = static_cast<double> (offset);
	updatePreview();
}

/**
 * Get the pattern offset on the X-axis
 *
 * @return Pattern offset (0-255; 127 is middle)
 */
int EFX::xOffset()
{
	return static_cast<int> (m_xOffset);
}

/**
 * Set the pattern offset on the Y-axis
 *
 * @param offset Pattern offset (0-255; 127 is middle)
 */
void EFX::setYOffset(int offset)
{
	m_yOffset = static_cast<double> (offset);
	updatePreview();
}

/**
 * Get the pattern offset on the Y-axis
 *
 * @return Pattern offset (0-255; 127 is middle)
 */
int EFX::yOffset()
{
	return static_cast<int> (m_yOffset);
}

/*****************************************************************************
 * Frequency
 *****************************************************************************/

/**
 * Set the lissajous pattern frequency  on the X-axis
 *
 * @param freq Pattern frequency (0-255)
 */
void EFX::setXFrequency(int freq)
{
	m_xFrequency = freq;
	updatePreview();
}

/**
 * Get the lissajous pattern frequency on the X-axis
 *
 * @return Pattern offset (0-255)
 */
int EFX::xFrequency()
{
	return static_cast<int> (m_xFrequency);
}

/**
 * Set the lissajous pattern frequency  on the Y-axis
 *
 * @param freq Pattern frequency (0-255)
 */
void EFX::setYFrequency(int freq)
{
	m_yFrequency = freq;
	updatePreview();
}

/**
 * Get the lissajous pattern frequency on the Y-axis
 *
 * @return Pattern offset (0-255)
 */
int EFX::yFrequency()
{
	return static_cast<int> (m_yFrequency);
}

/**
 * Returns true when lissajous has been selected
 */
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

/**
 * Set the lissajous pattern phase on the X-axis
 *
 * @param phase Pattern phase (0-255)
 */
void EFX::setXPhase(int phase)
{
	m_xPhase = static_cast<float> (phase * M_PI / 180.0);
	updatePreview();
}

/**
 * Get the lissajous pattern phase on the X-axis
 *
 * @return Pattern phase (0-255)
 */
int EFX::xPhase()
{
	return static_cast<int> (m_xPhase * 180.0 / M_PI);
}

/**
 * Set the lissajous pattern phase on the Y-axis
 *
 * @param phase Pattern phase (0-255)
 */
void EFX::setYPhase(int phase)
{
	m_yPhase = static_cast<float> (phase * M_PI) / 180.0;
	updatePreview();
}

/**
 * Get the lissajous pattern phase on the Y-axis
 *
 * @return Pattern phase (0-255)
 */
int EFX::yPhase()
{
	return static_cast<int> (m_yPhase * 180.0 / M_PI);
}

/**
 * Returns true when lissajous has been selected
 */
bool EFX::isPhaseEnabled()
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

void EFX::addFixture(t_fixture_id fxi_id)
{
	if (m_fixtures.contains(fxi_id) == false)
	{
		m_fixtures.append(fxi_id);
		_app->doc()->setModified();
		_app->doc()->emitFunctionChanged(m_id);
	}
}

void EFX::removeFixture(t_fixture_id fxi_id)
{
	m_fixtures.removeAll(fxi_id);
	_app->doc()->setModified();
	_app->doc()->emitFunctionChanged(m_id);
}

void EFX::raiseFixture(t_fixture_id fxi_id)
{
	int index = m_fixtures.indexOf(fxi_id);
	if (index > 0)
	{
		m_fixtures.move(index, index - 1);
		_app->doc()->setModified();
		_app->doc()->emitFunctionChanged(m_id);
	}
}

void EFX::lowerFixture(t_fixture_id fxi_id)
{
	int index = m_fixtures.indexOf(fxi_id);
	if (index < m_fixtures.count())
	{
		_app->doc()->setModified();
		_app->doc()->emitFunctionChanged(m_id);
		m_fixtures.move(index, index + 1);
	}
}

void EFX::setPropagationMode(PropagationMode mode)
{
	m_propagationMode = mode;
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

void EFX::slotFixtureRemoved(t_fixture_id fxi_id)
{
	m_fixtures.removeAll(fxi_id);
}

/*****************************************************************************
 * Start & Stop scenes
 *****************************************************************************/

void EFX::setStartScene(t_function_id scene)
{
	if (scene >= KNoID && scene <= KFunctionArraySize)
	{
		m_startSceneID = scene;
	}
}

/**
 * Get the id for start scene
 *
 */
t_function_id EFX::startScene()
{
	return m_startSceneID;
}

/**
 * Start scene enabled
 *
 */
void EFX::setStartSceneEnabled(bool set)
{
	m_startSceneEnabled = set;
}

/**
 * Get start scene enabled status
 *
 */
bool EFX::startSceneEnabled()
{
	return m_startSceneEnabled;
}

void EFX::setStopScene(t_function_id scene)
{
	if (scene >= KNoID && scene <= KFunctionArraySize)
	{
		m_stopSceneID = scene;
	}
}

/**
 * Get the id for stop scene
 *
 */
t_function_id EFX::stopScene()
{
	return m_stopSceneID;
}

/**
 * Stop scene enabled
 *
 */
void EFX::setStopSceneEnabled(bool set)
{
	m_stopSceneEnabled = set;
}

/**
 * Get stop scene enabled status
 *
 */
bool EFX::stopSceneEnabled()
{
	return m_stopSceneEnabled;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

/**
 * Save the function's contents to an XML document
 *
 * @param doc The QDomDocument to save to
 */
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
	root.setAttribute(KXMLQLCFunctionType, Function::typeToString(m_type));
	root.setAttribute(KXMLQLCFunctionName, name());

	/* Fixtures */
	QListIterator <t_fixture_id> it(m_fixtures);
	while (it.hasNext() == true)
	{
		tag = doc->createElement(KXMLQLCEFXFixture);
		root.appendChild(tag);
		text = doc->createTextNode(str.setNum(it.next()));
		tag.appendChild(text);
	}

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

bool EFX::loadXML(QDomDocument* doc, QDomElement* root)
{
	QString str;
	QDomNode node;
	QDomElement tag;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCFunction)
	{
		qWarning() << "Function node not found!";
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
			setBus(tag.text().toInt());
		}
		else if (tag.tagName() == KXMLQLCEFXFixture)
		{
			/* Fixture */
			addFixture(tag.text().toInt());
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
			loadXMLAxis(doc, &tag);
		}
		else
		{
			qWarning() << "Unknown EFX tag:" << tag.tagName();
		}
		
		node = node.nextSibling();
	}

	return true;
}

bool EFX::loadXMLAxis(QDomDocument*, QDomElement* root)
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
	}
	else if (axis == KXMLQLCEFXX)
	{
		setXOffset(offset);
		setXFrequency(frequency);
		setXPhase(phase);
	}
	else
	{
		qWarning() << "Unknown EFX axis:" << axis;
	}
	
	return true;
}

/*****************************************************************************
 * Bus
 *****************************************************************************/

/**
 * This is called by buses for each function when the
 * bus value is changed.
 *
 * @param id ID of the bus that has changed its value
 * @param value Bus' new value
 */
void EFX::slotBusValueChanged(t_bus_id id, t_bus_value value)
{
	if (id != m_busID)
		return;

	/* Size of one step */
	m_stepSize = float(1) / (float(value) / float(M_PI * 2));
}

/*****************************************************************************
 * Point calculation functions
 *****************************************************************************/

/**
 * Calculate a single point in a circle pattern based on
 * the value of iterator (which is basically a step number)
 *
 * @note This is a static function
 *
 * @param efx The EFX function using this
 * @param iterator Step number
 * @param x Holds the calculated X coordinate
 * @param y Holds the calculated Y coordinate
 */
void EFX::circlePoint(EFX* efx, float iterator, float* x, float* y)
{
	*x = cos(iterator + M_PI_2);
	*y = cos(iterator);

	rotateAndScale(x, y, efx->m_width, efx->m_height,
		       efx->m_xOffset, efx->m_yOffset, efx->m_rotation);
}

/**
 * Calculate a single point in an eight pattern based on
 * the value of iterator (which is basically a step number)
 *
 * @note This is a static function
 *
 * @param efx The EFX function using this
 * @param iterator Step number
 * @param x Holds the calculated X coordinate
 * @param y Holds the calculated Y coordinate
 */
void EFX::eightPoint(EFX* efx, float iterator, float* x, float* y)
{
	*x = cos((iterator * 2) + M_PI_2);
	*y = cos(iterator);

	rotateAndScale(x, y, efx->m_width, efx->m_height,
		       efx->m_xOffset, efx->m_yOffset, efx->m_rotation);
}

/**
 * Calculate a single point in a line pattern based on
 * the value of iterator (which is basically a step number)
 *
 * @note This is a static function
 *
 * @param efx The EFX function using this
 * @param iterator Step number
 * @param x Holds the calculated X coordinate
 * @param y Holds the calculated Y coordinate
 */
void EFX::linePoint(EFX* efx, float iterator, float* x, float* y)
{
	/* TODO: It's a simple line, I don't think we need cos() :) */
	*x = cos(iterator);
	*y = cos(iterator);

	rotateAndScale(x, y, efx->m_width, efx->m_height,
		       efx->m_xOffset, efx->m_yOffset, efx->m_rotation);
}

/**
 * Calculate a single point in a triangle pattern based on
 * the value of iterator (which is basically a step number)
 *
 * @note This is a static function
 *
 * @param efx The EFX function using this
 * @param iterator Step number
 * @param x Holds the calculated X coordinate
 * @param y Holds the calculated Y coordinate
 */
void EFX::trianglePoint(EFX* efx, float iterator, float* x, float* y)
{
	/* TODO !!! */
	*x = cos(iterator);
	*y = sin(iterator);

	rotateAndScale(x, y, efx->m_width, efx->m_height,
		       efx->m_xOffset, efx->m_yOffset, efx->m_rotation);
}

/**
 * Calculate a single point in a diamond pattern based on
 * the value of iterator (which is basically a step number)
 *
 * @note This is a static function
 *
 * @param efx The EFX function using this
 * @param iterator Step number
 * @param x Holds the calculated X coordinate
 * @param y Holds the calculated Y coordinate
 */
void EFX::diamondPoint(EFX* efx, float iterator, float* x, float* y)
{
	*x = pow(cos(iterator - M_PI_2), 3);
	*y = pow(cos(iterator), 3);

	rotateAndScale(x, y, efx->m_width, efx->m_height,
		       efx->m_xOffset, efx->m_yOffset, efx->m_rotation);
}

/**
 * Calculate a single point in a lissajous pattern based on
 * the value of iterator (which is basically a step number)
 *
 * @note This is a static function
 *
 * @param efx The EFX function using this
 * @param iterator Step number
 * @param x Holds the calculated X coordinate
 * @param y Holds the calculated Y coordinate
 */
void EFX::lissajousPoint(EFX* efx, float iterator, float* x, float* y)
{
	*x = cos((efx->m_xFrequency * iterator) - efx->m_xPhase);
	*y = cos((efx->m_yFrequency * iterator) - efx->m_yPhase);

	rotateAndScale(x, y, efx->m_width, efx->m_height,
		       efx->m_xOffset, efx->m_yOffset, efx->m_rotation);
}

/**
 * Rotate a single point in a  pattern by
 * the value of rot, scale height and width
 *
 *
 * @param x Holds the calculated X coordinate
 * @param y Holds the calculated Y coordinate
 * @param rot Amount of rotation in degrees
 */
void EFX::rotateAndScale(float* x, float* y, float w, float h,
			 float xOff, float yOff, float rotation)
{
	float xx;
	float yy;
	float r;

	xx = *x;
	yy = *y;

	r = M_PI/180 * rotation;
	*x = xOff + (xx * cos(r) + yy  * sin(r)) * w;
	*y = yOff + (-xx * sin(r) + yy * cos(r))  * h;
}

/*****************************************************************************
 * Running
 *****************************************************************************/

/**
 * Prepare this function for running. This is called when
 * the user sets the mode to Operate. Basically allocates everything
 * that is needed to run the function.
 */
void EFX::arm()
{
	class Scene* startScene = NULL;
	class Scene* stopScene = NULL;
	QLCFixtureMode* mode = NULL;
	QLCChannel* ch = NULL;
	t_fixture_id fxi_id = KNoID;
	Fixture* fxi = NULL;
	int channels = 0;
	int order = 0;

	m_channels = 0;

	/* Initialization scene */
	if (m_startSceneID != KNoID && m_startSceneEnabled == true)
	{
		startScene = static_cast <class Scene*>
			(_app->doc()->function(m_startSceneID));
		Q_ASSERT(startScene != NULL);
	}

	/* De-initialization scene */
	if (m_stopSceneID != KNoID && m_stopSceneEnabled == true)
	{
		stopScene = static_cast <class Scene*>
			(_app->doc()->function(m_stopSceneID));
		Q_ASSERT(stopScene != NULL);
	}

	QListIterator <t_function_id> it(m_fixtures);
	while (it.hasNext() == true)
	{
		fxi_id = it.next();
		Q_ASSERT(fxi_id != KNoID);

		EFXFixture ef(this, fxi_id, m_channels, order, m_direction,
			      startScene, stopScene);

		fxi = _app->doc()->fixture(fxi_id);
		Q_ASSERT(fxi != NULL);

		mode = fxi->fixtureMode();
		Q_ASSERT(mode != NULL);

		channels = 0;

		for (t_channel i = 0; i < mode->channels(); i++)
		{
			ch = mode->channel(i);
			Q_ASSERT(ch != NULL);

			if (ch->group() == KQLCChannelGroupPan)
			{
				if (ch->controlByte() == 0)
				{
					ef.setMsbPanChannel(
						fxi->universeAddress() + i);
					channels++;
				}
				else if (ch->controlByte() == 1)
				{
					ef.setLsbPanChannel(
						fxi->universeAddress() + i);
					channels++;
				}
			}
			else if (ch->group() == KQLCChannelGroupTilt)
			{
				if (ch->controlByte() == 0)
				{
					ef.setMsbTiltChannel(
						fxi->universeAddress() + i);
					channels++;
				}
				else if (ch->controlByte() == 1)
				{
					ef.setLsbTiltChannel(
						fxi->universeAddress() + i);
					channels++;
				}
			}
		}

		/* The fixture must have at least an LSB channel for 8bit
		   precision to get accepted into the EFX */
		if (ef.isValid() == true)
		{
			ef.updateSkipThreshold();
			m_runTimeData.append(ef);
			m_channels += channels;
			order++;
		}
	}

	/* Allocate space for channel data that is set to the eventbuffer */
	if (m_channelData == NULL)
		m_channelData = new unsigned int[m_channels];

	/* Allocate space for the event buffer, 1/2 seconds worth of events */
	if (m_eventBuffer == NULL)
		m_eventBuffer = new EventBuffer(m_channels, KFrequency >> 1);

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

/**
 * Free all run-time allocations. This is called respectively when
 * the user sets the mode back to Design.
 */
void EFX::disarm()
{
	m_runTimeData.clear();

	if (m_channelData != NULL)
		delete [] m_channelData;
	m_channelData = NULL;

	if (m_eventBuffer != NULL)
		delete m_eventBuffer;
	m_eventBuffer = NULL;

	pointFunc = NULL;
}

/**
 * Stop this EFX
 */
void EFX::stop()
{
	m_stopped = true;
}

/**
 * The worker thread that takes care of filling the function's
 * buffer with event data
 */
void EFX::run()
{
	int ready;

	m_stopped = true;

	emit running(m_id);

	/* Set initial speed */
	slotBusValueChanged(m_busID, Bus::value(m_busID));

	/* Append this function to running functions' list */
	_app->functionConsumer()->cue(this);

	if (pointFunc == NULL || m_fixtures.isEmpty() == true)
		return;

	m_stopped = false;

	/* Go thru all fixtures and calculate their next step */
	while (m_stopped == false)
	{
		ready = 0;

		QMutableListIterator <EFXFixture> it(m_runTimeData);
		while (it.hasNext() == true && m_stopped == false)
		{
			it.next();
			if (it.value().isReady() == false)
			{
				it.value().nextStep(m_channelData);
			}
			else
			{
				ready++;
			}
		}

		m_eventBuffer->put(m_channelData);

		if (ready == m_runTimeData.count())
			m_stopped = true;
	}

	/* Clear the event buffer's contents so that this function will
	   stop immediately. */
	m_eventBuffer->purge();

	/* De-initialize in the end (SingleShot will have de-initialized
	   all "ready" fixtures already) */
	if (m_stopSceneID != KNoID && m_stopSceneEnabled == true)
	{
		class Scene* stopScene = static_cast <class Scene*>
			(_app->doc()->function(m_stopSceneID));
		Q_ASSERT(stopScene != NULL);
		
		stopScene->writeValues();
	}

	/* Reset all fixtures */
	QMutableListIterator <EFXFixture> it(m_runTimeData);
	while (it.hasNext() == true)
		it.next().reset();

	/* Reset channel data */
	for (int i = 0; i < m_channels; i++)
		m_channelData[i] = 0;

}
