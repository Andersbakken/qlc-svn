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
#include "settings.h"
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

#define ID_FREQUENCY        2000
#define ID_16HZ             ID_FREQUENCY + 16
#define ID_32HZ             ID_FREQUENCY + 32
#define ID_64HZ             ID_FREQUENCY + 64

#define ID_RESIZE_SQUARE    0010

#define ID_CHOOSE_FONT      3000

const QString Monitor::KEY_MONITOR_FONT = "MonitorFont";
const QString Monitor::KEY_MONITOR_DISPLAY_STYLE = "MonitorDisplayStyle";
const QString Monitor::KEY_MONITOR_UPDATE_FREQUENCY = "MonitorUpdateFrequency";

Monitor::Monitor(QWidget* parent, t_channel fromChannel,
		 t_channel toChannel)
  : QWidget(parent)
{
  ASSERT(fromChannel <= toChannel);

  m_fromChannel = fromChannel;
  m_toChannel = toChannel;
  m_units = toChannel - fromChannel + 1;

  m_newValues = NULL;
  m_oldValues = NULL;

  m_displayStyle = ID_RELATIVE;
  m_updateFrequency = ID_64HZ;
}

Monitor::~Monitor()
{
  m_timer->stop();
  delete m_timer;

  while (m_painter.isActive());

  delete [] m_newValues;
  delete [] m_oldValues;
}


void Monitor::closeEvent(QCloseEvent* e)
{
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
  // Init the timer that updates the values
  m_timer = new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));

  //
  // Set font
  _app->settings()->get(KEY_MONITOR_FONT, config);
  
  if (config != QString::null)
    {
      m_font.fromString(config);
    }

  //
  // Set display style
  _app->settings()->get(KEY_MONITOR_DISPLAY_STYLE, config);
  m_displayStyle = config.toInt();

  //
  // Set display update frequency
  _app->settings()->get(KEY_MONITOR_UPDATE_FREQUENCY, config);
  m_updateFrequency = config.toInt() + ID_FREQUENCY;

  //
  // Try and resize the window to square shape
  slotResizeSquare();

  //
  // Background color
  setBackgroundColor(Qt::black);
}

void Monitor::show()
{
  m_timer->start(16);

  QWidget::show();
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
      QString dir;
      _app->settings()->get(KEY_SYSTEM_DIR, dir);
      dir += QString("/") + PIXMAPPATH;

      QPopupMenu* menu;
      menu = new QPopupMenu;
      menu->setCheckable(false);
      
      QPopupMenu* displayMenu;
      displayMenu = new QPopupMenu();
      displayMenu->setCheckable(true);
      displayMenu->insertItem("&Absolute", ID_ABSOLUTE);
      displayMenu->insertItem("&Relative to Device", ID_RELATIVE);
      displayMenu->setItemChecked(m_displayStyle, true);

      QPopupMenu* speedMenu;
      speedMenu = new QPopupMenu();
      speedMenu->setCheckable(true);
      speedMenu->insertItem("16Hz", ID_16HZ);
      speedMenu->insertItem("32Hz", ID_32HZ);
      speedMenu->insertItem("64Hz", ID_64HZ);
      speedMenu->setItemChecked(m_updateFrequency, true);

      menu->insertItem(QPixmap(dir + "/monitor.xpm"), 
		       "Channel &Display", displayMenu);
      menu->insertItem(QPixmap(dir + "/clock.xpm"), 
		       "&Update Speed", speedMenu);
      menu->insertSeparator();
      menu->insertItem(QPixmap(dir + "/move.xpm"), 
		       "&Resize to Default", ID_RESIZE_SQUARE);
      menu->insertItem(QPixmap(dir + "/rename.xpm"), 
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
      m_displayStyle = ID_ABSOLUTE;
      _app->settings()->set(KEY_MONITOR_DISPLAY_STYLE, m_displayStyle);
      repaint(true);
      break;
      
    case ID_RELATIVE:
      m_displayStyle = ID_RELATIVE;
      _app->settings()->set(KEY_MONITOR_DISPLAY_STYLE, m_displayStyle);
      repaint(true);
      break;

    case ID_RESIZE_SQUARE:
      slotResizeSquare();
      break;

    case ID_16HZ:
      m_timer->stop();
      m_timer->start(1000 / (item - ID_FREQUENCY));
      m_updateFrequency = item;
      _app->settings()->set(KEY_MONITOR_UPDATE_FREQUENCY,
			    item - ID_FREQUENCY);
      break;

    case ID_32HZ:
      m_timer->stop();
      m_timer->start(1000 / (item - ID_FREQUENCY));
      m_updateFrequency = item;
      _app->settings()->set(KEY_MONITOR_UPDATE_FREQUENCY, 
			    item - ID_FREQUENCY);
      break;

    case ID_64HZ:
      m_timer->stop();
      m_timer->start(1000 / (item - ID_FREQUENCY));
      m_updateFrequency = item;
      _app->settings()->set(KEY_MONITOR_UPDATE_FREQUENCY, 
			    item - ID_FREQUENCY);
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
      if (m_displayStyle == ID_ABSOLUTE)
	{
	  channelString.sprintf("%.3d", m_fromChannel + i);
	}
      else
	{
	  channelString.sprintf("%.3d", i);
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
