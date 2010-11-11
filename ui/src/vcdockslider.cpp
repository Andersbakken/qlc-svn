/*
  Q Light Controller
  vcdockslider.cpp

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

#include <QToolButton>
#include <QSlider>
#include <QString>
#include <QLabel>

#include "qlcfile.h"

#include "virtualconsole.h"
#include "vcproperties.h"
#include "vcdockslider.h"
#include "mastertimer.h"
#include "inputmap.h"
#include "app.h"
#include "doc.h"
#include "bus.h"

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

VCDockSlider::VCDockSlider(QWidget* parent, quint32 bus) : QFrame(parent)
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    new QVBoxLayout(this);
    layout()->setMargin(0);

    m_valueLabel = new QLabel(this);
    m_valueLabel->setAlignment(Qt::AlignHCenter);
    layout()->addWidget(m_valueLabel);

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addSpacing(1);
    m_slider = new QSlider(this);
    m_slider->setStyle(App::saneStyle());
    m_slider->setInvertedAppearance(true);
    connect(m_slider, SIGNAL(valueChanged(int)),
            this, SLOT(slotSliderValueChanged(int)));
    m_slider->setPageStep(1);
    hbox->addWidget(m_slider);
    hbox->addSpacing(1);
    layout()->addItem(hbox);

    m_tapButton = new QToolButton(this);
    layout()->addWidget(m_tapButton);
    connect(m_tapButton, SIGNAL(clicked()), this, SLOT(slotTapButtonClicked()));

    m_bus = bus;

    /* Bus connections */
    connect(Bus::instance(), SIGNAL(nameChanged(quint32, const QString&)),
            this, SLOT(slotBusNameChanged(quint32, const QString&)));
    connect(Bus::instance(), SIGNAL(valueChanged(quint32, quint32)),
            this, SLOT(slotBusValueChanged(quint32, quint32)));

    /* External input connection */
    connect(_app->inputMap(), SIGNAL(inputValueChanged(quint32, quint32, uchar)),
            this, SLOT(slotInputValueChanged(quint32, quint32, uchar)));

    /* Property refresh has effect on bus value, store it now and restore below */
    quint32 busValue = Bus::instance()->value(m_bus);

    /* Read slider's properties */
    refreshProperties();

    slotBusValueChanged(m_bus, busValue);
    slotBusNameChanged(m_bus, Bus::instance()->name(m_bus));

    m_time.start();
}


VCDockSlider::~VCDockSlider()
{
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCDockSlider::refreshProperties()
{
    quint32 low = 0;
    quint32 high = 10;

    if (m_bus == Bus::defaultFade())
    {
        low = VirtualConsole::properties().fadeLowLimit();
        high = VirtualConsole::properties().fadeHighLimit();
    }
    else
    {
        low = VirtualConsole::properties().holdLowLimit();
        high = VirtualConsole::properties().holdHighLimit();
    }

    Q_ASSERT(m_slider != NULL);
    m_slider->setRange(low * MasterTimer::frequency(), high * MasterTimer::frequency());

    /* Send feedback to the bus & possible external input profile */
    slotSliderValueChanged(m_slider->value());
}

/*****************************************************************************
 * Bus
 *****************************************************************************/

void VCDockSlider::slotBusNameChanged(quint32 bus, const QString& name)
{
    if (bus == m_bus)
    {
        QString str(name);
        if (str.simplified().isEmpty() == true)
            str = tr("Bus %1").arg(m_bus + 1);
        str = str.replace(" ", "\n");
        m_tapButton->setText(str);

        setToolTip(str + " " + tr("time"));
    }
}

void VCDockSlider::slotBusValueChanged(quint32 bus, quint32 value)
{
    if (bus == m_bus && m_slider->isSliderDown() == false)
        m_slider->setValue(value);
}

/*****************************************************************************
 * Slider
 *****************************************************************************/

void VCDockSlider::slotSliderValueChanged(int value)
{
    quint32 uni;
    quint32 ch;
    QString num;

    /* Set changed value to bus */
    Bus::instance()->setValue(m_bus, m_slider->value());

    /* Set value to label */
    num.sprintf("%.2f", float(value) / float(MasterTimer::frequency()));
    m_valueLabel->setText(num);

    /* Find out this slider's input universe & channel */
    if (m_bus == Bus::defaultFade())
    {
        uni = VirtualConsole::properties().fadeInputUniverse();
        ch = VirtualConsole::properties().fadeInputChannel();
    }
    else
    {
        uni = VirtualConsole::properties().holdInputUniverse();
        ch = VirtualConsole::properties().holdInputChannel();
    }

    /* Send input feedback */
    if (uni != InputMap::invalidUniverse() && ch != KInputChannelInvalid)
    {
        if (m_slider->invertedAppearance() == true)
            value = m_slider->maximum() - value;

        float fb = SCALE(float(value), float(m_slider->minimum()),
                         float(m_slider->maximum()), float(0),
                         float(UCHAR_MAX));

        _app->inputMap()->feedBack(uni, ch, int(fb));
    }
}

/*****************************************************************************
 * Tap button
 *****************************************************************************/

void VCDockSlider::slotTapButtonClicked()
{
    int t = m_time.elapsed();
    m_slider->setValue(static_cast<int> (t * 0.001 * MasterTimer::frequency()));
    Bus::instance()->tap(m_bus);
    m_time.restart();
}

/*****************************************************************************
 * External input
 *****************************************************************************/

void VCDockSlider::slotInputValueChanged(quint32 universe, quint32 channel,
                                         uchar value)
{
    quint32 uni;
    quint32 ch;

    /* Find out this slider's input source */
    if (m_bus == Bus::defaultFade())
    {
        uni = VirtualConsole::properties().fadeInputUniverse();
        ch = VirtualConsole::properties().fadeInputChannel();
    }
    else
    {
        uni = VirtualConsole::properties().holdInputUniverse();
        ch = VirtualConsole::properties().holdInputChannel();
    }

    if (universe == uni && channel == ch)
    {
        /* Scale the from input value range to this slider's range */
        float val;
        val = SCALE((float) value, (float) 0, (float) UCHAR_MAX,
                    (float) m_slider->minimum(),
                    (float) m_slider->maximum());

        if (m_slider->invertedAppearance() == true)
            m_slider->setValue(m_slider->maximum() - (int) val);
        else
            m_slider->setValue((int) val);
    }
}
