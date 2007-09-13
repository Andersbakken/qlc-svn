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
#include "common/types.h"

class QString;
class QMouseEvent;
class QDomDocument;
class QDomElement;

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

	/*********************************************************************
	 * Caption
	 *********************************************************************/
public:
	void setCaption(const QString& text);

	/*********************************************************************
	 * Properties
	 *********************************************************************/
public:
	void editProperties();

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
	 * Widget menu
	 *********************************************************************/
protected:
	void invokeMenu(QPoint point);

protected slots:
	void slotMenuCallback(int item);

	/*********************************************************************
	 * Widget adding
	 *********************************************************************/
public:
	void addButton(QPoint at = QPoint());
	void addSlider(QPoint at = QPoint());
	void addFrame(QPoint at = QPoint());
	void addXYPad(QPoint at = QPoint());
	void addLabel(QPoint at = QPoint());

	/*********************************************************************
	 * Event handlers
	 *********************************************************************/
protected:
	void mousePressEvent(QMouseEvent* e);
	void mouseDoubleClickEvent(QMouseEvent* e);
};

#endif
