/*
  Q Light Controller
  ledbar.cpp
  
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

#include "ledbar.h"

#include <qapplication.h>
#include <qwidget.h>
#include <qcolor.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qrect.h>
#include <qpixmap.h>
#include <qfont.h>
#include <qtooltip.h>

static const int KLedBarHeight = 150;

LedBar::LedBar(QWidget *parent, QApplication* qapp, const char *name) : QFrame (parent, name)
{
  m_min = 0;
  m_max = 255;
  m_value = 0;

  m_qapp = qapp;

  setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

  m_drawFrame = new QFrame(this);
  m_drawFrame->setGeometry(0, 0, 0, 0);
  m_drawFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
  m_drawFrame->setBackgroundMode(PaletteHighlight);
  m_drawFrame->show();
}

LedBar::~LedBar()
{
  disconnect(this);
}

int LedBar::width()
{
  int w = 0;

  QFontMetrics m = QApplication::fontMetrics();
  
  w = m.width(QString("0000"));
  
  return w;
}

int LedBar::height()
{
  return KLedBarHeight;
}

void LedBar::setRange(int min, int max)
{
  m_min = min;
  m_max = max;

  slotSetValue(m_value);
}

void LedBar::setText(QString text)
{
  m_text = QString(text);
}

void LedBar::setToolTip(QString text)
{
  if (text == QString::null)
    {
      QToolTip::remove(this);
    }
  else
    {
      QToolTip::remove(this);
      QToolTip::add(this, text);
    }
}

void LedBar::slotSetValue(unsigned char value)
{
  slotSetValue((int) value);
}

void LedBar::slotSetValue(int value)
{
  QRect fillRect, chRect, valRect;

  // Update only if value has changed
  if (m_value != value)
    {
      m_qapp->lock();

      m_value = value;

      if (value == 0)
	{
	  m_drawFrame->hide();
	}
      else
	{
	  m_drawFrame->show();

	  // Channel number
	  chRect = rect();
	  chRect.setTop(chRect.bottom() - m_qapp->fontMetrics().height());
	  
	  // Channel value
	  valRect = rect();
	  valRect.setBottom(m_qapp->fontMetrics().height());
	  
	  // Actual led bar
	  fillRect = rect();
	  fillRect.setTop( static_cast<int> (((m_max - value) * (chRect.top() - valRect.bottom()) / (double) m_max) + valRect.bottom()));
	  fillRect.setBottom(chRect.top() - 1);
	  fillRect.setRight(this->rect().width() - 5);
	  fillRect.setLeft(5);
	  
	  m_drawFrame->setGeometry(fillRect);
	}
      
      repaint();

      m_qapp->unlock();
    }
}

void LedBar::paintEvent(QPaintEvent *e)
{
  //  slotSetValue(m_value);
  paint();
  QFrame::paintEvent(e);
}

void LedBar::paint(void)
{
  QPainter painter(this);
  QRect txtRect;
  QRect valRect;
  QString value;

  double val = ((double) m_value / (double) m_max) * (double) 255;
  value.sprintf("%.3d", (int) val);

  // Text rect
  txtRect = rect();
  txtRect.setRight(this->rect().width() - 1);
  txtRect.setTop(txtRect.bottom() - m_qapp->fontMetrics().height());
  txtRect.setLeft(1);

  // Value rect
  valRect = rect();
  valRect.setRight(this->rect().width() - 1);
  valRect.setBottom(valRect.top() + m_qapp->fontMetrics().height());
  valRect.setLeft(1);

  // Set black pen for text
  painter.setPen(Qt::black);

  // Draw info text
  painter.drawText(txtRect, AlignCenter, m_text);

  // Draw value text
  painter.drawText(valRect, AlignCenter, value);
}
