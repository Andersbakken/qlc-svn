/*
  Q Light Controller
  vcdockslider.h

  Copyright () Heikki Junnila

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

#ifndef VCDOCKSLIDER_H
#define VCDOCKSLIDER_H

#include <qptrlist.h>
#include <qstring.h>
#include <qdatetime.h>

#include "common/types.h"
#include "uic_vcdockslider.h"

class QMouseEvent;
class QFile;
class QPoint;
class QDomDocument;
class QDomElement;

class SliderKeyBind;

#define KXMLQLCVCDockSlider "Slider"
#define KXMLQLCVCDockSliderMode "Mode"
#define KXMLQLCVCDockSliderValue "Value"
#define KXMLQLCVCDockSliderDMXChannel "DMXChannel"
#define KXMLQLCVCDockSliderInputChannel "InputChannel"

#define KXMLQLCVCDockSliderBus "Bus"
#define KXMLQLCVCDockSliderBusID "ID"
#define KXMLQLCVCDockSliderBusLowLimit "LowLimit"
#define KXMLQLCVCDockSliderBusHighLimit "HighLimit"

#define KXMLQLCVCDockSliderLevel "Level"
#define KXMLQLCVCDockSliderLevelLowLimit "LowLimit"
#define KXMLQLCVCDockSliderLevelHighLimit "HighLimit"

const QString KEY_DEFAULT_FADE_MIN ( "DefaultFadeMin" );
const QString KEY_DEFAULT_FADE_MAX ( "DefaultFadeMax" );
const QString KEY_DEFAULT_HOLD_MIN ( "DefaultHoldMin" );
const QString KEY_DEFAULT_HOLD_MAX ( "DefaultHoldMax" );

class VCDockSlider : public UI_VCDockSlider
{
	Q_OBJECT
    
	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	VCDockSlider(QWidget* parent, bool isStatic = false);
	~VCDockSlider();

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

	/** Invoke a color choosing dialog */
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

	/** Invoke a color choosing dialog */
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
	void setCaption(const QString&);
	void rename();

	/*********************************************************************
	 * Properties
	 *********************************************************************/
public:
	void editProperties();

	/*********************************************************************
	 * Key binding
	 *********************************************************************/
public:
	SliderKeyBind* sliderKeyBind() const { return m_sliderKeyBind; }
	void setSliderKeyBind(const SliderKeyBind* skb);

protected:
	SliderKeyBind* m_sliderKeyBind;

	/*********************************************************************
	 * Input channel
	 *********************************************************************/
public:
	void setInputChannel(t_channel channel) { m_inputChannel = channel; }
	int inputChannel() const { return m_inputChannel; }

protected:
	int m_inputChannel;

	/*********************************************************************
	 * SliderMode
	**********************************************************************/
public:
	enum SliderMode
	{
		Speed  = 0,
		Level = 1,
		Submaster = 2
	};
  
	void setMode(SliderMode m);
	SliderMode mode() { return m_mode; }
	static QString modeToString(const SliderMode mode);
	static SliderMode stringToMode(const QString& str);

protected:
	SliderMode m_mode;

	/*********************************************************************
	 * Bus
	 *********************************************************************/
public:
	bool setBusID(t_bus_id id);
	t_bus_id busID() const { return m_busID; }
	void setBusRange(t_bus_value lo, t_bus_value hi);
	void busRange(t_bus_value &lo, t_bus_value &hi);

public slots:
	void slotBusNameChanged(t_bus_id, const QString&);
	void slotBusValueChanged(t_bus_id, t_bus_value);

protected:
	t_bus_id m_busID;
	t_bus_value m_busLowLimit;
	t_bus_value m_busHighLimit;

	/*********************************************************************
	 * Level & Submaster
	 *********************************************************************/
public:
	QValueList <t_channel>* channels() { return &m_channels; }
	void assignSubmasters(bool assign);

	void setLevelRange(t_value low, t_value hi);
	void levelRange(t_value& lo, t_value& hi);

	void appendChannel(t_channel channel);
	void removeChannel(t_channel channel);
	void clearChannels();
	
protected:
	QValueList<t_channel> m_channels;
	int m_levelLowLimit;
	int m_levelHighLimit;

	/*********************************************************************
	 * Keyboard input
	 *********************************************************************/
public slots:
	void slotPressUp();
	void slotPressDown();

	/*********************************************************************
	 * External sliderboard input
	 *********************************************************************/
public slots:
	void slotFeedBack();
	void slotInputEvent(const int, const int, const int);

	/*********************************************************************
	 * Slider dragging & tap button clicks
	 *********************************************************************/
protected slots:
	void slotSliderValueChanged(const int);
	void slotTapInButtonClicked();

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	static bool loader(QDomDocument* doc, QDomElement* root, QWidget* parent);
	bool loadXML(QDomDocument* doc, QDomElement* root);
	bool saveXML(QDomDocument* doc, QDomElement* vc_root);

protected:
	bool loadXMLAppearance(QDomDocument* doc, QDomElement* appearance_root);
	bool saveXMLAppearance(QDomDocument* doc, QDomElement* slider_root);

	/*********************************************************************
	 * QLC Mode change
	 *********************************************************************/
protected slots:
	void slotModeChanged();

	/*********************************************************************
	 * Widget menu
	 *********************************************************************/
protected:
	void invokeMenu(QPoint point);
	void invokeStaticSliderMenu(QPoint point);
	void invokeDynamicSliderMenu(QPoint point);

protected slots:
	void slotMenuCallback(int item);

	/*********************************************************************
	 * Widget resize / move
	 *********************************************************************/
public:
	void resize(QPoint p);
	void move(QPoint p);

protected:
	int m_xpos;
	int m_ypos;
	bool m_resizeMode;
	QPoint m_mousePressPoint;
  
	/*********************************************************************
	 * Event handlers
	 *********************************************************************/
protected:
	void paintEvent(QPaintEvent* e);

	void mousePressEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void mouseDoubleClickEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void contextMenuEvent(QContextMenuEvent* e);

protected:
	bool m_static;
	bool m_updateOnly;

	QTime m_time;
};

#endif

