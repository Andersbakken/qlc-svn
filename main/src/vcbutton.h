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

#include <qpushbutton.h>
#include <qptrlist.h>

#include "common/types.h"
#include "app.h"

class QLineEdit;
class QMouseEvent;
class QPaintEvent;
class QFile;
class QString;
class QColor;
class QPixmap;
class QPoint;
class QDomDocument;
class QDomElement;

class FloatingEdit;
class KeyBind;

#define KXMLQLCVCButton "Button"

#define KXMLQLCVCButtonFunction "Function"
#define KXMLQLCVCButtonFunctionID "ID"

#define KXMLQLCVCButtonInputChannel "InputChannel"

class VCButton : public QPushButton
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	VCButton(QWidget* parent);
	~VCButton();

	void init();

	/** Destroy and delete were already taken, so... */
	void scram();

	/*********************************************************************
	 * Background image
	 *********************************************************************/
public:
	/** Set the widget's background image */
	void setBackgroundImage(const QString& path);

	/** Get the widget's background image */
	const QString& backgroundImage();

	/** Invoke an image choosing dialog */
	void chooseBackgroundImage();

protected:
	QString m_backgroundImage;

	/*********************************************************************
	 * Background color
	 *********************************************************************/
public:
	/** Set the widget's background color and invalidate background image */
	void setBackgroundColor(const QColor& color);

	/** Reset the widget's background color to whatever the platform uses */
	void resetBackgroundColor();

	/** Get the widget's background color. The color is invalid if the
	    widget has a background image. */
	const QColor& backgroundColor() { return paletteBackgroundColor(); }

	/** Invoke a color choosing dialog */
	void chooseBackgroundColor();

	/** Check, whether the widget has a custom background color */
	bool hasCustomBackgroundColor() { return m_hasCustomBackgroundColor; }

protected:
	bool m_hasCustomBackgroundColor;

	/*********************************************************************
	 * Foreground color
	 *********************************************************************/
public:
	/** Set the widget's foreground color */
	void setForegroundColor(const QColor& color);

	/** Reset the widget's background color to whatever the platform uses */
	void resetForegroundColor();

	/** Get the widget's foreground color */
	const QColor& foregroundColor() { return paletteForegroundColor(); }

	/** Invoke a color choosing dialog */
	void chooseForegroundColor();

	/** Check, whether the widget has a custom foreground color */
	bool hasCustomForegroundColor() const { return m_hasCustomForegroundColor; }

protected:
	bool m_hasCustomForegroundColor;

	/*********************************************************************
	 * Font
	 *********************************************************************/
public:
	/** Set the font used for the widget's caption */
	void setFont(const QFont& font);

	/** Get the font used for the widget's caption */
	QFont font() const { return QWidget::font(); }

	/** Reset the font used for the widget's caption to whatever the
	    platform uses */
	void resetFont();

	/** Invoke a font choosing dialog */
	void chooseFont();

	/** Check, whether the widget has a custom font */
	bool hasCustomFont() const { return m_hasCustomFont; }

protected:
	bool m_hasCustomFont;

	/*********************************************************************
	 * Caption
	 *********************************************************************/
public:
	/** Set this label's caption text */
	void setCaption(const QString& text);

	/** Invoke a renaming edit */
	void rename();

	/*********************************************************************
	 * Properties
	 *********************************************************************/
public:
	void editProperties();

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	static bool loader(QDomDocument* doc, QDomElement* root, QWidget* parent);
	bool loadXML(QDomDocument* doc, QDomElement* root);
	bool saveXML(QDomDocument* doc, QDomElement* vc_root);

protected:
	bool loadXMLAppearance(QDomDocument* doc, QDomElement* root);
	bool saveXMLAppearance(QDomDocument* doc, QDomElement* btn_root);
	
	/*********************************************************************
	 * KeyBind
	 *********************************************************************/
public:
	KeyBind* keyBind() { return m_keyBind; }
	void setKeyBind(const KeyBind* kb);

protected:
	KeyBind* m_keyBind;

	/*********************************************************************
	 * Button press / release handlers
	 *********************************************************************/
public:
	void attachFunction(t_function_id id);
	t_function_id functionID() const { return m_functionID; }

	void setExclusive(bool exclusive = true);
	bool isExclusive() { return m_isExclusive; }

protected slots:
	void slotFlashReady();
	void slotModeChanged(App::Mode mode);

public slots:
	void pressFunction();
	void releaseFunction();

protected:
	t_function_id m_functionID;
	bool m_isExclusive;

	/*********************************************************************
	 * External sliderboard
	 *********************************************************************/
public:
	void setInputChannel(int channel) { m_inputChannel = channel; }
	int inputChannel() const { return m_inputChannel; }

	void setStopFunctions(bool stop) { m_stopFunctions = stop; }
	bool stopFunctions() const { return m_stopFunctions; }

public slots:
	void slotInputEvent(const int,const int,const int);
	void slotFeedBack();

protected:
	int m_inputChannel;
	bool m_stopFunctions;

	/*********************************************************************
	 * Widget menu
	 *********************************************************************/
protected:
	void invokeMenu(QPoint at);

protected slots:
	void slotMenuCallback(int item);

	/*********************************************************************
	 * Widget move & resize
	 *********************************************************************/
public:
	void resize(QPoint p);
	void move(QPoint p);

protected:
	bool moveThreshold(int x, int y);

protected:
	QPoint m_mousePressPoint;
	bool m_resizeMode;

	/*********************************************************************
	 * Event Handlers
	 *********************************************************************/
protected:
	void paintEvent(QPaintEvent* e);

	void mousePressEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void mouseDoubleClickEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);

	void customEvent(QCustomEvent* e);
};

#endif
