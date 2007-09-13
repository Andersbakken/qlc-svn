/*
  Q Light Controller
  vcwidget.h
  
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

#ifndef VCWIDGET_H
#define VCWIDGET_H

#include <qframe.h>
#include <qptrlist.h>

#include "common/types.h"

class QFile;
class QString;
class QPopupMenu;
class QPaintEvent;
class QMouseEvent;
class QDomDocument;
class QDomElement;

#define KXMLQLCVCWidgetAppearance "Appearance"
#define KXMLQLCVCWidgetFrameStyle "FrameStyle"
#define KXMLQLCVCWidgetForegroundColor "ForegroundColor"
#define KXMLQLCVCWidgetBackgroundColor "BackgroundColor"
#define KXMLQLCVCWidgetColorDefault "Default"
#define KXMLQLCVCWidgetFont "Font"
#define KXMLQLCVCWidgetFontDefault "Default"
#define KXMLQLCVCWidgetBackgroundImage "BackgroundImage"
#define KXMLQLCVCWidgetBackgroundImageNone "None"

const int KVCWidgetFrameStyleSunken ( QFrame::StyledPanel | QFrame::Sunken );
const int KVCWidgetFrameStyleRaised ( QFrame::StyledPanel | QFrame::Raised );
const int KVCWidgetFrameStyleNone   ( QFrame::NoFrame );

class VCWidget : public QFrame
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
protected:
	/** Protected constructor so nobody makes an instance of this class
	    unless inherited */
	VCWidget(QWidget* parent, const char* name = 0);

public:
	virtual ~VCWidget();

	/** Destroy and delete were already taken, so... */
	virtual void scram();

	/*********************************************************************
	 * Background image
	 *********************************************************************/
public:
	/** Set the widget's background image */
	virtual void setBackgroundImage(const QString& path);

	/** Get the widget's background image */
	virtual const QString& backgroundImage();

	/** Invoke an image choosing dialog */
	virtual void chooseBackgroundImage();

protected:
	QString m_backgroundImage;

	/*********************************************************************
	 * Background color
	 *********************************************************************/
public:
	/** Set the widget's background color and invalidate background image */
	virtual void setBackgroundColor(const QColor& color);

	/** Reset the widget's background color to whatever the platform uses */
	virtual void resetBackgroundColor();

	/** Invoke a color choosing dialog */
	virtual void chooseBackgroundColor();

	/** Get the widget's background color. The color is invalid if the
	    widget has a background image. */
	virtual const QColor& backgroundColor() { return paletteBackgroundColor(); }

	/** Check, whether the widget has a custom background color */
	virtual bool hasCustomBackgroundColor() { return m_hasCustomBackgroundColor; }

protected:
	bool m_hasCustomBackgroundColor;

	/*********************************************************************
	 * Foreground color
	 *********************************************************************/
public:
	/** Set the widget's foreground color */
	virtual void setForegroundColor(const QColor& color);

	/** Reset the widget's background color to whatever the platform uses */
	virtual void resetForegroundColor();

	/** Invoke a color choosing dialog */
	virtual void chooseForegroundColor();

	/** Get the widget's foreground color */
	virtual const QColor& foregroundColor() { return paletteForegroundColor(); }

	/** Check, whether the widget has a custom foreground color */
	virtual bool hasCustomForegroundColor() { return m_hasCustomForegroundColor; }

protected:
	bool m_hasCustomForegroundColor;

	/*********************************************************************
	 * Font
	 *********************************************************************/
public:
	/** Set the font used for the widget's caption */
	virtual void setFont(const QFont& font);

	/** Get the font used for the widget's caption */
	virtual QFont font() { return QWidget::font(); }

	/** Reset the font used for the widget's caption to whatever the
	    platform uses */
	virtual void resetFont();

	/** Invoke a font choosing dialog */
	virtual void chooseFont();

	/** Check, whether the widget has a custom font */
	virtual bool hasCustomFont() { return m_hasCustomFont; }

protected:
	bool m_hasCustomFont;

	/*********************************************************************
	 * Caption
	 *********************************************************************/
public:
	virtual void setCaption(const QString& text);
	virtual void rename();

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	virtual bool loadXML(QDomDocument* doc, QDomElement* vc_root) = 0;
	virtual bool saveXML(QDomDocument* doc, QDomElement* vc_root) = 0;
	
protected:
	bool loadXMLAppearance(QDomDocument* doc, QDomElement* appearance_root);
	bool saveXMLAppearance(QDomDocument* doc, QDomElement* widget_root);

	/*********************************************************************
	 * QLC Mode change
	 *********************************************************************/
protected slots:
	virtual void slotModeChanged();

	/*********************************************************************
	 * Widget menu
	 *********************************************************************/
protected:
	virtual void invokeMenu(QPoint point);
	virtual QPopupMenu* createMenu();

protected slots:
	virtual void slotMenuCallback(int item);

	/*********************************************************************
	 * Widget move & resize
	 *********************************************************************/
public:
	virtual void resize(QPoint p);
	virtual void move(QPoint p);

protected:
	int m_xpos;
	int m_ypos;

	QPoint m_mousePressPoint;
	bool m_resizeMode;

	/*********************************************************************
	 * Frame style converters
	 *********************************************************************/
public:
	static QString frameStyleToString(const int style);
	static int stringToFrameStyle(const QString& style);

	/*********************************************************************
	 * Event handlers
	 *********************************************************************/
protected:
	virtual void paintEvent(QPaintEvent* e);

	virtual void mousePressEvent(QMouseEvent* e);
	virtual void mouseReleaseEvent(QMouseEvent* e);
	virtual void mouseDoubleClickEvent(QMouseEvent* e);
	virtual void mouseMoveEvent(QMouseEvent* e);

};

#endif
