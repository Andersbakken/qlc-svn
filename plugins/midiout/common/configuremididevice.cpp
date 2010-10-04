/*
  Q Light Controller
  configuremididevice.cpp

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
#include <QSpinBox>
#include <QDialog>

#include "configuremididevice.h"
#include "mididevice.h"

ConfigureMIDIDevice::ConfigureMIDIDevice(QWidget* parent, MIDIDevice* device)
        : QDialog(parent)
{
    Q_ASSERT(parent != NULL);
    Q_ASSERT(device != NULL);
    m_device = device;

    setupUi(this);

    m_modeCombo->addItem(MIDIDevice::modeToString(MIDIDevice::ControlChange));
    m_modeCombo->addItem(MIDIDevice::modeToString(MIDIDevice::Note));

    m_midiChannelSpin->setValue(device->midiChannel() + 1);
    m_modeCombo->setCurrentIndex(device->mode());
}

ConfigureMIDIDevice::~ConfigureMIDIDevice()
{
}

void ConfigureMIDIDevice::accept()
{
    m_device->setMidiChannel(m_midiChannelSpin->value() - 1);
    m_device->setMode(MIDIDevice::stringToMode(m_modeCombo->currentText()));

    QDialog::accept();
}
