/*
  Q Light Controller
  grandmasterslider.cpp

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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSlider>
#include <QLabel>

#include "grandmasterslider.h"
#include "virtualconsole.h"
#include "universearray.h"
#include "vcproperties.h"
#include "outputmap.h"
#include "inputmap.h"
#include "app.h"

extern App* _app;

GrandMasterSlider::GrandMasterSlider(QWidget* parent) : QFrame(parent)
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    new QVBoxLayout(this);
    layout()->setMargin(0);

    m_valueLabel = new QLabel(this);
    m_valueLabel->setAlignment(Qt::AlignHCenter);
    layout()->addWidget(m_valueLabel);

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->setMargin(0);
    hbox->addSpacing(1);
    m_slider = new QSlider(this);
    hbox->addWidget(m_slider);
    m_slider->setRange(0, UCHAR_MAX);
    m_slider->setStyle(App::saneStyle());
    connect(m_slider, SIGNAL(valueChanged(int)),
            this, SLOT(slotValueChanged(int)));
    hbox->addSpacing(1);
    layout()->addItem(hbox);

    m_nameLabel = new QLabel(this);
    m_nameLabel->setWordWrap(true);
    m_nameLabel->setAlignment(Qt::AlignHCenter);
    m_nameLabel->setText(tr("Grand<BR>Master"));
    layout()->addWidget(m_nameLabel);

    // Get the current grand master value
    m_slider->setValue(_app->outputMap()->peekUniverses()->gMValue());

    /* External input connection */
    connect(_app->inputMap(), SIGNAL(inputValueChanged(quint32, quint32, uchar)),
            this, SLOT(slotInputValueChanged(quint32, quint32, uchar)));

    refreshProperties();
}

GrandMasterSlider::~GrandMasterSlider()
{
}

void GrandMasterSlider::refreshProperties()
{
    QString tooltip;

    switch (VirtualConsole::properties().grandMasterValueMode())
    {
        case UniverseArray::GMLimit:
            tooltip += tr("Limits the maximum value of");
            break;
        case UniverseArray::GMReduce:
            tooltip += tr("Reduces the current value of");
            break;
    }

    tooltip += QString(" ");

    switch (VirtualConsole::properties().grandMasterChannelMode())
    {
        case UniverseArray::GMIntensity:
            tooltip += tr("intensity channels");
            break;
        case UniverseArray::GMAllChannels:
            tooltip += tr("all channels");
            break;
    }

    setToolTip(tooltip);

    /* Set properties to UniverseArray */
    UniverseArray* uni = _app->outputMap()->claimUniverses();
    uni->setGMChannelMode(VirtualConsole::properties().grandMasterChannelMode());
    uni->setGMValueMode(VirtualConsole::properties().grandMasterValueMode());
    _app->outputMap()->releaseUniverses();
}

void GrandMasterSlider::slotValueChanged(int value)
{
    // Write new grand master value to universes
    UniverseArray* uni = _app->outputMap()->claimUniverses();
    uni->setGMValue(value);
    _app->outputMap()->releaseUniverses();

    // Display value
    QString str;
    if (VirtualConsole::properties().grandMasterValueMode() == UniverseArray::GMLimit)
    {
        str = QString("%1").arg(value, 3, 10, QChar('0'));
    }
    else
    {
        int p = floor(((double(value) / double(UCHAR_MAX)) * double(100)) + 0.5);
        str = QString("%1%").arg(p, 3, 10, QChar('0'));
    }

    m_valueLabel->setText(str);
}

/*****************************************************************************
 * External input
 *****************************************************************************/

void GrandMasterSlider::slotInputValueChanged(quint32 universe, quint32 channel,
                                              uchar value)
{
    if (universe == VirtualConsole::properties().grandMasterInputUniverse() &&
        channel == VirtualConsole::properties().grandMasterInputChannel())
    {
        m_slider->setValue(value);
    }
}

