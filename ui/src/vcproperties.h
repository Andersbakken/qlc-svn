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

#include "vcwidgetproperties.h"
#include "ui_vcproperties.h"
#include "universearray.h"
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
#define KXMLQLCVCPropertiesDefaultSliderRole "Role"
#define KXMLQLCVCPropertiesDefaultSliderRoleFade "Fade"
#define KXMLQLCVCPropertiesDefaultSliderRoleHold "Hold"
#define KXMLQLCVCPropertiesDefaultSliderVisible "Visible"
#define KXMLQLCVCPropertiesLowLimit "Low"
#define KXMLQLCVCPropertiesHighLimit "High"

#define KXMLQLCVCPropertiesGrandMaster "GrandMaster"
#define KXMLQLCVCPropertiesGrandMasterChannelMode "ChannelMode"
#define KXMLQLCVCPropertiesGrandMasterValueMode "ValueMode"

#define KXMLQLCVCPropertiesBlackout "Blackout"

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

    /*************************************************************************
     * Grand Master
     *************************************************************************/
public:
    UniverseArray::GMChannelMode grandMasterChannelMode() const {
        return m_gmChannelMode;
    }

    void setGrandMasterChannelMode(UniverseArray::GMChannelMode mode) {
        m_gmChannelMode = mode;
    }

    UniverseArray::GMValueMode grandMasterValueMode() const {
        return m_gmValueMode;
    }

    void setGrandMasterValueMode(UniverseArray::GMValueMode mode) {
        m_gmValueMode = mode;
    }

    quint32 grandMasterInputUniverse() const {
        return m_gmInputUniverse;
    }

    quint32 grandMasterInputChannel() const {
        return m_gmInputChannel;
    }

    void setGrandMasterInputSource(quint32 universe, quint32 channel) {
        m_gmInputUniverse = universe;
        m_gmInputChannel = channel;
    }

protected:
    UniverseArray::GMChannelMode m_gmChannelMode;
    UniverseArray::GMValueMode m_gmValueMode;
    quint32 m_gmInputUniverse;
    quint32 m_gmInputChannel;

    /*************************************************************************
     * Blackout
     *************************************************************************/
public:
    quint32 blackoutInputUniverse() const {
        return m_blackoutInputUniverse;
    }

    quint32 blackoutInputChannel() const {
        return m_blackoutInputChannel;
    }

    void setBlackoutInputSource(quint32 universe, quint32 channel) {
        m_blackoutInputUniverse = universe;
        m_blackoutInputChannel = channel;
    }

protected:
    quint32 m_blackoutInputUniverse;
    quint32 m_blackoutInputChannel;

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

    /*************************************************************************
     * Load & Save
     *************************************************************************/
public:
    bool loadXML(const QDomElement* vc_root);
    bool saveXML(QDomDocument* doc, QDomElement* wksp_root);

protected:
    bool loadXMLInput(const QDomElement& tag, quint32* universe, quint32* channel);
    bool loadProperties(const QDomElement* root);
};

#endif
