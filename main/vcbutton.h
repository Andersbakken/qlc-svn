/*
  Q Light Controller
  vcbutton.h
  
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

#ifndef VCBUTTON_H
#define VCBUTTON_H

#include "vcwidget.h"
#include "common/qlctypes.h"

class QDomDocument;
class QDomElement;
class QMouseEvent;
class QPaintEvent;
class QPoint;
class QEvent;

class KeyBind;

#define KXMLQLCVCButton "Button"

#define KXMLQLCVCButtonFunction "Function"
#define KXMLQLCVCButtonFunctionID "ID"

#define KXMLQLCVCButtonInputChannel "InputChannel"

class VCButton : public VCWidget
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	VCButton(QWidget* parent);
	~VCButton();

public slots:
	/** Delete this widget */
	void slotDelete();

	/*********************************************************************
	 * Properties
	 *********************************************************************/
public slots:
	/** Edit this widget's properties */
	void slotProperties();

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	/**
	 * Create and load a new VCButton from an XML document
	 *
	 * @param doc An XML document to load from
	 * @param btn_root A VCButton XML root node
	 * @param parent A parent VCFrame for the new VCButton
	 * @return true if successful; otherwise false
	 */
	static bool loader(QDomDocument* doc, QDomElement* btn_root,
			   QWidget* parent);

	/**
	 * Load a VCButton's properties from an XML document node
	 *
	 * @param doc An XML document to load from
	 * @param btn_root A VCButton XML root node containing button properties
	 * @return true if successful; otherwise false
	 */
	bool loadXML(QDomDocument* doc, QDomElement* btn_root);

	/**
	 * Save a VCButton's properties to an XML document node
	 *
	 * @param doc The master XML document to save to
	 * @param frame_root The button's VCFrame XML parent node to save to
	 */
	bool saveXML(QDomDocument* doc, QDomElement* frame_root);

	/*********************************************************************
	 * Button state
	 *********************************************************************/
public:
	void setOn(bool on);
	bool isOn() const { return m_on; }

	void setFrameStyle(const int style);

protected:
	bool m_on;

	/*********************************************************************
	 * KeyBind
	 *********************************************************************/
public:
	/**
	 * Get the button's key binding object
	 */
	KeyBind* keyBind() { return m_keyBind; }

	/**
	 * Set the button's key binding object. This only makes a copy of the
	 * object's contents.
	 *
	 * @param kb A key binding object
	 */
	void setKeyBind(const KeyBind* kb);

protected:
	/** 
	 * This button's key binding object. This is present even when a
	 * button does not have a key binding.
	 */
	KeyBind* m_keyBind;

	/*********************************************************************
	 * Function attachment
	 *********************************************************************/
public:
	/**
	 * Attach a function to a VCButton. This function is started when the
	 * button is pressed down.
	 *
	 * @param function An ID of a function to attach
	 */
	void setFunction(t_function_id function);

	/**
	 * Get the ID of the function attached to a VCButton
	 *
	 * @return The ID of the attached function or KNoID if there isn't one
	 */
	t_function_id function() const { return m_function; }

	/**
	 * Set the button to behave exclusively inside its VCFrame parent.
	 * An exclusive button will, when pressed down, stop the functions
	 * attached to other VCButtons in the same frame.
	 *
	 * @param exclusive true to set exclusive, otherwise false
	 */
	void setExclusive(bool exclusive = true);

	/**
	 * Get the button's exclusive status. 
	 * See @ref setExclusive() for disambiguation.
	 *
	 * @return true if button is exclusive, false if it is not
	 */
	bool isExclusive() { return m_isExclusive; }

protected:
	/** The function that this button is controlling */
	t_function_id m_function;

	/** Exclusive status */
	bool m_isExclusive;

	/*********************************************************************
	 * Button press / release handlers
	 *********************************************************************/
public slots:
	/**
	 * Handler for function running signal
	 */
	void slotFunctionRunning(t_function_id fid);

	/**
	 * Handler for function stop signal
	 */
	void slotFunctionStopped(t_function_id fid);

	/**
	 * Handler for button press i.e. (mouse/key)button down, not click.
	 */
	void pressFunction();

	/**
	 * Handler for button release i.e. (mouse/key)button up, not click.
	 */
	void releaseFunction();

protected slots:
	/**
	 * Slot for brief widget flashing when the controlled function has
	 * (been) stopped
	 */
	void slotFlashReady();

	/*********************************************************************
	 * Stop functions
	 *********************************************************************/
public:
	/** Set this button to operate as a panic button */
	void setStopFunctions(bool stop) { m_stopFunctions = stop; }

	/** Check, whether this button should act as a panic button */
	bool stopFunctions() const { return m_stopFunctions; }

protected:
	/** Does this button act as a panic button or not */
	bool m_stopFunctions;

	/*********************************************************************
	 * Widget menu
	 *********************************************************************/
protected:
	/** Invoke the widget menu */
	void invokeMenu(QPoint point);

protected slots:
	/**
	 * Invoke a function selection dialog to set (attach) a function to
	 * this button.
	 */
	void slotAttachFunction();

	/*********************************************************************
	 * Event Handlers
	 *********************************************************************/
protected:
	void paintEvent(QPaintEvent* e);

	void mousePressEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
};

#endif
