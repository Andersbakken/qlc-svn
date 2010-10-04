/*
  Q Light Controller
  outputplugin_stub.h

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

#ifndef OUTPUTPLUGIN_STUB_H
#define OUTPUTPLUGIN_STUB_H

#include <QObject>
#include <QList>

#include "qlcoutplugin.h"
#include "qlctypes.h"

class OutputPluginStub : public QObject, public QLCOutPlugin
{
    Q_OBJECT
    Q_DISABLE_COPY(OutputPluginStub)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    OutputPluginStub();
    virtual ~OutputPluginStub();

    void init();

    /*********************************************************************
     * Open/close
     *********************************************************************/
public:
    void open(quint32 output = 0);
    void close(quint32 output = 0);
    QStringList outputs();

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

    /*********************************************************************
     * Status
     *********************************************************************/
public:
    QString infoText(quint32 output = KOutputInvalid);

    /*********************************************************************
     * Value Read/Write
     *********************************************************************/
public:
    void outputDMX(quint32 output, const QByteArray& universe);
    QByteArray m_array;
};

#endif
