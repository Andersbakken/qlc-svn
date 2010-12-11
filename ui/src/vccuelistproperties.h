/*
  Q Light Controller
  vccuelistproperties.h

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

#ifndef VCCUELISTPROPERTIES_H
#define VCCUELISTPROPERTIES_H

#include <QKeySequence>
#include <QDialog>

#include "ui_vccuelistproperties.h"
#include "qlctypes.h"

class VCCueList;

class VCCueListProperties : public QDialog, public Ui_VCCueListProperties
{
    Q_OBJECT
    Q_DISABLE_COPY(VCCueListProperties)

    /************************************************************************
     * Initialization
     ************************************************************************/
public:
    VCCueListProperties(QWidget* parent, VCCueList* cueList);
    ~VCCueListProperties();

public slots:
    void accept();
    void slotTabChanged();

protected:
    VCCueList* m_cueList;

    /************************************************************************
     * Cues
     ************************************************************************/
protected:
    void updateChaserName();

protected slots:
    void slotAttachClicked();
    void slotDetachClicked();

protected:
    t_function_id m_chaser;

    /************************************************************************
     * Next Cue
     ************************************************************************/
protected slots:
    void slotNextAttachClicked();
    void slotNextDetachClicked();
    void slotNextChooseInputClicked();
    void slotNextAutoDetectInputToggled(bool state);
    void slotNextInputValueChanged(quint32 uni, quint32 ch);

protected:
    void updateNextInputSource();

protected:
    QKeySequence m_nextKeySequence;
    quint32 m_nextInputUniverse;
    quint32 m_nextInputChannel;

    /************************************************************************
     * Previous Cue
     ************************************************************************/
protected slots:
    void slotPreviousAttachClicked();
    void slotPreviousDetachClicked();
    void slotPreviousChooseInputClicked();
    void slotPreviousAutoDetectInputToggled(bool state);
    void slotPreviousInputValueChanged(quint32 uni, quint32 ch);

protected:
    void updatePreviousInputSource();

protected:
    QKeySequence m_previousKeySequence;
    quint32 m_previousInputUniverse;
    quint32 m_previousInputChannel;
};

#endif
