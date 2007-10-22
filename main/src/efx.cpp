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

#include <qfile.h>
#include <qptrlist.h>
#include <time.h>
#include <qapplication.h>
#include <qpointarray.h>
#include <assert.h>
#include <math.h>

#include "common/filehandler.h"
#include "common/qlcfixturedef.h"

#include "app.h"
#include "doc.h"
#include "efx.h"
#include "bus.h"
#include "fixture.h"
#include "eventbuffer.h"
#include "functionconsumer.h"

extern App* _app;

/* Supported EFX algorithms */
static const char* KCircleAlgorithmName    ( "Circle" );
static const char* KEightAlgorithmName     ( "Eight" );
static const char* KLineAlgorithmName      ( "Line" );
static const char* KDiamondAlgorithmName   ( "Diamond" );
static const char* KTriangleAlgorithmName  ( "Triangle" );
static const char* KLissajousAlgorithmName ( "Lissajous" );

/**
 * Standard constructor
 */
EFX::EFX() :
	Function            ( Function::EFX ),

	pointFunc           ( NULL ),

	m_width             ( 127 ),
	m_height            ( 127 ),
	m_xOffset           ( 127 ),
	m_yOffset           ( 127 ),
	m_rotation	      ( 0 ),

	m_xFrequency        ( 2 ),
	m_yFrequency        ( 3 ),
	m_xPhase            ( 1.5707963267 ),
	m_yPhase            ( 0 ),

	m_xChannel          ( KChannelInvalid ),
	m_yChannel          ( KChannelInvalid ),

	m_runOrder          ( EFX::Loop ),
	m_direction         ( EFX::Forward ),

	m_modulationBus     ( KBusIDDefaultHold ),

	m_startSceneID      ( KNoID ),
	m_startSceneEnabled ( false ),

	m_stopSceneID       ( KNoID ),
	m_stopSceneEnabled  ( false ),

	m_previewPointArray ( NULL ),

	m_algorithm         ( KCircleAlgorithmName ),

	m_stepSize          ( 0 ),
	m_cycleDuration     ( KFrequency ),

	m_channelData       ( NULL ),

	m_address           ( KChannelInvalid )
{
	/* Set Default Fade as the speed bus */
	setBus(KBusIDDefaultFade);
}

/**
 * Standard destructor
 */
EFX::~EFX()
{
	stop();

	m_startMutex.lock();
	while (m_running)
	{
		m_startMutex.unlock();
		sched_yield();
		m_startMutex.lock();
	}
	m_startMutex.unlock();
}

/**
 * Set a pointer to a point array for updating the
 * changes when editing the function.
 *
 * @note Call this function with NULL after editing is finished!
 *
 * @param array The array to save the preview points to
 */
void EFX::setPreviewPointArray(QPointArray* array)
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
	{
		return;
	}

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

	list = list.grep(algorithm);
	if (list.isEmpty())
	{
		qDebug("Invalid algorithm for EFX: " + algorithm);
		m_algorithm = KCircleAlgorithmName;
	}
	else
	{
		m_algorithm = QString(algorithm);
	}

	updatePreview();
}

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

/**
 * Set a channel from a fixture instance to be used as the X axis.
 *
 * @param channel Relative number of the channel used as the X axis
 */
void EFX::setXChannel(t_channel channel)
{
	Fixture* fxi = NULL;

	fxi = _app->doc()->fixture(fixture());
	Q_ASSERT(fxi != NULL);

	if (channel < (t_channel) fxi->channels())
	{
		m_xChannel = channel;
		updatePreview();
	}
	else
	{
		qDebug("Invalid channel number!");
		assert(false);
	}
}

/**
 * Set a channel from a fixture instance to be used as the Y axis.
 *
 * @param channel Relative number of the channel used as the Y axis
 */
void EFX::setYChannel(t_channel channel)
{
	Fixture* fxi = NULL;

	fxi = _app->doc()->fixture(fixture());
	Q_ASSERT(fxi != NULL);

	if (channel < (t_channel) fxi->channels())
	{
		m_yChannel = channel;
		updatePreview();
	}
	else
	{
		qDebug("Invalid channel number!");
		assert(false);
	}
}

/**
 * Get the channel used as the X axis.
 *
 */
t_channel EFX::xChannel()
{
	return m_xChannel;
}

/**
 * Get the channel used as the Y axis.
 *
 */
t_channel EFX::yChannel()
{
	return m_yChannel;
}

/**
 * Set the run order
 *
 * @param runOrder Run Order
 */
void EFX::setRunOrder(EFX::RunOrder runOrder)
{
	m_runOrder = runOrder;
}

/**
 * Get the run order
 *
 */
EFX::RunOrder EFX::runOrder()
{
	return m_runOrder;
}

/**
 * Set the running direction
 *
 * @param dir Direction
 */
void EFX::setDirection(EFX::Direction dir)
{
	m_direction = dir;
}

/**
 * Get the direction
 *
 */
EFX::Direction EFX::direction()
{
	return m_direction;
}

/**
 * Set the modulation speed bus
 *
 */
void EFX::setModulationBus(t_bus_id bus)
{
	if (bus > KBusIDMin && bus < KBusCount)
	{
		m_modulationBus = bus;
	}
}

/**
 * Get the modulation speed bus
 *
 */
t_bus_id EFX::modulationBus()
{
	return m_modulationBus;
}


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

/**
 * Copy function contents from another function
 *
 * @param efx EFX function from which to copy contents to this function
 * @param to The new parent fixture instance for this function
 */
bool EFX::copyFrom(EFX* efx, t_fixture_id to)
{
	assert(efx);

	Function::setFixture(to);
	Function::setName(efx->name());
	Function::setBus(efx->busID());

	m_width = efx->width();
	m_height = efx->height();
	m_xOffset = efx->xOffset();
	m_yOffset = efx->yOffset();
	m_rotation = efx->rotation();

	m_xFrequency = efx->xFrequency();
	m_yFrequency = efx->yFrequency();
	m_xPhase = efx->xPhase();
	m_yPhase = efx->yPhase();

	m_xChannel = efx->xChannel();
	m_yChannel = efx->yChannel();

	m_runOrder = efx->runOrder();
	m_direction = efx->direction();

	m_modulationBus = efx->modulationBus();

	m_startSceneID = efx->startScene();
	m_startSceneEnabled = efx-startSceneEnabled();

	m_stopSceneID = efx->stopScene();
	m_stopSceneEnabled = efx-stopSceneEnabled();

	m_previewPointArray = NULL;

	m_algorithm = QString(efx->algorithm());

	m_stepSize = 0;
	m_cycleDuration = KFrequency;

	m_channelData = NULL;

	m_address = KChannelInvalid;

	return true;
}

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
	int i = 0;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(wksp_root != NULL);

	/* Function tag */
	root = doc->createElement(KXMLQLCFunction);
	wksp_root->appendChild(root);

	root.setAttribute(KXMLQLCFunctionID, id());
	root.setAttribute(KXMLQLCFunctionType, Function::typeToString(m_type));
	root.setAttribute(KXMLQLCFunctionName, name());
	root.setAttribute(KXMLQLCFunctionFixture, fixture());

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
	tag = doc->createElement(KXMLQLCFunctionEFXAlgorithm);
	root.appendChild(tag);
	text = doc->createTextNode(algorithm());
	tag.appendChild(text);

	/* Width */
	tag = doc->createElement(KXMLQLCFunctionEFXWidth);
	root.appendChild(tag);
	str.setNum(width());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Height */
	tag = doc->createElement(KXMLQLCFunctionEFXHeight);
	root.appendChild(tag);
	str.setNum(height());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Rotation */
	tag = doc->createElement(KXMLQLCFunctionEFXRotation);
	root.appendChild(tag);
	str.setNum(rotation());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Start function */
	tag = doc->createElement(KXMLQLCFunctionEFXStartScene);
	root.appendChild(tag);
	str.setNum(startScene());
	text = doc->createTextNode(str);
	tag.appendChild(text);
	if (startSceneEnabled() == true)
		tag.setAttribute(KXMLQLCFunctionEnabled, KXMLQLCTrue);
	else
		tag.setAttribute(KXMLQLCFunctionEnabled, KXMLQLCFalse);

	/* Stop function */
	tag = doc->createElement(KXMLQLCFunctionEFXStopScene);
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
	tag = doc->createElement(KXMLQLCFunctionEFXAxis);
	root.appendChild(tag);
	tag.setAttribute(KXMLQLCFunctionName, KXMLQLCFunctionEFXX);

	/* Offset */
	subtag = doc->createElement(KXMLQLCFunctionEFXOffset);
	tag.appendChild(subtag);
	str.setNum(xOffset());
	text = doc->createTextNode(str);
	subtag.appendChild(text);

        /* Frequency */
	subtag = doc->createElement(KXMLQLCFunctionEFXFrequency);
	tag.appendChild(subtag);
	str.setNum(xFrequency());
	text = doc->createTextNode(str);
	subtag.appendChild(text);

        /* Phase */
	subtag = doc->createElement(KXMLQLCFunctionEFXPhase);
	tag.appendChild(subtag);
	str.setNum(xPhase());
	text = doc->createTextNode(str);
	subtag.appendChild(text);

        /* Channel */
	subtag = doc->createElement(KXMLQLCFunctionEFXChannel);
	tag.appendChild(subtag);
	str.setNum(xChannel());
	text = doc->createTextNode(str);
	subtag.appendChild(text);

	/********************************************
	 * Y-Axis 
	 ********************************************/
	tag = doc->createElement(KXMLQLCFunctionEFXAxis);
	root.appendChild(tag);
	tag.setAttribute(KXMLQLCFunctionName, KXMLQLCFunctionEFXY);

	/* Offset */
	subtag = doc->createElement(KXMLQLCFunctionEFXOffset);
	tag.appendChild(subtag);
	str.setNum(yOffset());
	text = doc->createTextNode(str);
	subtag.appendChild(text);

	/* Frequency */
	subtag = doc->createElement(KXMLQLCFunctionEFXFrequency);
	tag.appendChild(subtag);
	str.setNum(yFrequency());
	text = doc->createTextNode(str);
	subtag.appendChild(text);

        /* Phase */
	subtag = doc->createElement(KXMLQLCFunctionEFXPhase);
	tag.appendChild(subtag);
	str.setNum(yPhase());
	text = doc->createTextNode(str);
	subtag.appendChild(text);

        /* Channel */
	subtag = doc->createElement(KXMLQLCFunctionEFXChannel);
	tag.appendChild(subtag);
	str.setNum(yChannel());
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
		qWarning("Function node not found!");
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
			Q_ASSERT(setBus(tag.text().toInt()) == true);
		}
		else if (tag.tagName() == KXMLQLCFunctionEFXAlgorithm)
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
		else if (tag.tagName() == KXMLQLCFunctionEFXWidth)
		{
			/* Width */
			setWidth(tag.text().toInt());
		}
		else if (tag.tagName() == KXMLQLCFunctionEFXHeight)
		{
			/* Height */
			setHeight(tag.text().toInt());
		}
		else if (tag.tagName() == KXMLQLCFunctionEFXRotation)
		{
			/* Rotation */
			setRotation(tag.text().toInt());
		}
		else if (tag.tagName() == KXMLQLCFunctionEFXStartScene)
		{
			/* Start scene */
			setStartScene(tag.text().toInt());

			if (tag.attribute(KXMLQLCFunctionEnabled) ==
			    KXMLQLCTrue)
				setStartSceneEnabled(true);
			else
				setStartSceneEnabled(false);
		}
		else if (tag.tagName() == KXMLQLCFunctionEFXStopScene)
		{
			/* Stop scene */
			setStopScene(tag.text().toInt());

			if (tag.attribute(KXMLQLCFunctionEnabled) ==
			    KXMLQLCTrue)
				setStopSceneEnabled(true);
			else
				setStopSceneEnabled(false);
		}
		else if (tag.tagName() == KXMLQLCFunctionEFXAxis)
		{
			/* Axes */
			loadXMLAxis(doc, &tag);
		}
		else
		{
			qDebug("Unknown EFX tag: %s",
			       (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}

	return true;
}

bool EFX::loadXMLAxis(QDomDocument* doc, QDomElement* root)
{
	QString axis;
	int offset = 0;
	int frequency = 0;
	int phase = 0;
	t_channel channel = 0;

	QDomNode node;
	QDomElement tag;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCFunctionEFXAxis)
	{
		qWarning("EFX axis node not found!");
		return false;
	}

	/* Get the axis name */
	axis = root->attribute(KXMLQLCFunctionName);

	/* Load axis contents */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		
		if (tag.tagName() == KXMLQLCFunctionEFXOffset)
		{
			offset = tag.text().toInt();
		}
		else if (tag.tagName() == KXMLQLCFunctionEFXFrequency)
		{
			frequency = tag.text().toInt();
		}
		else if (tag.tagName() == KXMLQLCFunctionEFXPhase)
		{
			phase = tag.text().toInt();
		}
		else if (tag.tagName() == KXMLQLCFunctionEFXChannel)
		{
			channel = tag.text().toInt();
		}
		else
		{
			qWarning("Unknown EFX axis tag: %s",
				 (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}

	if (axis == KXMLQLCFunctionEFXY)
	{
		setYOffset(offset);
		setYFrequency(frequency);
		setYPhase(phase);
		setYChannel(channel);
	}
	else if (axis == KXMLQLCFunctionEFXX)
	{
		setXOffset(offset);
		setXFrequency(frequency);
		setXPhase(phase);
		setXChannel(channel);
	}
	else
	{
		qWarning("Unknown EFX axis: %s", (const char*) axis);
	}
	
	return true;
}

/**
 * This is called by buses for each function when the
 * bus value is changed.
 *
 * @param id ID of the bus that has changed its value
 * @param value Bus' new value
 */
void EFX::busValueChanged(t_bus_id id, t_bus_value value)
{
	if (id != m_busID)
	{
		/* Not our bus */
		return;
	}

	m_startMutex.lock();

	/* Basically number of steps required to complete a full cycle */
	m_cycleDuration = static_cast<double> (value);

	/* Size of one step */
	m_stepSize = (double)(1) / ((double)(m_cycleDuration) / (M_PI * 2));

	m_startMutex.unlock();
}

/**
 * Prepare this function for running. This is called when
 * the user sets the mode to Operate. Basically allocates everything
 * that is needed to run the function.
 */
void EFX::arm()
{
	Fixture* fxi = NULL;

	/* Allocate space for channel data set to eventbuffer.
	 * There are only two channels to set.
	 */
	if (m_channelData == NULL)
		m_channelData = new unsigned int[2];

	/* Allocate space for the event buffer.
	 * There are only two channels to set.
	 */
	if (m_eventBuffer == NULL)
		m_eventBuffer = new EventBuffer(2, KFrequency >> 1);

	/* Set the run time address for channel data */
	fxi = _app->doc()->fixture(fixture());
	if (fxi != NULL)
	{
		m_address = fxi->universeAddress();
	}
	else
	{
		qDebug("No fixture instance for EFX: " + Function::name());
	}

	m_stopped = false;

	/* Choose a point calculation function depending on the algorithm */
	if (m_algorithm == KCircleAlgorithmName)
	{
		pointFunc = circlePoint;
	}
	else if (m_algorithm == KEightAlgorithmName)
	{
		pointFunc = eightPoint;
	}
	else if (m_algorithm == KLineAlgorithmName)
	{
		pointFunc = linePoint;
	}
	else if (m_algorithm == KTriangleAlgorithmName)
	{
		pointFunc = trianglePoint;
	}
	else if (m_algorithm == KDiamondAlgorithmName)
	{
		pointFunc = diamondPoint;
	}
	else if (m_algorithm == KLissajousAlgorithmName)
	{
		pointFunc = lissajousPoint;
	}
	else
	{
		/* There's something wrong, don't run this function */
		pointFunc = NULL;
		m_stopped = true;

		qDebug("Unknown algorithm used in EFX: " + m_name);
	}
}

/**
 * Free all run-time allocations. This is called respectively when
 * the user sets the mode back to Design.
 */
void EFX::disarm()
{
	if (m_channelData) delete [] m_channelData;
	m_channelData = NULL;

	if (m_eventBuffer) delete m_eventBuffer;
	m_eventBuffer = NULL;

	m_address = KChannelInvalid;

	pointFunc = NULL;
}

/**
 * Called by FunctionConsumer after the function has stopped running.
 * Usually notifies parent function and/or virtual console that the
 * function has been stopped.
 */
void EFX::cleanup()
{
	m_stopped = false;

	if (m_virtualController)
	{
		QApplication::postEvent(m_virtualController,
					new FunctionStopEvent(m_id));
		m_virtualController = NULL;
	}

	if (m_parentFunction)
	{
		m_parentFunction->childFinished();
		m_parentFunction = NULL;
	}

	m_startMutex.lock();
	m_running = false;
	m_startMutex.unlock();
}

/**
 * Pre-run initialization that is run just before the function is started.
 */
void EFX::init()
{
	t_bus_value speed;

	m_removeAfterEmpty = false;

	// Get speed
	Bus::value(m_busID, speed);
	busValueChanged(m_busID, speed);

	// Append this function to running functions' list
	_app->functionConsumer()->cue(this);
}

/**
 * The worker thread that takes care of filling the function's
 * buffer with event data
 */
void EFX::run()
{
	float i = 0;
	float x = 0;
	float y = 0;
	Direction dir = direction();

	// Initialize this function for running
	init();

	if (!m_stopped)
	{
		if (startScene() != KNoID && startSceneEnabled())
		{
			_app->doc()->function(startScene())->engage(this);
		}
	}

	while (!m_stopped)
	{
		if (dir == Forward)
		{
			for (i = 0; i < (M_PI * 2.0) && !m_stopped; i += m_stepSize)
			{
				/* Calculate the next point */
				pointFunc(this, i, &x, &y);
	      
				/* Write the point to event buffer */
				setPoint(static_cast<t_value> (x), static_cast<t_value> (y));
			}
		}
		else
		{
			for (i = (M_PI * 2.0); i > 0 && !m_stopped; i -= m_stepSize)
			{
				/* Calculate the next point */
				pointFunc(this, i, &x, &y);
	      
				/* Write the point to event buffer */
				setPoint(static_cast<t_value> (x), static_cast<t_value> (y));
			}
		}

		if (runOrder() == PingPong)
		{
			if (dir == Forward)
				dir = Backward;
			else
				dir = Forward;
		}
		else if (runOrder() == SingleShot)
		{
			m_stopped = true;
		}
	}

	if (stopScene() != KNoID && stopSceneEnabled())
	{
		_app->doc()->function(stopScene())->engage(this);
	}

	/* Finished */
	m_removeAfterEmpty = true;
}

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
	efx->rotateAndScale(efx, x, y, efx->m_rotation);
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
	efx->rotateAndScale(efx, x, y, efx->m_rotation);
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
	efx->rotateAndScale(efx, x, y, efx->m_rotation);
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
	efx->rotateAndScale(efx, x, y, efx->m_rotation);
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
	efx->rotateAndScale(efx, x, y, efx->m_rotation);
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
	efx->rotateAndScale(efx, x, y, efx->m_rotation);
}

/**
 * Write the actual calculated coordinate data to
 * event buffer.
 */
void EFX::setPoint(t_value x, t_value y)
{
	m_channelData[0] = (m_address + m_xChannel) << 8;
	m_channelData[0] |= x;

	m_channelData[1] = (m_address + m_yChannel) << 8;
	m_channelData[1] |= y;

	m_eventBuffer->put(m_channelData);
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
void EFX::rotateAndScale(EFX* efx, float* x, float* y, int rot)
{
	float xx, yy;
	xx = *x;
	yy = *y;
	float r = M_PI/180 * float (rot);
	*x = efx->m_xOffset + (xx * cos(r) + yy  * sin(r)) * efx->m_width ;
	*y = efx->m_yOffset + (-xx * sin(r) + yy * cos(r))  * efx->m_height;
}


