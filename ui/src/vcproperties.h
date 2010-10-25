/*
  Q Light Controller
  vcproperties.h

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

#ifndef VCPROPERTIES_H
#define VCPROPERTIES_H

#include <QDialog>

#include "ui_vcproperties.h"
#include "vcwidgetproperties.h"
#include "qlctypes.h"

class VirtualConsole;
class QDomDocument;
class QDomElement;
class VCFrame;

#define KXMLQLCVirtualConsole "VirtualConsole"

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

class VCProperties : public VCWidgetProperties
{
    friend class VCPropertiesEditor;
    friend class VirtualConsole;

public:
    VCProperties();
    VCProperties(const VCProperties& properties);
    ~VCProperties();

    VCProperties& operator=(const VCProperties& properties);

    /*********************************************************************
     * VC Contents
     *********************************************************************/
public:
    /** Get Virtual Console's bottom-most frame */
    VCFrame* contents() const {
        return m_contents;
    }

    /** Reset Virtual Console's bottom-most frame to initial state */
    void resetContents();

protected:
    /** The bottom-most frame in Virtual Console containing all widgets */
    VCFrame* m_contents;

    /*********************************************************************
     * Grid
     *********************************************************************/
public:
    bool isGridEnabled() const {
        return m_gridEnabled;
    }
    int gridX() const {
        return m_gridX;
    }
    int gridY() const {
        return m_gridY;
    }

protected:
    void setGridEnabled(bool enable) {
        m_gridEnabled = enable;
    }
    void setGridX(int x) {
        m_gridX = x;
    }
    void setGridY(int y) {
        m_gridY = y;
    }

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
    bool isKeyRepeatOff() const {
        return m_keyRepeatOff;
    }
    bool isGrabKeyboard() const {
        return m_grabKeyboard;
    }

protected:
    void setKeyRepeatOff(bool set) {
        m_keyRepeatOff = set;
    }
    void setGrabKeyboard(bool grab) {
        m_grabKeyboard = grab;
    }

protected:
    /** Key repeat off during operate mode? */
    bool m_keyRepeatOff;

    /** Grab keyboard in operate mode? */
    bool m_grabKeyboard;

    /*********************************************************************
     * Default sliders
     *********************************************************************/
public:
    bool slidersVisible() const {
        return m_slidersVisible;
    }

    quint32 fadeLowLimit() const {
        return m_fadeLowLimit;
    }
    quint32 fadeHighLimit() const {
        return m_fadeHighLimit;
    }

    quint32 holdLowLimit() const {
        return m_holdLowLimit;
    }
    quint32 holdHighLimit() const {
        return m_holdHighLimit;
    }

    quint32 fadeInputUniverse() const {
        return m_fadeInputUniverse;
    }
    quint32 fadeInputChannel() const {
        return m_fadeInputChannel;
    }

    quint32 holdInputUniverse() const {
        return m_holdInputUniverse;
    }
    quint32 holdInputChannel() const {
        return m_holdInputChannel;
    }

protected:
    void setSlidersVisible(bool visible) {
        m_slidersVisible = visible;
    }

    void setFadeLimits(quint32 low, quint32 high)
    {
        m_fadeLowLimit = low;
        m_fadeHighLimit = high;
    }
    void setHoldLimits(quint32 low, quint32 high)
    {
        m_holdLowLimit = low;
        m_holdHighLimit = high;
    }

    void setFadeInputSource(quint32 uni, quint32 ch)
    {
        m_fadeInputUniverse = uni;
        m_fadeInputChannel = ch;
    }
    void setHoldInputSource(quint32 uni, quint32 ch)
    {
        m_holdInputUniverse = uni;
        m_holdInputChannel = ch;
    }

protected:
    /** Default fade & hold sliders visible? */
    bool m_slidersVisible;

    /** Input source for fade slider */
    quint32 m_fadeInputUniverse;
    quint32 m_fadeInputChannel;

    /** Limits for fade slider */
    quint32 m_fadeLowLimit;
    quint32 m_fadeHighLimit;

    /** Input source for hold slider */
    quint32 m_holdInputUniverse;
    quint32 m_holdInputChannel;

    /** Limits for hold slider */
    quint32 m_holdLowLimit;
    quint32 m_holdHighLimit;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    bool loadXML(const QDomElement* vc_root);
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

protected:
    bool loadProperties(const QDomElement* root);
};

/*****************************************************************************
 * Properties dialog
 *****************************************************************************/

class VCPropertiesEditor : public QDialog, public Ui_VCPropertiesEditor
{
    Q_OBJECT
    Q_DISABLE_COPY(VCPropertiesEditor)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCPropertiesEditor(QWidget* parent, const VCProperties& properties);
    ~VCPropertiesEditor();

    VCProperties properties() const {
        return m_properties;
    }

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

    void slotAutoDetectFadeInputToggled(bool checked);
    void slotAutoDetectHoldInputToggled(bool checked);
    void slotFadeInputValueChanged(quint32 universe, quint32 channel);
    void slotHoldInputValueChanged(quint32 universe, quint32 channel);

    void slotChooseFadeInputClicked();
    void slotChooseHoldInputClicked();

protected:
    void doAutoDetectConnections(bool checked);
    void updateFadeInputSource();
    void updateHoldInputSource();
};

#endif
