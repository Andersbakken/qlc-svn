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

#define X_OFFSET            15
#define Y_OFFSET            15

#define ID_CHANNEL_TYPE     1000
#define ID_ABSOLUTE         1010
#define ID_RELATIVE         1020

#define ID_16HZ             16
#define ID_32HZ             32
#define ID_64HZ             64

#define ID_RESIZE_SQUARE    0010

#define ID_CHOOSE_FONT      3000

const QString Monitor::KEY_MONITOR_FONT             (          "MonitorFont" );
const QString Monitor::KEY_MONITOR_DISPLAY_STYLE    (  "MonitorDisplayStyle" );
const QString Monitor::KEY_MONITOR_UPDATE_FREQUENCY ("MonitorUpdateFrequency");

QTimer* Monitor::s_timer         (         NULL );
int Monitor::s_monitors          (            0 );
QMutex* Monitor::s_monitorsMutex ( new QMutex() );
int Monitor::s_updateFrequency   (      ID_16HZ );
int Monitor::s_displayStyle      (  ID_RELATIVE );

Monitor::Monitor(QWidget* parent, t_channel fromChannel, t_channel toChannel)
  : QWidget(parent)
{
  ASSERT(fromChannel <= toChannel);

  m_fromChannel = fromChannel;
  m_toChannel = toChannel;
  m_units = toChannel - fromChannel + 1;

  m_newValues = NULL;
  m_oldValues = NULL;
}


Monitor::~Monitor()
{
  while (m_painter.isActive());

  delete [] m_newValues;
  delete [] m_oldValues;
}


void Monitor::closeEvent(QCloseEvent* e)
{
  disconnectTimer();
  emit closed();
}


void Monitor::init()
{
  QString config;

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
  // Set display style
  if (_app->settings()->get(KEY_MONITOR_DISPLAY_STYLE, config) != -1 &&
	config != "")
  {
	s_displayStyle = config.toInt();
	if (s_displayStyle == 0)
	{
		s_displayStyle = ID_RELATIVE;
	}
	else
	{
		s_displayStyle = ID_ABSOLUTE;
	}
  }
  else
  {
	s_displayStyle = ID_RELATIVE;
  }

  //
  // Set display update frequency
  if (_app->settings()->get(KEY_MONITOR_UPDATE_FREQUENCY, config) != -1)
  {
	s_updateFrequency = config.toInt();
	if (s_updateFrequency > ID_64HZ)
	{
		s_updateFrequency = ID_64HZ;
	}
	else if (s_updateFrequency < ID_16HZ)
	{
		s_updateFrequency = ID_16HZ;
	}
  }

  //
  // Try and resize the window to square shape
  slotResizeSquare();

  //
  // Background color
  setBackgroundColor(Qt::black);

  connectTimer();
}


void Monitor::connectTimer()
{
  s_monitorsMutex->lock();

  if (s_monitors == 0)
    {
      s_timer = new QTimer();
      s_timer->start(1000 / s_updateFrequency);
    }

  connect(s_timer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));
  s_monitors++;

  s_monitorsMutex->unlock();
}


void Monitor::disconnectTimer()
{
  s_monitorsMutex->lock();

  disconnect(s_timer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));
  s_monitors--;

  if (s_monitors == 0)
    {
      s_timer->stop();
      delete s_timer;
      s_timer = NULL;
    }

  s_monitorsMutex->unlock();
}

void Monitor::slotResizeSquare()
{
  QFontMetrics metrics(m_font);

  int unitW = metrics.width(QString("000"));
  int unitH = metrics.height();

  int unitsPerSide = static_cast<int> (ceil(sqrt(m_units)));

  resize(unitsPerSide * (unitW * 2) + X_OFFSET,
	 unitsPerSide * (unitH * 3) + Y_OFFSET);
}

void Monitor::mousePressEvent(QMouseEvent* e)
{
  if (e->button() & RightButton)
    {
      QPopupMenu* menu;
      menu = new QPopupMenu;
      menu->setCheckable(false);

      QPopupMenu* displayMenu;
      displayMenu = new QPopupMenu();
      displayMenu->setCheckable(true);
      displayMenu->insertItem("&Absolute", ID_ABSOLUTE);
      displayMenu->insertItem("&Relative to Device", ID_RELATIVE);
      displayMenu->setItemChecked(s_displayStyle, true);

      QPopupMenu* speedMenu;
      speedMenu = new QPopupMenu();
      speedMenu->setCheckable(true);
      speedMenu->insertItem("16Hz", ID_16HZ);
      speedMenu->insertItem("32Hz", ID_32HZ);
      speedMenu->insertItem("64Hz", ID_64HZ);
      speedMenu->setItemChecked(s_updateFrequency, true);

      menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/monitor.png")),
		       "Channel &Display", displayMenu);
      menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/clock.png")),
		       "&Update Speed", speedMenu);
      menu->insertSeparator();
      menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/window_nofullscreen.png")),
		       "&Resize to Default", ID_RESIZE_SQUARE);
      menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/fonts.png")),
		       "Choose &Font", ID_CHOOSE_FONT);

      connect(menu, SIGNAL(activated(int)),
	      this, SLOT(slotMenuCallback(int)));
      connect(displayMenu, SIGNAL(activated(int)),
	      this, SLOT(slotMenuCallback(int)));
      connect(speedMenu, SIGNAL(activated(int)),
	      this, SLOT(slotMenuCallback(int)));

      menu->exec(mapToGlobal(e->pos()));

      delete displayMenu;
      delete speedMenu;
      delete menu;
    }
}

void Monitor::slotMenuCallback(int item)
{
  switch (item)
    {
    case ID_ABSOLUTE:
      s_displayStyle = ID_ABSOLUTE;
      _app->settings()->set(KEY_MONITOR_DISPLAY_STYLE, s_displayStyle);
      repaint(true);
      break;

    case ID_RELATIVE:
      s_displayStyle = ID_RELATIVE;
      _app->settings()->set(KEY_MONITOR_DISPLAY_STYLE, s_displayStyle);
      repaint(true);
      break;

    case ID_RESIZE_SQUARE:
      slotResizeSquare();
      break;

    case ID_16HZ:
      s_timer->stop();
      s_timer->start(1000 / item);
      s_updateFrequency = item;
      _app->settings()->set(KEY_MONITOR_UPDATE_FREQUENCY, item);
      break;

    case ID_32HZ:
      s_timer->stop();
      s_timer->start(1000 / item);
      s_updateFrequency = item;
      _app->settings()->set(KEY_MONITOR_UPDATE_FREQUENCY, item);
      break;

    case ID_64HZ:
      s_timer->stop();
      s_timer->start(1000 / item);
      s_updateFrequency = item;
      _app->settings()->set(KEY_MONITOR_UPDATE_FREQUENCY, item);
      break;

    case ID_CHOOSE_FONT:
      {
	bool ok;
	QFont font = QFontDialog::getFont(&ok, m_font, this);
	if (ok)
	  {
	    m_font = font;
	    _app->settings()->set(KEY_MONITOR_FONT, font.toString());
	    slotResizeSquare();
	  }
	break;
      }

    default:
      break;
    }
}


void Monitor::slotTimeOut()
{
  _app->outputPlugin()->readRange(m_fromChannel, m_newValues, m_units);

  // Paint only changed values
  repaint(false);
}


void Monitor::paintEvent(QPaintEvent* e)
{
  if (e->erased())
    {
      while (m_painter.isActive()); // Wait until previous painter is finished
      paintAll();// Everything needs to be repainted
    }
  else
    {
      while (m_painter.isActive()); // Wait until previous painter is finished
      paint(); // Paint only changed values
    }

  QWidget::paintEvent(e);
}

void Monitor::paint(void)
{
  QFontMetrics metrics(m_font);
  short unitW = metrics.width(QString("000"));
  short unitH = metrics.height();
  short unitsX = rect().width() / (unitW * 2);

  short x = 0;
  short y = 0;
  short value = 0;

  QString valueString;

  m_painter.begin(this);

  m_painter.setFont(m_font);
  m_painter.setPen(Qt::white);

  for (short i = 0; i < m_units; i++)
    {
      m_mutex.lock(); // Lock access to value array

      if (m_oldValues[i] != m_newValues[i])
	{
	  m_oldValues[i] = m_newValues[i];

	  // Get channel value from array;
	  value = m_newValues[i];
	  m_mutex.unlock(); // Unlock array

	  valueString.sprintf("%.3d", value);

	  // Calculate x and y positions for this channel
	  x = X_OFFSET + ((i % unitsX) * (unitW * 2));
	  y = Y_OFFSET + static_cast<short>
	    (floor((i / unitsX)) * (unitH * 3));

	  _qapp->lock(); // Lock QT to draw only to this widget
	  m_painter.eraseRect(x, y + unitH, unitW, unitH);
	  m_painter.drawText(x, y + unitH, unitW, unitH,
			     AlignBottom, valueString);
	  _qapp->unlock(); // Unlock QT
	}
      else
	{
	  m_mutex.unlock(); // Unlock array
	}
    }

  m_painter.end();
}

void Monitor::paintAll(void)
{
  QFontMetrics metrics(m_font);
  short unitW = metrics.width(QString("000"));
  short unitH = metrics.height();
  short unitsX = rect().width() / (unitW * 2);

  short x = 0;
  short y = 0;

  QString channelString;
  QString valueString;

  m_painter.begin(this);

  m_painter.setFont(m_font);

  for (short i = 0; i < m_units; i++)
    {
      if (s_displayStyle == ID_ABSOLUTE)
	{
	  channelString.sprintf("%.3d", (m_fromChannel & 0x1FF) + i + 1);
	}
      else
	{
	  channelString.sprintf("%.3d", i + 1);
	}

      valueString.sprintf("%.3d", m_newValues[i]);

      // Calculate x and y positions for this channel
      x = X_OFFSET + ((i % unitsX) * (unitW * 2));
      y = Y_OFFSET + static_cast<short> (floor((i / unitsX)) * (unitH * 3));

      _qapp->lock(); // Lock QT to draw only to this widget
      m_painter.setPen(Qt::gray);
      m_painter.eraseRect(x, y, unitW, unitH);
      m_painter.drawText(x, y, unitW, unitH, AlignBottom, channelString);

      m_painter.setPen(Qt::white);
      m_painter.eraseRect(x, y + unitH, unitW, unitH);
      m_painter.drawText(x, y + unitH, unitW, unitH, AlignBottom, valueString);
      _qapp->unlock(); // Unlock QT
    }

  m_painter.end();
}
