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

#include <qframe.h>
#include <qptrlist.h>

#include "common/types.h"

class QFile;
class QString;
class QPaintEvent;
class QMouseEvent;
class QDomDocument;
class QDomElement;

#define KXMLQLCVCFrame "Frame"
#define KXMLQLCVCFrameButtonBehaviour "ButtonBehaviour"

class VCFrame : public QFrame
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
 public:
	VCFrame(QWidget* parent);
	virtual ~VCFrame();

	void init(bool bottomFrame = false);

	void destroy();

	/* Check if this is the virtual console's draw area */
	bool isBottomFrame();

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

	/** Invoke a color choosing dialog */
	void chooseBackgroundColor();

	/** Get the widget's background color. The color is invalid if the
	    widget has a background image. */
	const QColor& backgroundColor() { return paletteBackgroundColor(); }

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

	/** Invoke a color choosing dialog */
	void chooseForegroundColor();

	/** Get the widget's foreground color */
	const QColor& foregroundColor() { return paletteForegroundColor(); }

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
	void setCaption(const QString& text);
	void rename();

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

protected:
	bool loadXMLAppearance(QDomDocument* doc, QDomElement* appearance_root);
	bool saveXMLAppearance(QDomDocument* doc, QDomElement* frame_root);

	/*********************************************************************
	 * QLC Mode change
	 *********************************************************************/
protected slots:
	void slotModeChanged();

	/*********************************************************************
	 * Event handlers
	 *********************************************************************/
protected:
	void mousePressEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void paintEvent(QPaintEvent* e);
	void mouseDoubleClickEvent(QMouseEvent* e);
	void customEvent(QCustomEvent* e);

	/*********************************************************************
	 * Widget move & resize
	 *********************************************************************/
protected:
	void resizeTo(QPoint p);
	void moveTo(QPoint p);

protected:
	int m_xpos;
	int m_ypos;

	QPoint m_mousePressPoint;
	bool m_resizeMode;

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
};

#endif
