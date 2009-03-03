/*
  Q Light Controller
  virtualconsoleproperties.h

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

#ifndef VIRTUALCONSOLEPROPERTIES_H
#define VIRTUALCONSOLEPROPERTIES_H

#include <QDialog>

#include "ui_virtualconsoleproperties.h"
#include "common/qlcwidgetproperties.h"
#include "common/qlctypes.h"

class VirtualConsole;
class QDomDocument;
class QDomElement;

#define KXMLQLCVCProperties "Properties"
#define KXMLQLCVCPropertiesGrid "Grid"
#define KXMLQLCVCPropertiesGridEnabled "Enabled"
#define KXMLQLCVCPropertiesGridXResolution "XResolution"
#define KXMLQLCVCPropertiesGridYResolution "YResolution"

#define KXMLQLCVCPropertiesKeyboard "Keyboard"
#define KXMLQLCVCPropertiesKeyboardGrab "Grab"
#define KXMLQLCVCPropertiesKeyboardRepeatOff "RepeatOff"

#define KXMLQLCVCPropertiesDefaultSlider "DefaultSlider"
#define KXMLQLCVCPropertiesDefaultSliderVisible "Visible"
#define KXMLQLCVCPropertiesLowLimit "Low"
#define KXMLQLCVCPropertiesHighLimit "High"

#define KXMLQLCVCPropertiesInput "Input"
#define KXMLQLCVCPropertiesInputUniverse "Universe"
#define KXMLQLCVCPropertiesInputChannel "Channel"

/*****************************************************************************
 * Properties
 *****************************************************************************/

class VCProperties : public QLCWidgetProperties
{
	friend class VirtualConsoleProperties;
	friend class VirtualConsole;

public:
	VCProperties();
	VCProperties(const VCProperties& properties);
	~VCProperties();

	VCProperties& operator=(const VCProperties& properties);

	/** Store current VC properties */
	void store(VirtualConsole* vc);

	/*********************************************************************
	 * Grid
	 *********************************************************************/
public:
	bool isGridEnabled() const { return m_gridEnabled; }
	int gridX() const { return m_gridX; }
	int gridY() const { return m_gridY; }

protected:
	void setGridEnabled(bool enable) { m_gridEnabled = enable; }
	void setGridX(int x) { m_gridX = x; }
	void setGridY(int y) { m_gridY = y; }

protected:
	/** Widget placement grid enabled? */
	bool m_gridEnabled;

	/** Widget placement grid X resolution */
	int m_gridX;

	/** Widget placement grid Y resolution */
	int m_gridY;

	/*********************************************************************
	 * Keyboard state
	 *********************************************************************/
public:
	bool isKeyRepeatOff() const { return m_keyRepeatOff; }
	bool isGrabKeyboard() const { return m_grabKeyboard; }

protected:
	void setKeyRepeatOff(bool set) { m_keyRepeatOff = set; }
	void setGrabKeyboard(bool grab) { m_grabKeyboard = grab; }

protected:
	/** Key repeat off during operate mode? */
	bool m_keyRepeatOff;

	/** Grab keyboard in operate mode? */
	bool m_grabKeyboard;

	/*********************************************************************
	 * Default sliders
	 *********************************************************************/
public:
	bool slidersVisible() const { return m_slidersVisible; }

	t_bus_value fadeLowLimit() const { return m_fadeLowLimit; }
	t_bus_value fadeHighLimit() const { return m_fadeHighLimit; }

	t_bus_value holdLowLimit() const { return m_holdLowLimit; }
	t_bus_value holdHighLimit() const { return m_holdHighLimit; }

	t_input_universe fadeInputUniverse() const { return m_fadeInputUniverse; }
	t_input_channel fadeInputChannel() const { return m_fadeInputChannel; }

	t_input_universe holdInputUniverse() const { return m_holdInputUniverse; }
	t_input_channel holdInputChannel() const { return m_holdInputChannel; }

protected:
	void setSlidersVisible(bool visible) { m_slidersVisible = visible; }

	void setFadeLimits(t_bus_value low, t_bus_value high)
		{ m_fadeLowLimit = low; m_fadeHighLimit = high; }
	void setHoldLimits(t_bus_value low, t_bus_value high)
		{ m_holdLowLimit = low; m_holdHighLimit = high; }

	void setFadeInputSource(t_input_universe uni, t_input_channel ch)
		{ m_fadeInputUniverse = uni; m_fadeInputChannel = ch; }
	void setHoldInputSource(t_input_universe uni, t_input_channel ch)
		{ m_holdInputUniverse = uni; m_holdInputChannel = ch; }

protected:
	/** Default fade & hold sliders visible? */
	bool m_slidersVisible;

	/** Input source for fade slider */
	t_input_universe m_fadeInputUniverse;
	t_input_channel m_fadeInputChannel;

	/** Limits for fade slider */
	t_bus_value m_fadeLowLimit;
	t_bus_value m_fadeHighLimit;

	/** Input source for hold slider */
	t_input_universe m_holdInputUniverse;
	t_input_channel m_holdInputChannel;

	/** Limits for hold slider */
	t_bus_value m_holdLowLimit;
	t_bus_value m_holdHighLimit;

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	bool loadXML(QDomDocument* doc, QDomElement* root);
	bool saveXML(QDomDocument* doc, QDomElement* root);
};

/*****************************************************************************
 * Properties dialog
 *****************************************************************************/

class VirtualConsoleProperties : public QDialog,
	public Ui_VirtualConsoleProperties
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	VirtualConsoleProperties(QWidget* parent,
				 const VCProperties& properties);
	~VirtualConsoleProperties();

	VCProperties properties() const { return m_properties; }

private:
	Q_DISABLE_COPY(VirtualConsoleProperties)

protected:
	VCProperties m_properties;

	/*********************************************************************
	 * Layout page
	 *********************************************************************/
protected slots:
	void slotGrabKeyboardClicked();
	void slotKeyRepeatOffClicked();
	void slotGridClicked();
	
	void slotGridXChanged(int value);
	void slotGridYChanged(int value);

	/*********************************************************************
	 * Sliders page
	 *********************************************************************/
protected slots:
	void slotFadeLimitsChanged();
	void slotHoldLimitsChanged();

	void slotChooseFadeInputClicked();
	void slotChooseHoldInputClicked();

protected:
	void updateFadeInputSource();
	void updateHoldInputSource();
};

#endif
