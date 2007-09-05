/*
  Q Light Controller
  vclabel.h

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

#ifndef VCLABEL_H
#define VCLABEL_H

#include <qlabel.h>
#include <qptrlist.h>

class QLineEdit;
class QMouseEvent;
class QPaintEvent;
class QFile;
class QString;
class QDomDocument;
class QDomElement;

class VCFrame;
class FloatingEdit;

#define KXMLQLCVCLabel "Label"

class VCLabel : public QLabel
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	VCLabel(QWidget* parent);
	~VCLabel();

	void init();

	/*********************************************************************
	 * Background image
	 *********************************************************************/
public:
	/** Set the widget's background image */
	void setBackgroundImage(const QString& path);

	/** Get the widget's background image */
	const QString& backgroundImage();

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

	/** Check, whether the widget has a custom background color */
	bool hasCustomBackgroundColor() { return m_hasCustomBackgroundColor; }

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

	/** Check, whether the widget has a custom foreground color */
	bool hasCustomForegroundColor() const { return m_hasCustomForegroundColor; }

protected:
	bool m_hasCustomBackgroundColor;
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

	/** Check, whether the widget has a custom font */
	bool hasCustomFont() const { return m_hasCustomFont; }

protected:
	bool m_hasCustomFont;

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	static bool loader(QDomDocument* doc, QDomElement* root, QWidget* parent);
	bool loadXML(QDomDocument* doc, QDomElement* root);
	bool saveXML(QDomDocument* doc, QDomElement* vc_root);

public slots:
	void setCaption(const QString&);

private:
	void invokeMenu(QPoint);
	void parseWidgetMenu(int);
	void resizeTo(QPoint);
	void moveTo(QPoint);

	private slots:
	void slotModeChanged();

protected:
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void mouseDoubleClickEvent(QMouseEvent*);
	void paintEvent(QPaintEvent*);
	void customEvent(QCustomEvent*);

private:
	QPoint m_mousePressPoint;
	bool m_resizeMode;
};

#endif

