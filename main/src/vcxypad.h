/*
  Q Light Controller
  vcxypad.h
  
  Copyright (c) Heikki Junnila, Stefan Krumm
  
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

#ifndef VCXYPAD_H
#define VCXYPAD_H

#include <qframe.h>
#include <qptrlist.h>
#include <qpixmap.h>

#include "common/types.h"

class QFile;
class QString;
class QPaintEvent;
class QMouseEvent;
class QDomDocument;
class QDomElement;

class XYChannelUnit;

#define KXMLQLCVCXYPad "XYPad"
#define KXMLQLCVCXYPadPosition "Position"
#define KXMLQLCVCXYPadPositionX "X"
#define KXMLQLCVCXYPadPositionY "Y"

#define KXMLQLCVCXYPadChannel "Channel"
#define KXMLQLCVCXYPadChannelAxis "Axis"
#define KXMLQLCVCXYPadChannelAxisX "X"
#define KXMLQLCVCXYPadChannelAxisY "Y"

#define KXMLQLCVCXYPadChannelFixture "Fixture"
#define KXMLQLCVCXYPadChannelLowLimit "LowLimit"
#define KXMLQLCVCXYPadChannelHighLimit "HighLimit"
#define KXMLQLCVCXYPadChannelReverse "Reverse"

class VCXYPad : public QFrame
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
 public:
	VCXYPad(QWidget* parent);
	virtual ~VCXYPad();

	void init();

	void destroy();

	/*********************************************************************
	 * Background image
	 *********************************************************************/
public:
	/** Set the widget's background image */
	void setBackgroundImage(const QString& path);
	
	/** Get the widget's background image */
	const QString& backgroundImage();
	
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

	void chooseBackgroundColor();

	/** Get the widget's background color. The color is invalid if the
	    widget has a background image. */
	const QColor& backgroundColor() const { return paletteBackgroundColor(); }

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

	void chooseForegroundColor();

	/** Get the widget's foreground color */
	const QColor& foregroundColor() const { return paletteForegroundColor(); }

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

	void chooseFont();

	/** Reset the font used for the widget's caption to whatever the
	    platform uses */
	void resetFont();

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
	 * Channels
	 *********************************************************************/
public:
	/* Append a new channel to this pad */
	void appendChannel(t_axis axis, t_fixture_id fixture, t_channel channel,
			   t_value lowLimit, t_value highLimit, bool reverse);

	/* Remove a channel from this pad */
	void removeChannel(t_axis axis, t_fixture_id fixture, t_channel channel);

	/* Get a certain channel from this pad */
	XYChannelUnit* channel(t_axis axis, t_fixture_id fixture,
			       t_channel channel);
	
	/* Clear all channel lists from this pad */
	void clearChannels();

	QPtrList<XYChannelUnit>* channelsX() { return &m_channelsX; }
	QPtrList<XYChannelUnit>* channelsY() { return &m_channelsY; }

protected:  
	QPtrList<XYChannelUnit> m_channelsX;
	QPtrList<XYChannelUnit> m_channelsY;

	/*********************************************************************
	 * Current position
	 *********************************************************************/
public:
	const QPoint currentXYPosition() const { return m_currentXYPosition; }
	void setCurrentXYPosition(const QPoint& point);
	void setCurrentXYPosition(int x, int y);

protected:
	QPoint m_currentXYPosition;
	QPixmap m_xyPosPixmap;

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	static bool loader(QDomDocument* doc, QDomElement* root, QWidget* parent);
	bool loadXML(QDomDocument* doc, QDomElement* root);
	bool saveXML(QDomDocument* doc, QDomElement* root);

protected:
	bool loadXMLAppearance(QDomDocument* doc, QDomElement* appearance_root);
	bool saveXMLAppearance(QDomDocument* doc, QDomElement* xypad_root);

	/*********************************************************************
	 * QLC Mode change
	 *********************************************************************/
protected slots:
	void slotModeChanged();

	/*********************************************************************
	 * DMX writer
	 *********************************************************************/
protected:
	void outputDMX(int x, int y);

	/*********************************************************************
	 * Widget menu
	 *********************************************************************/
protected:
	void invokeMenu(QPoint point);

protected slots:
	void slotMenuCallback(int item);

	/*********************************************************************
	 * Event handlers
	 *********************************************************************/
protected:
	void mousePressEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void paintEvent(QPaintEvent* e);
	void customEvent(QCustomEvent* e);

	/*********************************************************************
	 * Widget move / resize
	 *********************************************************************/
protected:
	void resizeTo(QPoint p);
	void moveTo(QPoint p);

protected:
	int m_xpos;
	int m_ypos;

	QPoint m_mousePressPoint;
	bool m_resizeMode;
};

#endif
