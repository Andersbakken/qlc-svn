/*
  Q Light Controller
  vcproperties.cpp

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

#include <QtXml>

#include "qlcfile.h"

#include "virtualconsole.h"
#include "vcproperties.h"
#include "inputpatch.h"
#include "inputmap.h"
#include "vcframe.h"
#include "app.h"
#include "bus.h"

extern App* _app;

/*****************************************************************************
 * Properties Initialization
 *****************************************************************************/

VCProperties::VCProperties() : VCWidgetProperties()
{
    m_contents = NULL;

    m_width = 640;
    m_height = 480;

    m_gridEnabled = true;
    m_gridX = 10;
    m_gridY = 10;

    m_keyRepeatOff = true;
    m_grabKeyboard = true;

    m_gmChannelMode = UniverseArray::GMIntensity;
    m_gmValueMode = UniverseArray::GMReduce;
    m_gmInputUniverse = InputMap::invalidUniverse();
    m_gmInputChannel = KInputChannelInvalid;

    m_blackoutInputUniverse = InputMap::invalidUniverse();
    m_blackoutInputChannel = KInputChannelInvalid;

    m_slidersVisible = true;

    m_fadeLowLimit = 0;
    m_fadeHighLimit = 10;
    m_holdLowLimit = 0;
    m_holdHighLimit = 10;

    m_fadeInputUniverse = InputMap::invalidUniverse();
    m_fadeInputChannel = KInputChannelInvalid;
    m_holdInputUniverse = InputMap::invalidUniverse();
    m_holdInputChannel = KInputChannelInvalid;
}

VCProperties::VCProperties(const VCProperties& properties)
        : VCWidgetProperties(properties)
{
    *this = properties;
}

VCProperties::~VCProperties()
{
    /* Don't delete m_contents because there might be several copies of
       this class in memory and deleting just one of them would delete
       the contents for others as well. */
}

VCProperties& VCProperties::operator=(const VCProperties& properties)
{
    /* The contents of m_contents cannot be copied. Instead, only the
       pointer is copied. */
    m_contents = properties.m_contents;

    m_gridEnabled = properties.m_gridEnabled;
    m_gridX = properties.m_gridX;
    m_gridY = properties.m_gridY;

    m_keyRepeatOff = properties.m_keyRepeatOff;
    m_grabKeyboard = properties.m_grabKeyboard;

    m_gmChannelMode = properties.m_gmChannelMode;
    m_gmValueMode = properties.m_gmValueMode;
    m_gmInputUniverse = properties.m_gmInputUniverse;
    m_gmInputChannel = properties.m_gmInputChannel;

    m_blackoutInputUniverse = properties.m_blackoutInputUniverse;
    m_blackoutInputChannel = properties.m_blackoutInputChannel;

    m_slidersVisible = properties.m_slidersVisible;

    m_fadeLowLimit = properties.m_fadeLowLimit;
    m_fadeHighLimit = properties.m_fadeHighLimit;
    m_holdLowLimit = properties.m_holdLowLimit;
    m_holdHighLimit = properties.m_holdHighLimit;

    m_fadeInputUniverse = properties.m_fadeInputUniverse;
    m_fadeInputChannel = properties.m_fadeInputChannel;
    m_holdInputUniverse = properties.m_holdInputUniverse;
    m_holdInputChannel = properties.m_holdInputChannel;

    return *this;
}

/*****************************************************************************
 * VC Contents
 *****************************************************************************/

void VCProperties::resetContents()
{
    /* Get rid of any existing contents */
    if (m_contents != NULL)
        delete m_contents;

    /* Create new contents */
    m_contents = new VCFrame(_app);
}

/*****************************************************************************
 * Properties Load & Save
 *****************************************************************************/

bool VCProperties::loadXML(const QDomElement* vc_root)
{
    QDomNode node;
    QDomElement tag;
    QString str;

    Q_ASSERT(vc_root != NULL);

    if (vc_root->tagName() != KXMLQLCVirtualConsole)
    {
        qWarning() << Q_FUNC_INFO << "Virtual Console node not found";
        return false;
    }

    node = vc_root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();

        if (tag.tagName() == KXMLQLCVCProperties)
        {
            /* Properties */
            loadProperties(&tag);
        }
        else if (tag.tagName() == KXMLQLCVCFrame)
        {
            /* Contents */
            if (m_contents == NULL)
                m_contents = new VCFrame(_app);
            Q_ASSERT(m_contents != NULL);
            m_contents->loadXML(&tag);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Virtual Console tag"
                       << tag.tagName();
        }

        /* Next node */
        node = node.nextSibling();
    }

    return true;
}

bool VCProperties::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement prop_root;
    QDomElement vc_root;
    QDomElement subtag;
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Virtual Console entry */
    vc_root = doc->createElement(KXMLQLCVirtualConsole);
    wksp_root->appendChild(vc_root);

    /* Contents */
    if (m_contents != NULL)
        m_contents->saveXML(doc, &vc_root);

    /* Properties entry */
    prop_root = doc->createElement(KXMLQLCVCProperties);
    vc_root.appendChild(prop_root);

    /* Grid */
    tag = doc->createElement(KXMLQLCVCPropertiesGrid);
    prop_root.appendChild(tag);
    if (m_gridEnabled == true)
        tag.setAttribute(KXMLQLCVCPropertiesGridEnabled, KXMLQLCTrue);
    else
        tag.setAttribute(KXMLQLCVCPropertiesGridEnabled, KXMLQLCFalse);

    tag.setAttribute(KXMLQLCVCPropertiesGridXResolution,
                     QString("%1").arg(m_gridX));
    tag.setAttribute(KXMLQLCVCPropertiesGridYResolution,
                     QString("%1").arg(m_gridY));

    /* Keyboard settings */
    tag = doc->createElement(KXMLQLCVCPropertiesKeyboard);
    prop_root.appendChild(tag);

    /* Grab keyboard */
    if (m_grabKeyboard == true)
        tag.setAttribute(KXMLQLCVCPropertiesKeyboardGrab, KXMLQLCTrue);
    else
        tag.setAttribute(KXMLQLCVCPropertiesKeyboardGrab, KXMLQLCFalse);

    /* Key repeat off */
    if (m_keyRepeatOff == true)
        tag.setAttribute(KXMLQLCVCPropertiesKeyboardRepeatOff, KXMLQLCTrue);
    else
        tag.setAttribute(KXMLQLCVCPropertiesKeyboardRepeatOff, KXMLQLCFalse);

    /***********************
     * Default fade slider *
     ***********************/
    tag = doc->createElement(KXMLQLCVCPropertiesDefaultSlider);
    prop_root.appendChild(tag);
    tag.setAttribute(KXMLQLCVCPropertiesDefaultSliderRole,
                     KXMLQLCVCPropertiesDefaultSliderRoleFade);

    /* Slider visibility */
    if (m_slidersVisible == true)
        tag.setAttribute(KXMLQLCVCPropertiesDefaultSliderVisible, KXMLQLCTrue);
    else
        tag.setAttribute(KXMLQLCVCPropertiesDefaultSliderVisible, KXMLQLCFalse);

    /* Fade slider limits */
    tag.setAttribute(KXMLQLCVCPropertiesLowLimit,
                     QString("%1").arg(m_fadeLowLimit));
    tag.setAttribute(KXMLQLCVCPropertiesHighLimit,
                     QString("%1").arg(m_fadeHighLimit));

    /* Fade slider external input */
    if (m_fadeInputUniverse != InputMap::invalidUniverse() &&
        m_fadeInputChannel != KInputChannelInvalid)
    {
        subtag = doc->createElement(KXMLQLCVCPropertiesInput);
        tag.appendChild(subtag);
        subtag.setAttribute(KXMLQLCVCPropertiesInputUniverse,
                            QString("%1").arg(m_fadeInputUniverse));
        subtag.setAttribute(KXMLQLCVCPropertiesInputChannel,
                            QString("%1").arg(m_fadeInputChannel));
    }

    /***********************
     * Default hold slider *
     ***********************/
    tag = doc->createElement(KXMLQLCVCPropertiesDefaultSlider);
    prop_root.appendChild(tag);
    tag.setAttribute(KXMLQLCVCPropertiesDefaultSliderRole,
                     KXMLQLCVCPropertiesDefaultSliderRoleHold);

    /* Hold slider limits */
    tag.setAttribute(KXMLQLCVCPropertiesLowLimit,
                     QString("%1").arg(m_holdLowLimit));
    tag.setAttribute(KXMLQLCVCPropertiesHighLimit,
                     QString("%1").arg(m_holdHighLimit));

    /* Hold slider external input */
    if (m_holdInputUniverse != InputMap::invalidUniverse() &&
        m_holdInputChannel != KInputChannelInvalid)
    {
        subtag = doc->createElement(KXMLQLCVCPropertiesInput);
        tag.appendChild(subtag);
        subtag.setAttribute(KXMLQLCVCPropertiesInputUniverse,
                            QString("%1").arg(m_holdInputUniverse));
        subtag.setAttribute(KXMLQLCVCPropertiesInputChannel,
                            QString("%1").arg(m_holdInputChannel));
    }

    /***********************
     * Grand Master slider *
     ***********************/
    tag = doc->createElement(KXMLQLCVCPropertiesGrandMaster);
    prop_root.appendChild(tag);

    /* Channel mode */
    tag.setAttribute(KXMLQLCVCPropertiesGrandMasterChannelMode,
                     UniverseArray::gMChannelModeToString(m_gmChannelMode));

    /* Value mode */
    tag.setAttribute(KXMLQLCVCPropertiesGrandMasterValueMode,
                     UniverseArray::gMValueModeToString(m_gmValueMode));

    /* Grand Master external input */
    if (m_gmInputUniverse != InputMap::invalidUniverse() &&
        m_gmInputChannel != KInputChannelInvalid)
    {
        subtag = doc->createElement(KXMLQLCVCPropertiesInput);
        tag.appendChild(subtag);
        subtag.setAttribute(KXMLQLCVCPropertiesInputUniverse,
                            QString("%1").arg(m_gmInputUniverse));
        subtag.setAttribute(KXMLQLCVCPropertiesInputChannel,
                            QString("%1").arg(m_gmInputChannel));
    }

    /************
     * Blackout *
     ************/
    tag = doc->createElement(KXMLQLCVCPropertiesBlackout);
    prop_root.appendChild(tag);

    /* Grand Master external input */
    if (m_blackoutInputUniverse != InputMap::invalidUniverse() &&
        m_blackoutInputChannel != KInputChannelInvalid)
    {
        subtag = doc->createElement(KXMLQLCVCPropertiesInput);
        tag.appendChild(subtag);
        subtag.setAttribute(KXMLQLCVCPropertiesInputUniverse,
                            QString("%1").arg(m_blackoutInputUniverse));
        subtag.setAttribute(KXMLQLCVCPropertiesInputChannel,
                            QString("%1").arg(m_blackoutInputChannel));
    }

    /* Save widget properties */
    return VCWidgetProperties::saveXML(doc, &prop_root);
}

bool VCProperties::loadProperties(const QDomElement* root)
{
    QDomElement tag;
    QDomNode node;
    QString str;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCProperties)
    {
        qWarning() << Q_FUNC_INFO << "Virtual console properties node not found";
        return false;
    }

    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();
        if (tag.tagName() == KXMLQLCVCPropertiesGrid)
        {
            /* Grid X resolution */
            str = tag.attribute(KXMLQLCVCPropertiesGridXResolution);
            setGridX(str.toInt());

            /* Grid Y resolution */
            str = tag.attribute(KXMLQLCVCPropertiesGridYResolution);
            setGridY(str.toInt());

            /* Grid enabled */
            str = tag.attribute(KXMLQLCVCPropertiesGridEnabled);
            if (str == KXMLQLCTrue)
                setGridEnabled(true);
            else
                setGridEnabled(false);
        }
        else if (tag.tagName() == KXMLQLCVCPropertiesKeyboard)
        {
            /* Keyboard grab */
            str = tag.attribute(KXMLQLCVCPropertiesKeyboardGrab);
            if (str == KXMLQLCTrue)
                setGrabKeyboard(true);
            else
                setGrabKeyboard(false);

            /* Key repeat */
            str = tag.attribute(KXMLQLCVCPropertiesKeyboardRepeatOff);
            if (str == KXMLQLCTrue)
                setKeyRepeatOff(true);
            else
                setKeyRepeatOff(false);
        }
        else if (tag.tagName() == KXMLQLCVCPropertiesDefaultSlider)
        {
            quint32 low = 0;
            quint32 high = 10;
            quint32 universe = InputMap::invalidUniverse();
            quint32 channel = KInputChannelInvalid;
            QDomElement subtag;

            /* Bus low limit */
            str = tag.attribute(KXMLQLCVCPropertiesLowLimit);
            if (str.isNull() == false)
                low = quint32(str.toUInt());

            /* Bus high limit */
            str = tag.attribute(KXMLQLCVCPropertiesHighLimit);
            if (str.isNull() == false)
                high = quint32(str.toUInt());

            /* Sliders' visibility (on by default) */
            str = tag.attribute(KXMLQLCVCPropertiesDefaultSliderVisible);
            if (str == KXMLQLCFalse)
                setSlidersVisible(false);
            else
                setSlidersVisible(true);

            /* External input */
            bool inputOK = loadXMLInput(tag.firstChild().toElement(),
                                        &universe, &channel);

            /* Set the gathered properties to the correct slider */
            if (tag.attribute(KXMLQLCBusRole) == KXMLQLCBusFade)
            {
                setFadeLimits(low, high);
                if (inputOK == true)
                    setFadeInputSource(universe, channel);
            }
            else
            {
                setHoldLimits(low, high);
                if (inputOK == true)
                    setHoldInputSource(universe, channel);
            }
        }
        else if (tag.tagName() == KXMLQLCVCPropertiesGrandMaster)
        {
            quint32 universe = InputMap::invalidUniverse();
            quint32 channel = KInputChannelInvalid;

            str = tag.attribute(KXMLQLCVCPropertiesGrandMasterChannelMode);
            m_gmChannelMode = UniverseArray::stringToGMChannelMode(str);

            str = tag.attribute(KXMLQLCVCPropertiesGrandMasterValueMode);
            m_gmValueMode = UniverseArray::stringToGMValueMode(str);

            /* External input */
            bool inputOK = loadXMLInput(tag.firstChild().toElement(),
                                        &universe, &channel);
            if (inputOK == true)
                setGrandMasterInputSource(universe, channel);
        }
        else if (tag.tagName() == KXMLQLCVCPropertiesBlackout)
        {
            quint32 universe = InputMap::invalidUniverse();
            quint32 channel = KInputChannelInvalid;
            /* External input */
            bool inputOK = loadXMLInput(tag.firstChild().toElement(),
                                        &universe, &channel);
            if (inputOK == true)
                setBlackoutInputSource(universe, channel);
        }
        else if (tag.tagName() == KXMLQLCWidgetProperties)
        {
            VCWidgetProperties::loadXML(&tag);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown virtual console property tag:" << tag.tagName();
        }

        /* Next node */
        node = node.nextSibling();
    }

    return true;
}

bool VCProperties::loadXMLInput(const QDomElement& tag, quint32* universe,
                                quint32* channel)
{
    /* External input */
    if (tag.isNull() == false && tag.tagName() == KXMLQLCVCPropertiesInput)
    {
        QString str;

        /* Universe */
        str = tag.attribute(KXMLQLCVCPropertiesInputUniverse);
        if (str.isNull() == false)
            *universe = str.toUInt();
        else
            *universe = InputMap::invalidUniverse();

        /* Channel */
        str = tag.attribute(KXMLQLCVCPropertiesInputChannel);
        if (str.isNull() == false)
            *channel = str.toUInt();
        else
            *channel = KInputChannelInvalid;

        if (*universe != InputMap::invalidUniverse() && *channel != KInputChannelInvalid)
            return true;
        else
            return false;
    }
    else
    {
        return false;
    }
}
