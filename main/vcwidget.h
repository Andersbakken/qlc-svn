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

#include <QFrame>

#include "common/qlctypes.h"
#include "app.h"

class QDomDocument;
class QDomElement;
class QPaintEvent;
class QMouseEvent;
class QString;
class QMenu;
class QFile;

#define KXMLQLCVCWidgetAppearance "Appearance"
#define KXMLQLCVCWidgetFrameStyle "FrameStyle"

#define KXMLQLCVCWidgetForegroundColor "ForegroundColor"
#define KXMLQLCVCWidgetBackgroundColor "BackgroundColor"
#define KXMLQLCVCWidgetColorDefault "Default"

#define KXMLQLCVCWidgetFont "Font"
#define KXMLQLCVCWidgetFontDefault "Default"

#define KXMLQLCVCWidgetBackgroundImage "BackgroundImage"
#define KXMLQLCVCWidgetBackgroundImageNone "None"

#define KXMLQLCVCWidgetInput "Input"
#define KXMLQLCVCWidgetInputUniverse "Universe"
#define KXMLQLCVCWidgetInputChannel "Channel"

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
	VCWidget(QWidget* parent);

public:
	virtual ~VCWidget();

public slots:
	virtual void slotCut();
	virtual void slotCopy();
	virtual void slotPaste();

	/** Delete this widget */
	virtual void slotDelete();

	/*********************************************************************
	 * Background image
	 *********************************************************************/
public:
	/** Set the widget's background image */
	virtual void setBackgroundImage(const QString& path);

	/** Get the widget's background image */
	virtual const QString& backgroundImage() { return m_backgroundImage; }

public slots:
	/** Invoke an image choosing dialog */
	virtual void slotChooseBackgroundImage();

protected:
	QString m_backgroundImage;

	/*********************************************************************
	 * Background color
	 *********************************************************************/
public:
	/** Set the widget's background color and invalidate background image */
	virtual void setBackgroundColor(const QColor& color);

	/** Get the widget's background color. The color is invalid if the
	    widget has a background image. */
	virtual const QColor& backgroundColor() {
		return palette().color(QPalette::Window); }

	/** Check, whether the widget has a custom background color */
	virtual bool hasCustomBackgroundColor() {
		return m_hasCustomBackgroundColor; }

public slots:
	/** Invoke a color choosing dialog */
	virtual void slotChooseBackgroundColor();

	/** Reset the widget's background color to whatever the platform uses */
	virtual void slotResetBackgroundColor();

protected:
	bool m_hasCustomBackgroundColor;

	/*********************************************************************
	 * Foreground color
	 *********************************************************************/
public:
	/** Set the widget's foreground color */
	virtual void setForegroundColor(const QColor& color);

public:
	/** Get the widget's foreground color */
	virtual const QColor& foregroundColor() { 
		return palette().color(QPalette::WindowText); }

	/** Check, whether the widget has a custom foreground color */
	virtual bool hasCustomForegroundColor() {
		return m_hasCustomForegroundColor; }

public slots:
	/** Invoke a color choosing dialog */
	virtual void slotChooseForegroundColor();

	/** Reset the widget's background color to whatever the platform uses */
	virtual void slotResetForegroundColor();

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

	/** Check, whether the widget has a custom font */
	virtual bool hasCustomFont() { return m_hasCustomFont; }

public slots:
	/** Invoke a font choosing dialog */
	virtual void slotChooseFont();

	/** Reset the font used for the widget's caption to whatever the
	    platform uses */
	virtual void slotResetFont();

protected:
	bool m_hasCustomFont;

	/*********************************************************************
	 * Caption
	 *********************************************************************/
public:
	/** Set this widget's caption text */
	virtual void setCaption(const QString& text);

	/** Get this widget's caption text */
	virtual QString caption() const { return windowTitle(); }

public slots:
	/** Invoke a dialog to rename this widget */
	virtual void slotRename();

	/*********************************************************************
	 * Frame style
	 *********************************************************************/
public slots:
	virtual void slotSetFrameSunken();
	virtual void slotSetFrameRaised();
	virtual void slotResetFrame();

	/*********************************************************************
	 * Stacking
	 *********************************************************************/
public slots:
	virtual void raise();
	virtual void lower();

	/*********************************************************************
	 * Properties
	 *********************************************************************/
public slots:
	virtual void slotProperties();

	/*********************************************************************
	 * External input
	 *********************************************************************/
public:
	/** Set external input universe & channel number to listen to */
	virtual void setInputSource(t_input_universe universe,
				    t_input_channel channel);

	/** Get the assigned external input universe */
	virtual t_input_universe inputUniverse() const;

	/** Get the assigned external input channel within inputUniverse() */
	virtual t_input_channel inputChannel() const;

protected:
	t_input_universe m_inputUniverse;
	t_input_channel m_inputChannel;

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
	virtual void slotModeChanged(App::Mode mode);

signals:
	void modeChanged(App::Mode);

	/*********************************************************************
	 * Widget menu
	 *********************************************************************/
protected:
	virtual void invokeMenu(QPoint point);
	virtual QMenu* createMenu();

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
