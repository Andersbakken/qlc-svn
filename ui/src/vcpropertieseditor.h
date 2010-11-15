/*
  Q Light Controller
  vcpropertieseditor.h

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

#ifndef VCPROPERTIESEDITOR_H
#define VCPROPERTIESEDITOR_H

#include <QDialog>

#include "ui_vcproperties.h"

#include "vcwidgetproperties.h"
#include "universearray.h"
#include "vcproperties.h"
#include "qlctypes.h"

class VirtualConsole;
class QDomDocument;
class QDomElement;
class VCFrame;

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

    VCProperties properties() const;

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

    /*************************************************************************
     * Fade slider page
     *************************************************************************/
protected slots:
    void slotFadeLimitsChanged();
    void slotAutoDetectFadeInputToggled(bool checked);
    void slotFadeInputValueChanged(quint32 universe, quint32 channel);
    void slotChooseFadeInputClicked();

protected:
    void updateFadeInputSource();

    /*************************************************************************
     * Hold slider page
     *************************************************************************/
protected slots:
    void slotHoldLimitsChanged();
    void slotAutoDetectHoldInputToggled(bool checked);
    void slotHoldInputValueChanged(quint32 universe, quint32 channel);
    void slotChooseHoldInputClicked();

protected:
    void updateHoldInputSource();

    /*************************************************************************
     * Grand Master page
     *************************************************************************/
protected slots:
    void slotGrandMasterIntensityToggled(bool checked);
    void slotGrandMasterReduceToggled(bool checked);
    void slotAutoDetectGrandMasterInputToggled(bool checked);
    void slotGrandMasterInputValueChanged(quint32 universe, quint32 channel);
    void slotChooseGrandMasterInputClicked();

protected:
    void updateGrandMasterInputSource();

    /*************************************************************************
     * Blackout page
     *************************************************************************/
protected slots:
    void slotAutoDetectBlackoutInputToggled(bool checked);
    void slotBlackoutInputValueChanged(quint32 universe, quint32 channel);
    void slotChooseBlackoutInputClicked();

protected:
    void updateBlackoutInputSource();

    /*************************************************************************
     * Input Source helper
     *************************************************************************/
protected:
    bool inputSourceNames(quint32 universe, quint32 channel,
                          QString& uniName, QString& chName) const;
};

#endif
