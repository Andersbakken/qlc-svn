/*
  Q Light Controller
  inputplugin_stub.h

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

#ifndef INPUTPLUGIN_STUB_H
#define INPUTPLUGIN_STUB_H

#include <QStringList>
#include <QtPlugin>
#include <QString>

#include "qlcinplugin.h"

/*****************************************************************************
 * InputPlugin
 *****************************************************************************/

class InputPluginStub : public QLCInPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCInPlugin)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    InputPluginStub();
    virtual ~InputPluginStub();

    void init();
    QString name();

    /*********************************************************************
     * Inputs
     *********************************************************************/
public:
    void open(quint32 input = 0);
    void close(quint32 input = 0);
    QStringList inputs();
    QString infoText(quint32 input = KInputInvalid);

    QList <quint32> m_openLines;

    void emitValueChanged(quint32 input, quint32 channel, uchar value);

    /*********************************************************************
     * Configuration
     *********************************************************************/
public:
    void configure();

    bool canConfigure();

public:
    int m_configureCalled;
    bool m_canConfigure;

    /*********************************************************************
     * Feedback
     *********************************************************************/
public:
    void feedBack(quint32 input, quint32 channel, uchar value);

    quint32 m_feedBackInput;
    quint32 m_feedBackChannel;
    uchar m_feedBackValue;
};

#endif
