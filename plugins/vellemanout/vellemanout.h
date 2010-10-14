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

class VellemanOut : public QLCOutPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCOutPlugin)

    /*************************************************************************
     * Initialization
     *************************************************************************/
public:
    /** @reimp */
    virtual ~VellemanOut();

    /** @reimp */
    void init();

    /** @reimp */
    QString name();

    /*************************************************************************
     * Open/close
     *************************************************************************/
public:
    /** @reimp */
    void open(quint32 output = 0);

    /** @reimp */
    void close(quint32 output = 0);

    /** @reimp */
    QStringList outputs();

    /** @reimp */
    QString infoText(quint32 output = KOutputInvalid);

    /** @reimp */
    void outputDMX(quint32 output, const QByteArray& universe);

protected:
    bool m_currentlyOpen;

    /*************************************************************************
     * Configuration
     *************************************************************************/
public:
    /** @reimp */
    void configure();

    /** @reimp */
    bool canConfigure();
};

#endif
