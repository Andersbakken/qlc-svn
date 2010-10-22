/*
  Q Light Controller
  mididevice.cpp

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

#include <QApplication>
#include <Windows.h>
#include <QSettings>
#include <QVariant>
#include <QObject>
#include <QString>
#include <QDebug>

#include "mididevice.h"
#include "midiinput.h"
#include "midiinputevent.h"
#include "midiprotocol.h"

MIDIDevice::MIDIDevice(MIDIInput* parent, UINT id) : QObject(parent)
{
    m_id = id;
    m_feedBackId = UINT_MAX;
    m_handle = NULL;
    m_feedBackHandle = NULL;
    m_isOK = false;
    m_midiChannel = 0;

    extractName();
    loadSettings();
}

MIDIDevice::~MIDIDevice()
{
    saveSettings();
    close();
}

void MIDIDevice::loadSettings()
{
    QSettings settings;
    QVariant value;
    QString key;

    /* Attempt to load feedback line number */
    key = QString("/midiinput/%1/feedbackid").arg(m_id);
    value = settings.value(key);
    if (value.isValid() == true && value.toUInt() != UINT_MAX)
        setFeedBackId(value.toUInt());


    /* Attempt to get a MIDI channel from settings */
    key = QString("/midiinput/%1/midichannel").arg(m_id);
    value = settings.value(key);
    if (value.isValid() == true)
        setMidiChannel(value.toInt());
    else
        setMidiChannel(0);
}

void MIDIDevice::saveSettings()
{
    QSettings settings;
    QString key;

    /* Store Feedback line number */
    key = QString("/midiinput/%1/feedbackid").arg(m_id);
    settings.setValue(key, m_feedBackId);

    /* Store MIDI channel to settings */
    key = QString("/midiinput/%1/midichannel").arg(m_id);
    settings.setValue(key, m_midiChannel);
}

/*****************************************************************************
 * File operations
 *****************************************************************************/

static void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg,
                                DWORD_PTR dwInstance, DWORD_PTR dwParam1,
                                DWORD_PTR dwParam2)
{
    MIDIDevice* self = (MIDIDevice*) dwInstance;
    Q_ASSERT(self != NULL);

    if (wMsg == MIM_DATA)
    {
        BYTE cmd = dwParam1 & 0xFF;
        BYTE data1 = (dwParam1 & 0xFF00) >> 8;
        BYTE data2 = (dwParam1 & 0xFF0000) >> 16;
        quint32 channel = 0;
        uchar value = 0;

        if (QLCMIDIProtocol::midiToInput(cmd, data1, data2,
            uchar(self->midiChannel()), &channel, &value) == true)
        {
            MIDIInputEvent* event = new MIDIInputEvent(self, channel, value);
            QApplication::postEvent(self, event);
        }
    }
}

bool MIDIDevice::open()
{
    bool result = false;
    MMRESULT res;

    res = midiInOpen(&m_handle, m_id, (DWORD) MidiInProc,
                     (DWORD_PTR) this, CALLBACK_FUNCTION);
    if (res == MMSYSERR_ALLOCATED)
    {
        qDebug() << QString("Unable to open %1:").arg(m_name)
                 << QString("Resource is already allocated.");
        m_handle = NULL;
        result = false;
    }
    else if (res == MMSYSERR_BADDEVICEID)
    {
        qDebug() << QString("Unable to open %1:").arg(m_name)
                 << QString("Bad device ID.");
        m_handle = NULL;
        result = false;
    }
    else if (res == MMSYSERR_INVALFLAG)
    {
        qDebug() << QString("Unable to open %1:").arg(m_name)
                 << QString("Invalid flags.");
        m_handle = NULL;
        result = false;
    }
    else if (res == MMSYSERR_INVALPARAM)
    {
        qDebug() << QString("Unable to open %1:").arg(m_name)
                 << QString("Invalid parameters.");
        m_handle = NULL;
        result = false;
    }
    else if (res == MMSYSERR_NOMEM)
    {
        qDebug() << QString("Unable to open %1:").arg(m_name)
                 << QString("Out of memory.");
        result = false;
    }
    else
    {
        qDebug() << QString("%1 opened.").arg(m_name);
        midiInStart(m_handle);
        result = true;

        /* Open feedback output */
        openOutput();
    }

    return result;
}

void MIDIDevice::close()
{
    MMRESULT res;

    res = midiInClose(m_handle);
    if (res == MIDIERR_STILLPLAYING)
    {
        qDebug() << QString("Unable to close %1: Buffer not empty").arg(m_name);
    }
    else if (res == MMSYSERR_INVALHANDLE)
    {
        qDebug() << QString("Unable to close %1: Invalid handle").arg(m_name);
    }
    else if (res == MMSYSERR_NOMEM)
    {
        qDebug() << QString("Unable t close %1: Out of memory").arg(m_name);
    }
    else
    {
        qDebug() << QString("%1 closed successfully.").arg(m_name);
        m_handle = NULL;

        /* Close feedback output */
        closeOutput();
    }
}

/*****************************************************************************
 * Device info
 *****************************************************************************/

QString MIDIDevice::infoText()
{
    QString info;
    info += QString("<B>%1</B>").arg(name());

    if (m_isOK == true)
    {
        info += QString("<P>");
        info += tr("Device is working correctly.");
        info += QString("</P>");
        info += QString("<P>");

        // MIDI channel
        info += QString("<B>%1:</B> ").arg(tr("MIDI Channel"));
        if (midiChannel() < 16)
            info += QString("%1<BR/>").arg(midiChannel() + 1);
        else
            info += QString("%1<BR/>").arg(tr("Any"));

        // Feedback line
        QStringList fbNames(feedBackNames());
        info += QString("<B>%1:</B> ").arg(tr("Feedback Line"));
        if (feedBackId() < quint32(fbNames.size()))
            info += QString("%1<BR/>").arg(fbNames[feedBackId()]);
        else
            info += QString("%1<BR/>").arg(tr("None"));

        info += QString("</P>");
    }
    else
    {
        info += QString("<P>");
        info += tr("MIDI Input is not available.");
        info += QString("</P>");
    }

    return info;
}

QString MIDIDevice::name()
{
    return m_name;
}

void MIDIDevice::extractName()
{
    MIDIINCAPS inCaps;
    MMRESULT res;

    /* Extract name */
    m_name = QString("MIDI Input %1: ").arg(m_id + 1);
    res = midiInGetDevCaps(m_id, &inCaps, sizeof(MIDIINCAPS));
    if (res == MMSYSERR_BADDEVICEID)
    {
        m_name += QString("Bad device ID");
        qDebug() << "MIDI IN device" << m_id + 1 << "has bad device ID";
    }
    else if (res == MMSYSERR_INVALPARAM)
    {
        m_name += QString("Invalid parameters");
        qDebug() << "Invalid params for MIDI IN device" << m_id + 1;
    }
    else if (res == MMSYSERR_NODRIVER)
    {
        m_name += QString("No driver installed");
        qDebug() << "MIDI IN device" << m_id + 1 << "has no driver";
    }
    else if (res == MMSYSERR_NOMEM)
    {
        m_name += QString("Out of memory");
        qDebug() << "Out of memory while opening MIDI IN" << m_id + 1;
    }
    else
    {
        m_name += QString::fromWCharArray(inCaps.szPname);
        m_isOK = true;
    }
}

/*****************************************************************************
 * Input data
 *****************************************************************************/

void MIDIDevice::customEvent(QEvent* event)
{
    if (event->type() == MIDIInputEvent::eventType)
    {
        MIDIInputEvent* e = static_cast<MIDIInputEvent*> (event);
        emit valueChanged(e->m_channel, e->m_value);
        event->accept();
    }
}

/*****************************************************************************
 * Feedback
 *****************************************************************************/

UINT MIDIDevice::feedBackId() const
{
    return m_feedBackId;
}

void MIDIDevice::setFeedBackId(UINT id)
{
    QSettings settings;
    bool reopen = false;
    QString key;

    if (m_feedBackHandle != NULL)
    {
        reopen = true;
        closeOutput();
    }

    m_feedBackId = id;

    /* Save the feedback line number */
    key = QString("/MIDIInput/Inputs/%1/FeedBackLine").arg(m_id);
    settings.setValue(key, id);

    if (m_feedBackId  != UINT_MAX && reopen == true)
        openOutput();
}

void MIDIDevice::openOutput()
{
    if (m_feedBackHandle == NULL && m_feedBackId != UINT_MAX)
    {
        MMRESULT res;
        res = midiOutOpen(&m_feedBackHandle, m_feedBackId, 0, 0, 0);
        if (res != MMSYSERR_NOERROR)
            m_feedBackHandle = NULL;
    }
}

void MIDIDevice::closeOutput()
{
    if (m_feedBackHandle != NULL)
    {
        MMRESULT res;
        res = midiOutClose(m_feedBackHandle);
        if (res == MMSYSERR_NOERROR)
            m_feedBackHandle = NULL;
    }
}

QStringList MIDIDevice::feedBackNames()
{
    QStringList list;
    MIDIOUTCAPS caps;
    MMRESULT res;
    QString name;
    UINT num;

    num = midiOutGetNumDevs();
    for (UINT id = 0; id < num; id++)
    {
        res = midiOutGetDevCaps(id, &caps, sizeof(MIDIOUTCAPS));
        if (res == MMSYSERR_NOERROR)
        {
            name = QString("MIDI Output %1: %2").arg(id + 1)
                   .arg(QString::fromWCharArray(caps.szPname));
            list << name;
        }
    }

    return list;
}

void MIDIDevice::feedBack(quint32 channel, uchar value)
{
    /* Attempt to open the same line for feedback output as well */
    if (m_feedBackId != UINT_MAX && m_feedBackHandle == NULL)
        openOutput();

    uchar cmd = 0;
    uchar data1 = 0;
    uchar data2 = 0;
    bool d2v = false;
    if (QLCMIDIProtocol::feedbackToMidi(channel, value, midiChannel(),
        &cmd, &data1, &data2, &d2v) == true)
    {
        union
        {
            DWORD dwData;
            BYTE bData[4];
        } msg;

        msg.bData[0] = (BYTE) cmd;
        msg.bData[1] = (BYTE) data1;
        msg.bData[2] = (BYTE) data2;
        msg.bData[3] = 0;

        /* Push the message out */
        midiOutShortMsg(m_feedBackHandle, msg.dwData);
    }
}
