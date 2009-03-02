/*
  Q Light Controller
  vcdockarea.h

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

#ifndef VCDOCKAREA_H
#define VCDOCKAREA_H

#include <qframe.h>

class VCDockSlider;
class QShowEvent;
class QHideEvent;

class VCDockArea : public QFrame
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	VCDockArea(QWidget* parent);
	~VCDockArea();

private:
	Q_DISABLE_COPY(VCDockArea)

	/*********************************************************************
	 * Sliders
	 *********************************************************************/
public:
	/** Refresh slider properties */
	void refreshProperties();

protected:
	VCDockSlider* m_fade;
	VCDockSlider* m_hold;

	/*********************************************************************
	 * Event Handlers & Signals
	 *********************************************************************/
protected:
	void showEvent(QShowEvent* event);
	void hideEvent(QHideEvent* event);

signals:
	void visibilityChanged(bool isVisible);
};

#endif
