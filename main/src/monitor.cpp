/*
  Q Light Controller
  monitor.h

  Copyright (C) 2000, 2001, 2002 Heikki Junnila

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

#include "monitor.h"
#include "app.h"
#include "doc.h"
#include "configkeys.h"
#include "common/settings.h"
#include "common/minmax.h"

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
#include <qfontdialog.h>
#include <qevent.h>

extern App* _app;
extern QApplication* _qapp;

#define X_OFFSET            10
#define Y_OFFSET            5

#define ID_16HZ             16
#define ID_32HZ             32
#define ID_64HZ             64

#define ID_RESIZE_SQUARE    1000
#define ID_CHOOSE_FONT      2000

static const QString KEY_MONITOR_FONT             ( "MonitorFont" );
static const QString KEY_MONITOR_UPDATE_FREQUENCY ( "MonitorUpdateFrequency" );
static const QString KEY_MONITOR_GEOMETRY         ( "MonitorGeometry" );

//
// Constructor
//
Monitor::Monitor(QWidget* parent) : QWidget(parent, "Monitor"),
	m_newValues ( NULL ),
	m_oldValues ( NULL ),
	m_timer ( NULL ),
	m_updateFrequency ( ID_16HZ )
{
}


//
// Destructor
//
Monitor::~Monitor()
{
	saveGeometry();
	
	if (m_timer != NULL)
	{
		delete m_timer;
		m_timer = NULL;
	}

	while (m_painter.isActive());
		
	delete [] m_newValues;
	delete [] m_oldValues;
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
	QString config;

	// Set the window icon
	setIcon(QString(PIXMAPS) + QString("/monitor.png"));

	// Set the window title
	setCaption("DMX Monitor - Universe 1");
	
	//
	// Init the arrays that hold the values
	m_newValues = new t_value[512];
	m_oldValues = new t_value[512];
	
	for (t_channel i = 0; i < 512; i++)
	{
		m_newValues[i] = m_oldValues[i] = 0;
	}

	//
	// Set font
	if (_app->settings()->get(KEY_MONITOR_FONT, config) != -1 && 
		config != "")
	{
		m_font.fromString(config);
	}
	else
	{
		m_font = QApplication::font();
	}

	//
	// Set display update frequency
	if (_app->settings()->get(KEY_MONITOR_UPDATE_FREQUENCY, config) != -1)
	{
		m_updateFrequency = config.toInt();
		if (m_updateFrequency > ID_64HZ)
		{
			m_updateFrequency = ID_64HZ;
		}
		else if (m_updateFrequency < ID_16HZ)
		{
			m_updateFrequency = ID_16HZ;
		}
	}

	// Set window geometry
	loadGeometry();
	
	// Set background color
	setBackgroundColor(colorGroup().base());
	
	//
	// Connect and start the update timer
	//
	connectTimer();
}


//
// Connect and start the update timer
//
void Monitor::connectTimer()
{
	m_timer = new QTimer();
	m_timer->start(1000 / m_updateFrequency);

	connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));
}


//
// Invoke a menu
//
void Monitor::mousePressEvent(QMouseEvent* e)
{
	if (e->button() & RightButton)
	{
		QPopupMenu* menu;
		menu = new QPopupMenu;
		menu->setCheckable(false);

		QPopupMenu* speedMenu;
		speedMenu = new QPopupMenu();
		speedMenu->setCheckable(true);
		speedMenu->insertItem("16Hz", ID_16HZ);
		speedMenu->insertItem("32Hz", ID_32HZ);
		speedMenu->insertItem("64Hz", ID_64HZ);
		speedMenu->setItemChecked(m_updateFrequency, true);
		
		menu->insertItem(QPixmap(QString(PIXMAPS) + 
				 QString("/clock.png")),
				"&Update Speed", speedMenu);
		menu->insertItem(QPixmap(QString(PIXMAPS) + 
				 QString("/fonts.png")),
				"Choose &Font", ID_CHOOSE_FONT);
		
		connect(menu, SIGNAL(activated(int)),
			this, SLOT(slotMenuCallback(int)));
			
		connect(speedMenu, SIGNAL(activated(int)),
			this, SLOT(slotMenuCallback(int)));
		
		menu->exec(mapToGlobal(e->pos()));

		delete menu;
	}
}

//
// Invoke an action from the menu
//
void Monitor::slotMenuCallback(int item)
{
	switch (item)
	{
	case ID_16HZ:
		m_timer->stop();
		m_timer->start(1000 / item);
		m_updateFrequency = item;
		_app->settings()->set(KEY_MONITOR_UPDATE_FREQUENCY, item);
		break;

	case ID_32HZ:
		m_timer->stop();
		m_timer->start(1000 / item);
		m_updateFrequency = item;
		_app->settings()->set(KEY_MONITOR_UPDATE_FREQUENCY, item);
		break;

	case ID_64HZ:
		m_timer->stop();
		m_timer->start(1000 / item);
		m_updateFrequency = item;
		_app->settings()->set(KEY_MONITOR_UPDATE_FREQUENCY, item);
		break;

	case ID_CHOOSE_FONT:
	{
		bool ok = false;
		QFont font = QFontDialog::getFont(&ok, m_font, this);
		if (ok)
		{
			m_font = font;
			_app->settings()->set(KEY_MONITOR_FONT,
						font.toString());
			repaint(true); // Paint all
		}
		break;
	}

	default:
		break;
	}
}


//
// Timer expiration handler
//
void Monitor::slotTimeOut()
{
	_app->outputPlugin()->readRange(0, m_newValues, 512);

	// Paint only changed values
	repaint(false);
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
	short unitW = metrics.width(QString("000")) + X_OFFSET;
	short unitH = metrics.height();
	short unitsX = rect().width() / (unitW);

	// Paint the generic graphics
	QWidget::paintEvent(e);

	// Wait until previous painter is finished
	while (m_painter.isActive());

	// Begin painting
	m_painter.begin(this);

	// Set the font used for drawing text
	m_painter.setFont(m_font);

	if (e->erased())
	{
		// Draw everything that is inside the invalidated region
		paintDeviceLabelAll(e->region(), unitW, unitH, unitsX);
		paintChannelLabelAll(e->region(), unitW, unitH, unitsX);
		paintChannelValueAll(e->region(), unitW, unitH, unitsX, false);
	}
	else
	{
		// Draw only changed values
		paintChannelValueAll(e->region(), unitW, unitH, unitsX, true);
	}

	m_painter.end();
}

//
// Paint all visible device labels
//
void Monitor::paintDeviceLabelAll(QRegion region,
				  short unitW, short unitH, short unitsX)
{
	short x = 0;
	short y = 0;
	short w = 0;
	short wcur = 0;
	short h = 0;
	
	t_device_id id = KNoID;
	Device* dev = NULL;
	
	// Draw device names and their channel spaces
	for (id = 0; id < KDeviceArraySize; id++)
	{
		dev = _app->doc()->device(id);
		if (dev == NULL) continue;
		
		// Calculate x and y positions for this device label
		x = ((dev->address() % unitsX) * unitW);
		y = static_cast<short> 
			(floor((dev->address() / unitsX)) * (unitH * 3));

		// Get width and height for this device label
		w = (dev->deviceClass()->channels()->count() * unitW)
			- X_OFFSET;
		h = unitH;
			
		// Check if this label needs to be painted at all
		if (region.contains(QRect(x, y, w, h)) == false)
			continue;

		if ((x + w + X_OFFSET) <= width())
		{
			// The label fits to one line, just draw it
			paintDeviceLabel(x, y, w, h, dev->name());
		}
		else
		{
			// The label needs to be drawn on at least two lines
			while (w > 0)
			{	
				wcur = min(w, (unitsX * unitW) - (x + X_OFFSET));
				
				// Draw the label
				paintDeviceLabel(x, y, wcur, h, dev->name());
			
				// Calculate remaining width
				w = w - wcur - X_OFFSET;
			
				// Next line
				y += (unitH * 3);
				x = 0;
			}
		}
	}
}

//
// Draw the device label to the given coordinates
//
void Monitor::paintDeviceLabel(int x, int y, int w, int h, QString label)
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
void Monitor::paintChannelLabelAll(QRegion region,
				   short unitW, short unitH, short unitsX)
{
	short x = 0;
	short y = 0;
	short i = 0;

	QString s;
	
	for (i = 0; i < 512; i++)
	{
		s.sprintf("%.3d", i + 1);

		// Calculate x and y positions for this channel
		x = ((i % unitsX) * (unitW));
		y = unitH + static_cast<short> 
			(floor((i / unitsX)) * (unitH * 3));

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
void Monitor::paintChannelLabel(short x, short y, short w, short h, QString s)
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
void Monitor::paintChannelValueAll(QRegion region,
				   short unitW, short unitH, short unitsX,
				   bool onlyDelta)
{
	short x = 0;
	short y = 0;
	short value = 0;
	short i = 0;
	
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
		y = (unitH * 2) + static_cast<short>
			(floor((i / unitsX)) * (unitH * 3));

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
void Monitor::paintChannelValue(short x, short y, short w, short h, QString s)
{
	_qapp->lock();
	m_painter.eraseRect(x, y, w, h);
	m_painter.drawText(x, y, w, h, AlignBottom, s);
	_qapp->unlock();
}

/****************************************************************************
 * Window geometry load/save
 ****************************************************************************/

//
// Load window geometry
//
void Monitor::loadGeometry()
{
	QString s;
	QStringList list;
	QStringList::Iterator it;

	int x = 0;
	int y = 0;
	int w = 400;
	int h = 300;
	
	_app->settings()->get(KEY_MONITOR_GEOMETRY, s);
	list = QStringList::split(',', s);
	
	// X
	it = list.begin();
	if (it != list.end())
		x = (*it).toInt();
	
	// Y
	if (++it != list.end())
		y = (*it).toInt();
	
	// Width
	if (++it != list.end())
		w = (*it).toInt();
	
	// Height
	if (++it != list.end())
		h = (*it).toInt();
	
	// Set window geometry
	setGeometry(x, y, w, h);
}


//
// Save window geometry
//
void Monitor::saveGeometry()
{
	QString s;
	
	s.sprintf("%d,%d,%d,%d", x(), y(), rect().width(), rect().height());
	_app->settings()->set(KEY_MONITOR_GEOMETRY, s);
}

