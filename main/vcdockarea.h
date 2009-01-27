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

class QDomDocument;
class QDomElement;
class QShowEvent;
class QHideEvent;

class VCDockSlider;

#define KXMLQLCVCDockArea "DockArea"
#define KXMLQLCVCDockAreaVisible "Visible"

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
	 * Load & Save
	 *********************************************************************/
public:
	/**
	 * Load this slider's settings
	 *
	 * @param doc An XML document to load from
	 * @param root A VCDockSlider XML root node to load from
	 */
	bool loadXML(QDomDocument* doc, QDomElement* root);

	/**
	 * Save this slider's settings
	 *
	 * @param doc An XML document to save to
	 * @param vc_root VirtualConsole XML root node to save to
	 */
	bool saveXML(QDomDocument* doc, QDomElement* vc_root);

	/*********************************************************************
	 * Event Handlers & Signals
	 *********************************************************************/
protected:
	void showEvent(QShowEvent* event);
	void hideEvent(QHideEvent* event);

signals:
	void visibilityChanged(bool isVisible);

	/*********************************************************************
	 * Widgets
	 *********************************************************************/
public:
	VCDockSlider* defaultFadeSlider() { return m_defaultFadeSlider; }

protected:
	VCDockSlider* m_defaultFadeSlider;
};

#endif
