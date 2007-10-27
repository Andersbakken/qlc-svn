/*
  Q Light Controller
  monitor.cpp

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

#include <qapplication.h>
#include <qcolor.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qrect.h>
#include <qpixmap.h>
#include <qfont.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <math.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qfontdialog.h>
#include <qevent.h>
#include <assert.h>

#include "app.h"
#include "doc.h"
#include "dmxmap.h"
#include "monitor.h"

#include "common/types.h"
#include "common/filehandler.h"

extern App* _app;
extern QApplication* _qapp;

#define X_OFFSET            10
#define Y_OFFSET            5

#define ID_16HZ             16
#define ID_32HZ             32
#define ID_64HZ             64

#define ID_CHOOSE_FONT      1000

static const QString KEY_MONITOR_FONT             ( "MonitorFont" );
static const QString KEY_MONITOR_UPDATE_FREQUENCY ( "MonitorUpdateFrequency" );
static const QString KEY_MONITOR_GEOMETRY         ( "MonitorGeometry" );

//
// Constructor
//
Monitor::Monitor(QWidget* parent, DMXMap* dmxMap) :
	QWidget(parent, "Monitor"),
	m_universe        ( 0 ),
	m_visibleMin      ( 0 ),
	m_visibleMax      ( 0 ),
	m_newValues       ( NULL ),
	m_oldValues       ( NULL ),
	m_dmxMap          ( dmxMap ),
	m_timer           ( NULL ),
	m_updateFrequency ( ID_16HZ ),
	m_menuBar         ( NULL ),
	m_universeMenu    ( NULL ),
	m_displayMenu     ( NULL ),
	m_speedMenu       ( NULL )
{
}


//
// Destructor
//
Monitor::~Monitor()
{
	if (m_timer != NULL)
		delete m_timer;

	while (m_painter.isActive());
		
	delete [] m_newValues;
	delete [] m_oldValues;
	
	if (m_menuBar != NULL)
		delete m_menuBar;
}


//
// Window has been closed
//
void Monitor::closeEvent(QCloseEvent* e)
{
	emit closed();
}


//
// Initialize the UI
//
void Monitor::init()
{
	// Set the window icon
	setIcon(QString(PIXMAPS) + QString("/monitor.png"));

	// Set the window title
	setCaption("DMX Monitor - Universe 1");
	
	// Set background color
	setBackgroundColor(colorGroup().base());

	// Init the arrays that hold the values
	m_newValues = new t_value[512];
	m_oldValues = new t_value[512];
	for (t_channel i = 0; i < 512; i++)
		m_newValues[i] = m_oldValues[i] = 0;

	// Create the menu bar
	initMenu();
	
	// Connect and start the update timer
	setFrequency(ID_16HZ);

	// Set a reasonable default size
	resize(280, 150);
}

//
// Create a menu bar
//
void Monitor::initMenu()
{
	QString s;

	m_menuBar = new QMenuBar(this);
	m_menuBar->show();
	
	// Universe
	m_universeMenu = new QPopupMenu();
	m_universeMenu->setCheckable(true);
	m_menuBar->insertItem("&Universe", m_universeMenu);
	connect(m_universeMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));

	for (t_channel i = 0; i < KUniverseCount; i++)
	{
		s.sprintf("Universe %d", i + 1);
		m_universeMenu->insertItem(s, i);
		
		if (m_universe == i)
			m_universeMenu->setItemChecked(i, true);
	}

	// Display menu
	m_displayMenu = new QPopupMenu();
	m_displayMenu->setCheckable(false);
	m_menuBar->insertItem("&Display", m_displayMenu);
	connect(m_displayMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));
	
	// Update speed
	m_speedMenu = new QPopupMenu();
	m_speedMenu->setCheckable(true);
	m_displayMenu->insertItem(QPixmap(QString(PIXMAPS) +
				  QString("/clock.png")), 
				  "&Update Speed", m_speedMenu);
	connect(m_speedMenu, SIGNAL(activated(int)),
		this, SLOT(slotMenuCallback(int)));

	m_speedMenu->insertItem("16Hz", ID_16HZ);
	m_speedMenu->insertItem("32Hz", ID_32HZ);
	m_speedMenu->insertItem("64Hz", ID_64HZ);
	m_speedMenu->setItemChecked(m_updateFrequency, true);
		
	// Font
	m_displayMenu->insertItem(QPixmap(QString(PIXMAPS) +
					  QString("/fonts.png")),
				  "Choose &Font", ID_CHOOSE_FONT);
}

void Monitor::setUniverse(t_channel universe)
{
	QString s;
	
	assert(universe < KUniverseCount);

	m_universeMenu->setItemChecked(m_universe, false);
	m_universe = universe;
	m_universeMenu->setItemChecked(m_universe, true);

	setCaption(s.sprintf("DMX Monitor - Universe %d", universe + 1));

	repaint(true);
}

void Monitor::setFrequency(int freq)
{
	if (m_speedMenu != NULL)
	{
		m_speedMenu->setItemChecked(m_updateFrequency, false);
		m_speedMenu->setItemChecked(freq, true);
	}

	m_updateFrequency = freq;

	if (m_timer == NULL)
	{
		m_timer = new QTimer();
		connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));
	}
	else
	{
		m_timer->stop();
	}

	m_timer->start(1000 / m_updateFrequency);
}

//
// Invoke an action from the menu
//
void Monitor::slotMenuCallback(int item)
{
	if (item >= 0 && item < KUniverseCount)
	{
		setUniverse(item);
	}
	else
	{
		switch (item)
		{
		case ID_16HZ:
		case ID_32HZ:
		case ID_64HZ:
			setFrequency(item);
			break;

		case ID_CHOOSE_FONT:
		{
			bool ok = false;
			QFont font = QFontDialog::getFont(&ok, m_font, this);
			if (ok == true)
			{
				m_font = font;
				repaint(true); // Paint all
			}
			break;
		}

		default:
			break;
		}
	}
}

//
// Timer expiration handler
//
void Monitor::slotTimeOut()
{
	/* Read only the visible range of values. There's no point in reading
	   such values that aren't going to get drawn anyway. */
	m_dmxMap->getValueRange(512 * m_universe + m_visibleMin,
				m_newValues, m_visibleMax + 1);

	// Paint only changed values
	repaint(false);
}

/****************************************************************************
 * Load/save settings
 ****************************************************************************/

void Monitor::loader(QDomDocument* doc, QDomElement* root)
{
	_app->createMonitor();
	_app->monitor()->loadXML(doc, root);
}

bool Monitor::loadXML(QDomDocument* doc, QDomElement* root)
{
	bool visible = false;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	
	QDomNode node;
	QDomElement tag;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);
	
	if (root->tagName() != KXMLQLCMonitor)
	{
		qWarning("Monitor node not found!");
		return false;
	}

	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCWindowState)
		{
			FileHandler::loadXMLWindowState(&tag, &x, &y, &w, &h,
							&visible);
		}
		else if (tag.tagName() == KXMLQLCMonitorFont)
		{
			m_font.fromString(tag.text());
			repaint(true); // Repaint all
		}
		else if (tag.tagName() == KXMLQLCMonitorUpdateFrequency)
		{
			setFrequency(tag.text().toInt());
		}
		else
		{
			qDebug("Unknown monitor tag: %s",
			       (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}

	hide();
	setGeometry(x, y, w, h);
	if (visible == false)
		showMinimized();
	else
		showNormal();

	return true;
}

bool Monitor::saveXML(QDomDocument* doc, QDomElement* fxi_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(fxi_root != NULL);

	/* Fixture Console entry */
	root = doc->createElement(KXMLQLCMonitor);
	fxi_root->appendChild(root);

	/* Font */
	tag = doc->createElement(KXMLQLCMonitorFont);
	root.appendChild(tag);
	text = doc->createTextNode(m_font.toString());
	tag.appendChild(text);

	/* Update frequency */
	tag = doc->createElement(KXMLQLCMonitorUpdateFrequency);
	root.appendChild(tag);
	str.setNum(m_updateFrequency);
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Save window state. parentWidget() should be used for all
	   widgets within the workspace. */
	return FileHandler::saveXMLWindowState(doc, &root, parentWidget());
}

/****************************************************************************
 * Label & value painting
 ****************************************************************************/

//
// Paint event has been received
//
void Monitor::paintEvent(QPaintEvent* e)
{
	QFontMetrics metrics(m_font);
	int unitW = metrics.width(QString("000")) + X_OFFSET;
	int unitH = metrics.height();
	int unitsX = (rect().width() - X_OFFSET) / (unitW);
	int y_offset = m_menuBar->height() + Y_OFFSET;
	int x_offset = X_OFFSET; // Doesn't work yet with anything else than 0
	
	// Wait until previous painter is finished
	while (m_painter.isActive());

	// Begin painting
	m_painter.begin(this);

	// Set the font used for drawing text
	m_painter.setFont(m_font);

	if (e->erased())
	{
		// Draw everything that is inside the invalidated region
		paintFixtureLabelAll(e->region(), x_offset, y_offset,
				     unitW, unitH, unitsX);
		paintChannelLabelAll(e->region(), x_offset, y_offset,
				     unitW, unitH, unitsX);
		paintChannelValueAll(e->region(), x_offset, y_offset,
				     unitW, unitH, unitsX, false);
	}
	else
	{
		// Draw only changed values
		paintChannelValueAll(e->region(), x_offset, y_offset,
				     unitW, unitH, unitsX, true);
	}

	m_painter.end();
	
	// Paint the generic graphics
	QWidget::paintEvent(e);
}

//
// Paint all visible fixture labels
//
void Monitor::paintFixtureLabelAll(QRegion region, int x_offset, int y_offset,
				   int unitW, int unitH, int unitsX)
{
	int x = 0;
	int y = 0;
	int w = 0;
	int wcur = 0;
	int h = 0;
	
	t_fixture_id id = KNoID;
	Fixture* fxi = NULL;
	
	// Draw fixture names and their channel spaces
	for (id = 0; id < KFixtureArraySize; id++)
	{
		fxi = _app->doc()->fixture(id);
		if (fxi == NULL) continue;
		if (fxi->universe() != m_universe) continue;
		
		// Calculate x and y positions for this fixture label
		x = ((fxi->address() % unitsX) * unitW);
		x += x_offset;
		
		y = static_cast<int> 
			(floor((fxi->address() / unitsX)) * (unitH * 3));
		y += y_offset;
		
		// Get width and height for this fixture label
		w = (fxi->channels() * unitW) - X_OFFSET;
		h = unitH;
			
		// Check if this label needs to be painted at all
		if (region.contains(QRect(x, y, w, h)) == false)
			continue;

		if ((x + w + X_OFFSET) <= width())
		{
			// The label fits to one line, just draw it
			paintFixtureLabel(x, y, w, h, fxi->name());
		}
		else
		{
			// The label needs to be drawn on at least two lines
			while (w > 0)
			{	
				wcur = MIN(w, (unitsX * unitW) - (x));
				
				// Draw the label
				paintFixtureLabel(x, y, wcur, h, fxi->name());
			
				// Calculate remaining width
				w = w - wcur - X_OFFSET;
				
				// Next line
				y += (unitH * 3);
				x = x_offset;
			}
		}
	}
}

//
// Draw the fixture label to the given coordinates
//
void Monitor::paintFixtureLabel(int x, int y, int w, int h, QString label)
{
	_qapp->lock();
		
	m_painter.fillRect(x, y, w, h, colorGroup().highlight());
		
	m_painter.setPen(colorGroup().shadow());
	m_painter.drawRect(x, y, w, h);

	m_painter.setPen(colorGroup().highlightedText());
	m_painter.drawText(x, y, w, h, AlignLeft | AlignVCenter, label);

	_qapp->unlock();
}

//
// Paint all channel labels
//
void Monitor::paintChannelLabelAll(QRegion region, int x_offset, int y_offset,
				   int unitW, int unitH, int unitsX)
{
	int x = 0;
	int y = 0;
	int i = 0;

	QString s;
	
	for (i = 0; i < 512; i++)
	{
		s.sprintf("%.3d", i + 1);

		// Calculate x and y positions for this channel
		x = ((i % unitsX) * (unitW));
		x += x_offset;
		
		y = unitH + static_cast<int> 
			(floor((i / unitsX)) * (unitH * 3));
		y += y_offset;
		
		if (region.contains(QRect(x, y, unitW, unitH)))
		{
			// Paint channel label
			paintChannelLabel(x, y, unitW, unitH, s);
		}
	}
}

//
// Paint channel label
//
void Monitor::paintChannelLabel(int x, int y, int w, int h, QString s)
{
	_qapp->lock();
	
	m_painter.setPen(colorGroup().dark());
	m_painter.eraseRect(x, y, w, h);
	m_painter.drawText(x, y, w, h, AlignBottom, s);
	
	_qapp->unlock();
}

//
// Paint all channel values
//
void Monitor::paintChannelValueAll(QRegion region, int x_offset, int y_offset,
				   int unitW, int unitH, int unitsX,
				   bool onlyDelta)
{
	int x = 0;
	int y = 0;
	int value = 0;
	int i = 0;
	
	QString s;

	// Set normal text color to painter
	m_painter.setPen(colorGroup().text());
	
	for (i = 0; i < 512; i++)
	{
		// Lock value array
		m_valueMutex.lock();
	
		if (onlyDelta && m_oldValues[i] == m_newValues[i])
		{
			m_valueMutex.unlock();
			continue;
		}
		
		m_oldValues[i] = m_newValues[i];

		// Get channel value from array;
		value = m_newValues[i];
		
		// Unlock array
		m_valueMutex.unlock();
		
		// Calculate xy position for this channel
		x = ((i % unitsX) * (unitW));
		x += x_offset;
		
		y = (unitH * 2) + 
		    static_cast<int> (floor((i / unitsX)) * (unitH * 3));
		y += y_offset;
		
		// If all values must be drawn, draw only those that are
		// inside the invalidated area, otherwise draw the delta values
		if (!onlyDelta && !region.contains(QRect(x, y, unitW, unitH)))
			continue;
		
		// Draw only those values that are visible
		if (y < height())
		{
			// Convert the value to a string
			s.sprintf("%.3d", value);
		
			// Paint the value
			paintChannelValue(x, y, unitW, unitH, s);

			/* Update the biggest visible channel number only,
			   when the visibility of all channels is being
			   checked */
			if (!onlyDelta)
				m_visibleMax = i;
		}
		else
		{
			// Rest of the values are not visible either
			break;
		}
	}
}

//
// Paint one channel value
//
void Monitor::paintChannelValue(int x, int y, int w, int h, QString s)
{
	_qapp->lock();
	m_painter.eraseRect(x, y, w, h);
	m_painter.drawText(x, y, w, h, AlignBottom, s);
	_qapp->unlock();
}
