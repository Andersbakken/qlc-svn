/*
  Q Light Controller
  vcframe.h
  
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

#ifndef VCFRAME_H
#define VCFRAME_H

#include "vcwidget.h"
#include "common/qlctypes.h"

class QDomDocument;
class QDomElement;
class QMouseEvent;
class QString;

#define KXMLQLCVCFrame "Frame"
#define KXMLQLCVCFrameButtonBehaviour "ButtonBehaviour"

class VCFrame : public VCWidget
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	VCFrame(QWidget* parent);
	virtual ~VCFrame();

	void init(bool bottomFrame = false);

	/* Check if this is the virtual console's draw area */
	bool isBottomFrame();

public slots:
	/* Delete this widget */
	void slotDelete();

private:
	/** Prevent copying thru operator= or copy constructor since QObject's
	    parental properties get confused when copied. */
	Q_DISABLE_COPY(VCFrame)

	/*********************************************************************
	 * Properties
	 *********************************************************************/
public slots:
	/** Edit this widget's properties */
	void slotProperties();

	/*********************************************************************
	 * Button behaviour
	 *********************************************************************/
public:
	enum ButtonBehaviour
	{
		Normal = 0,
		Exclusive = 1
	};

	/** Set the way buttons behave inside this frame */
	void setButtonBehaviour(ButtonBehaviour);

	/** Get the way buttons behave inside this frame */
	ButtonBehaviour buttonBehaviour() { return m_buttonBehaviour; }

protected:
	ButtonBehaviour m_buttonBehaviour;

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	static bool loader(QDomDocument* doc, QDomElement* root, QWidget* parent);
	bool loadXML(QDomDocument* doc, QDomElement* vc_root);
	bool saveXML(QDomDocument* doc, QDomElement* vc_root);

	/*********************************************************************
	 * Widget menu & actions
	 *********************************************************************/
protected:
	void invokeMenu(QPoint point);

	/*********************************************************************
	 * Widget adding
	 *********************************************************************/
public slots:
	void slotAddButton();
	void slotAddSlider();
	void slotAddXYPad();
	void slotAddCueList();
	void slotAddFrame();
	void slotAddLabel();

	/*********************************************************************
	 * Event handlers
	 *********************************************************************/
protected:
	void paintEvent(QPaintEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
};

#endif
