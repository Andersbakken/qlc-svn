/*
  Q Light Controller
  configuremidiline.cpp

  Copyright (C) Heikki Junnila

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

#include <QComboBox>
#include <QDialog>

#include "configuremidiline.h"
#include "mididevice.h"

ConfigureMIDILine::ConfigureMIDILine(QWidget* parent, MIDIDevice* device)
        : QDialog(parent)
{
    Q_ASSERT(device != NULL);
    m_device = device;

    setupUi(this);

#ifdef WIN32
    m_feedBackCombo->addItems(MIDIDevice::feedBackNames());
    m_feedBackCombo->setCurrentIndex(m_device->feedBackId());
#else
    m_feedBackLabel->hide();
    m_feedBackCombo->hide();
#endif
    m_midiChannelCombo->setCurrentIndex(device->midiChannel());
}

ConfigureMIDILine::~ConfigureMIDILine()
{
}

void ConfigureMIDILine::accept()
{
#ifdef WIN32
    m_device->setFeedBackId(m_feedBackCombo->currentIndex());
#endif
    m_device->setMidiChannel(m_midiChannelCombo->currentIndex());

    QDialog::accept();
}
