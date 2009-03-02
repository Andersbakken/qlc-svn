/*
  Q Light Controller
  vcdockarea.cpp

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

#include <QVBoxLayout>
#include <QString>
#include <QDebug>
#include <QtXml>

#include "common/qlcfile.h"

#include "virtualconsoleproperties.h"
#include "virtualconsole.h"
#include "vcdockslider.h"
#include "vcdockarea.h"
#include "bus.h"
#include "app.h"

extern App* _app;

VCDockArea::VCDockArea(QWidget* parent) : QFrame(parent)
{
	/* Align widgets vertically in the area, no margins */
	new QVBoxLayout(this);
	layout()->setMargin(0);

	/* Default fade time slider */
	m_fade = new VCDockSlider(this, KBusIDDefaultFade);
	layout()->addWidget(m_fade);

	/* Default hold time slider */
	m_hold = new VCDockSlider(this, KBusIDDefaultHold);
	layout()->addWidget(m_hold);
}

VCDockArea::~VCDockArea()
{
}

/*****************************************************************************
 * Property refresh
 *****************************************************************************/

void VCDockArea::refreshProperties()
{
	Q_ASSERT(m_fade != NULL);
	Q_ASSERT(m_hold != NULL);

	m_fade->refreshProperties();
	m_hold->refreshProperties();

	if (VirtualConsole::properties().slidersVisible() == true)
		show();
	else
		hide();

}

/*****************************************************************************
 * Event Handlers
 *****************************************************************************/

void VCDockArea::showEvent(QShowEvent* event)
{
	Q_UNUSED(event);
	emit visibilityChanged(true);
}

void VCDockArea::hideEvent(QHideEvent* event)
{
	Q_UNUSED(event);
	emit visibilityChanged(false);
}
