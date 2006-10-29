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
#include "common/settings.h"
#include "configkeys.h"

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

#define X_OFFSET            5
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
	m_fromChannel ( 0 ),
	m_toChannel ( 511 ),
	m_units ( 512 ),
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
	m_newValues = new t_value[m_units];
	m_oldValues = new t_value[m_units];
	
	for (t_channel i = 0; i < m_units; i++)
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
	_app->outputPlugin()->readRange(m_fromChannel, m_newValues, m_units);

	// Paint only changed values
	repaint(false);
}

//
// Paint event has been received
//
void Monitor::paintEvent(QPaintEvent* e)
{
	// Paint the generic graphics
	QWidget::paintEvent(e);

	if (e->erased())
	{
		// Wait until previous painter is finished
		while (m_painter.isActive());
		
		// Everything needs to be repainted
		paintAll(e);
	}
	else
	{
		// Wait until previous painter is finished
		while (m_painter.isActive());
		
		// Paint only changed values
		paint(e);
	}
}

//
// Paint only changed channel values
//
void Monitor::paint(QPaintEvent* e)
{
	QFontMetrics metrics(m_font);
	short unitW = metrics.width(QString("000")) + X_OFFSET;
	short unitH = metrics.height();
	short unitsX = rect().width() / (unitW);
	
	short x = 0;
	short y = 0;
	short value = 0;
	
	QString valueString;
	
	m_painter.begin(this);
	
	m_painter.setFont(m_font);
	m_painter.setPen(colorGroup().text());
	
	for (short i = 0; i < m_units; i++)
	{
		m_valueMutex.lock();
	
		if (m_oldValues[i] != m_newValues[i])
		{
			m_oldValues[i] = m_newValues[i];

			// Get channel value from array;
			value = m_newValues[i];
			m_valueMutex.unlock(); // Unlock array
		
			valueString.sprintf("%.3d", value);
		
			// Calculate x and y positions for this channel
			x = ((i % unitsX) * (unitW));
			y = unitH + static_cast<short> 
			    (floor((i / unitsX)) * (unitH * 3));

			// Draw only those values that are visible
			if (y < height())
			{
				_qapp->lock();
				m_painter.eraseRect(x, y + unitH, unitW, unitH);
				m_painter.drawText(x, y + unitH, unitW, unitH,
						AlignBottom, valueString);
				_qapp->unlock();
			}
			else
			{
				m_valueMutex.unlock();
				
				// Rest of the values are not visible either
				break;
			}
		}
		else
		{
			m_valueMutex.unlock();
		}
	}

	m_painter.end();
}

//
// Paint everything
//
void Monitor::paintAll(QPaintEvent* e)
{
	QFontMetrics metrics(m_font);
	short unitW = metrics.width(QString("000")) + X_OFFSET;
	short unitH = metrics.height();
	short unitsX = rect().width() / (unitW);

	short x = 0;
	short y = 0;
	short w = 0;
	short h = 0;
	
	QString channelString;
	QString valueString;
	
	t_device_id id = KNoID;
	Device* dev = NULL;
	
	m_painter.begin(this);
	m_painter.setFont(m_font);
	
	// Draw device names and their channel spaces
	for (id = 0; id < KDeviceArraySize; id++)
	{
		dev = _app->doc()->device(id);
		if (dev == NULL)
		{
			continue;
		}
		else
		{
			// Calculate x and y positions for this device label
			x = ((dev->address() % unitsX) * unitW);
			y = static_cast<short> 
			    (floor((dev->address() / unitsX)) * (unitH * 3));

			w = (dev->deviceClass()->channels()->count() * unitW)
				- X_OFFSET;
			h = unitH;

			if (e->region().contains(QRect(x, y, w, h)))
			{
				_qapp->lock();
			
				m_painter.fillRect(x, y, w, h, 
						colorGroup().highlight());
			
				m_painter.setPen(colorGroup().shadow());
				m_painter.drawRect(x, y, w, h);
			
				m_painter.setPen(colorGroup().highlightedText());
				m_painter.drawText(x + X_OFFSET, y,
						w, h, 
						AlignLeft | AlignVCenter,
						dev->name());
				_qapp->unlock();
			}
		}
	}
		
	for (short i = 0; i < m_units; i++)
	{
		channelString.sprintf("%.3d", (m_fromChannel & 0x1FF) + i + 1);
		valueString.sprintf("%.3d", m_newValues[i]);

		// Calculate x and y positions for this channel
		x = ((i % unitsX) * (unitW));
		y = unitH + static_cast<short> 
		    (floor((i / unitsX)) * (unitH * 3));

		if (e->region().contains(QRect(x, y, unitW, unitH * 3)))
		{
			_qapp->lock();
		
			// Draw channel number text
			m_painter.setPen(colorGroup().dark());
			m_painter.eraseRect(x, y, unitW, unitH);
			m_painter.drawText(x, y, unitW, unitH,
					AlignBottom,
					channelString);

			// Draw value text
			m_painter.setPen(colorGroup().text());
			m_painter.eraseRect(x, y + unitH, unitW, unitH);
			m_painter.drawText(x, y + unitH, unitW, unitH,
					AlignBottom,
					valueString);
		
			_qapp->unlock();
		}
	}

	m_painter.end();
}


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

