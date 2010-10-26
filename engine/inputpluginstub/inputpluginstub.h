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
#include "qlcinplugin.h"

/*****************************************************************************
 * InputPlugin
 *****************************************************************************/

class InputPluginStub : public QLCInPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCInPlugin)

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    virtual ~InputPluginStub();

    /** @reimp */
    void init();

    /** @reimp */
    QString name();

    /*************************************************************************
     * Inputs
     *************************************************************************/
public:
    /** @reimp */
    void open(quint32 input = 0);

    /** @reimp */
    void close(quint32 input = 0);

    /** @reimp */
    QStringList inputs();

    /** @reimp */
    QString infoText(quint32 input = KInputInvalid);

    /** Tell the plugin to emit valueChanged signal */
    void emitValueChanged(quint32 input, quint32 channel, uchar value) {
        emit valueChanged(input, channel, value);
    }

public:
    /** List of input lines opened with open() and not closed with close() */
    QList <quint32> m_openLines;

    /*************************************************************************
     * Configuration
     *************************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();

public:
    /** Number of times configure() has been called */
    int m_configureCalled;

    /** Return value for canConfigure() */
    bool m_canConfigure;

    /*************************************************************************
     * Feedback
     *************************************************************************/
public:
    /** @reimp */
    void feedBack(quint32 input, quint32 channel, uchar value);

public:
    /** $input of the latest call to feedBack() */
    quint32 m_feedBackInput;

    /** $channel of the latest call to feedBack() */
    quint32 m_feedBackChannel;

    /** $value of the latest call to feedBack() */
    uchar m_feedBackValue;
};

#endif
