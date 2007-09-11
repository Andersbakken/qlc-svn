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
	/** Normal constructor */
	VCLabel(QWidget* parent);

	/** Copy constructor */
	VCLabel(QWidget* parent, VCLabel* label);

	/** Assignment operator */
	VCLabel& operator=(VCLabel& label);

	/** Destructor */
	~VCLabel();

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
	 * Load & Save
	 *********************************************************************/
public:
	static bool loader(QDomDocument* doc, QDomElement* root, QWidget* parent);
	bool loadXML(QDomDocument* doc, QDomElement* root);
	bool saveXML(QDomDocument* doc, QDomElement* vc_root);

protected:
	bool loadXMLAppearance(QDomDocument* doc, QDomElement* appearance_root);
	bool saveXMLAppearance(QDomDocument* doc, QDomElement* label_root);

	/*********************************************************************
	 * QLC Mode change
	 *********************************************************************/
protected slots:
	void slotModeChanged();

	/*********************************************************************
	 * Widget menu
	 *********************************************************************/
protected:
	void invokeMenu(QPoint);

protected slots:
	void slotMenuCallback(int item);

	/*********************************************************************
	 * Widget moving / resizing
	 *********************************************************************/
public:
	void resize(QPoint p);
	void move(QPoint p);

protected:
	QPoint m_mousePressPoint;
	bool m_resizeMode;

	/*********************************************************************
	 * Event handlers
	 *********************************************************************/
protected:
	void paintEvent(QPaintEvent* e);

	void mousePressEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void mouseDoubleClickEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
};

#endif

