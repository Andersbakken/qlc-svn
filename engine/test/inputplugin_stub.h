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
#include <QObject>

#include "qlcinplugin.h"
#include "qlctypes.h"

/*****************************************************************************
 * InputPlugin
 *****************************************************************************/

class InputPluginStub : public QObject, public QLCInPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCInPlugin)
    Q_DISABLE_COPY(InputPluginStub)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    InputPluginStub();
    ~InputPluginStub();

    void init();

    /*********************************************************************
     * Inputs
     *********************************************************************/
public:
    void open(quint32 input = 0);
    void close(quint32 input = 0);
    QStringList inputs();

    QList <quint32> m_openLines;

    /*********************************************************************
     * Name
     *********************************************************************/
public:
    QString name();

    /*********************************************************************
     * Configuration
     *********************************************************************/
public:
    void configure();

    /** Number of times configure() has been called */
    int m_configureCalled;

    /*********************************************************************
     * Status
     *********************************************************************/
public:
    QString infoText(quint32 input = KInputInvalid);

    /*********************************************************************
     * Input data listener
     *********************************************************************/
public:
    void connectInputData(QObject* listener);

    /*********************************************************************
     * Feedback
     *********************************************************************/
public:
    void feedBack(quint32 input, quint32 channel,
                  uchar value);

    quint32 m_feedBackInput;
    quint32 m_feedBackChannel;
    uchar m_feedBackValue;
};

#endif
