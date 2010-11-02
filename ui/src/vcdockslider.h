/*
  Q Light Controller
  vcdockslider.h

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

#ifndef VCDOCKSLIDER_H
#define VCDOCKSLIDER_H

#include <QFrame>
#include <QTime>

#include "qlctypes.h"

class QToolButton;
class QSlider;
class QLabel;

class VCDockSlider : public QFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(VCDockSlider)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    VCDockSlider(QWidget* parent, quint32 bus);
    ~VCDockSlider();

    /** Refresh properties from VCProperties */
    void refreshProperties();

    /*********************************************************************
     * Bus
     *********************************************************************/
protected slots:
    /** Catches bus name changes */
    void slotBusNameChanged(quint32 bus, const QString& name);

    /** Catches bus value changes */
    void slotBusValueChanged(quint32 bus, quint32 value);

protected:
    quint32 m_bus;

    /*************************************************************************
     * UI Controls
     *************************************************************************/
protected slots:
    /** Catches slider value changes */
    void slotSliderValueChanged(int value);

    /** Catches tap button clicks */
    void slotTapButtonClicked();

protected:
    QLabel* m_valueLabel;
    QSlider* m_slider;
    QToolButton* m_tapButton;

    QTime m_time;

    /*************************************************************************
     * External input
     *************************************************************************/
protected slots:
    /** Slot for external input value change signals */
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);
};

#endif

