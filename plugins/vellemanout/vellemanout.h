/*
  Q Light Controller
  vellemanout.h

  Copyright (c) Matthew Jaggard

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

#ifndef VELLEMANOUT_H
#define VELLEMANOUT_H

#include <QString>

#include "qlcoutplugin.h"
#include "qlctypes.h"

class VellemanOut : public QObject, public QLCOutPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCOutPlugin)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    void init();

    /*********************************************************************
     * Open/close
     *********************************************************************/
public:
    void open(quint32 output = 0);
    void close(quint32 output = 0);
    QStringList outputs();

protected:
    bool m_currentlyOpen;

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
     * Write
     *********************************************************************/
public:
    void outputDMX(quint32 output, const QByteArray& universe);
};

#endif
