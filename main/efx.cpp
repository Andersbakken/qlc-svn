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

#include "efxeditor.h"
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

	setName(tr("New EFX"));

	/* Set Default Fade as the speed bus */
	setBus(KBusIDDefaultFade);
	connect(Bus::emitter(), SIGNAL(valueChanged(t_bus_id,t_bus_value)),
		this, SLOT(slotBusValueChanged(t_bus_id,t_bus_value)));
}

/**
 * Standard destructor
 */
EFX::~EFX()
{
	while (m_fixtures.isEmpty() == false)
		delete m_fixtures.takeFirst();
}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* EFX::createCopy()
{
	Function* function = _app->doc()->newFunction(Function::EFX);
	if (function == NULL)
		return NULL;

	if (function->copyFrom(this) == false)
	{
		_app->doc()->deleteFunction(function->id());
		function = NULL;
	}
	else
	{
		function->setName(tr("Copy of %1").arg(function->name()));
	}

	return function;
}

bool EFX::copyFrom(const Function* function)
{
	const EFX* efx = qobject_cast<const EFX*> (function);
	if (efx == NULL)
		return false;

	m_fixtures.clear();
	QListIterator <EFXFixture*> it(efx->m_fixtures);
	while (it.hasNext() == true)
	{
		EFXFixture* ef = new EFXFixture(this);
		ef->copyFrom(it.next());
		m_fixtures.append(ef);
	}

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

	m_algorithm = QString(efx->algorithm());

	return Function::copyFrom(function);
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
 */
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

/**
 * Get the current algorithm
 *
 * @return Name of the current algorithm. See @ref algorithmList
 */
QString EFX::algorithm() const
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
	if (algorithmList().contains(algorithm) == true)
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
int EFX::width() const
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
int EFX::height() const
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
int EFX::rotation() const
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
int EFX::xOffset() const
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
int EFX::yOffset() const
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
int EFX::xFrequency() const
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
int EFX::yFrequency() const
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
int EFX::xPhase() const
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
int EFX::yPhase() const
{
	return static_cast<int> (m_yPhase * 180.0 / M_PI);
}

/**
 * Returns true when lissajous has been selected
 */
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

	return true;
}

bool EFX::removeFixture(EFXFixture* ef)
{
	Q_ASSERT(ef != NULL);

	if (m_fixtures.removeAll(ef) > 0)
		return true;
	else
		return false;
}

bool EFX::raiseFixture(EFXFixture* ef)
{
	Q_ASSERT(ef != NULL);

	int index = m_fixtures.indexOf(ef);
	if (index > 0)
	{
		m_fixtures.move(index, index - 1);
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
			it.remove();
			delete it.value();
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
	if (scene >= KNoID && scene <= KFunctionArraySize)
	{
		m_startSceneID = scene;
	}
}

/**
 * Get the id for start scene
 *
 */
t_function_id EFX::startScene() const
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
bool EFX::startSceneEnabled() const
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
t_function_id EFX::stopScene() const
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
bool EFX::stopSceneEnabled() const
{
	return m_stopSceneEnabled;
}

/*****************************************************************************
 * Edit
 *****************************************************************************/

int EFX::edit()
{
	EFXEditor editor(_app, this);
	int result = editor.exec();
	if (result == QDialog::Accepted)
		emit changed(m_id);
	return result;
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
			EFXFixture* ef = new EFXFixture(this);
			ef->loadXML(doc, &tag);
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

void EFX::arm()
{
	class Scene* startScene = NULL;
	class Scene* stopScene = NULL;
	QLCFixtureMode* mode = NULL;
	QLCChannel* ch = NULL;
	int serialNumber = 0;
	Fixture* fxi = NULL;

	/* Initialization scene */
	if (m_startSceneID != KNoID && m_startSceneEnabled == true)
		startScene = static_cast <class Scene*>
			(_app->doc()->function(m_startSceneID));

	/* De-initialization scene */
	if (m_stopSceneID != KNoID && m_stopSceneEnabled == true)
		stopScene = static_cast <class Scene*>
			(_app->doc()->function(m_stopSceneID));

	QListIterator <EFXFixture*> it(m_fixtures);
	while (it.hasNext() == true)
	{
		EFXFixture* ef = it.next();
		Q_ASSERT(ef != NULL);

		ef->setSerialNumber(serialNumber++);
		ef->setStartScene(startScene);
		ef->setStopScene(stopScene);

		/* If fxi == NULL, the fixture has been destroyed */
		fxi = _app->doc()->fixture(ef->fixture());
		if (fxi == NULL)
			continue;

		/* If this fixture has no mode, it's a generic dimmer that
		   can't do pan&tilt anyway. */
		mode = fxi->fixtureMode();
		if (mode == NULL)
			continue;

		/* Find exact channel numbers for MSB/LSB pan and tilt */
		for (t_channel i = 0; i < mode->channels(); i++)
		{
			ch = mode->channel(i);
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

void EFX::start()
{
	/* Set initial speed */
	slotBusValueChanged(m_busID, Bus::value(m_busID));
	Function::start();
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

void EFX::stop()
{
	Function::stop();

	/* Reset all fixtures */
	QListIterator <EFXFixture*> it(m_fixtures);
	while (it.hasNext() == true)
		it.next()->reset();
}

