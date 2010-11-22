/*
  Q Light Controller
  eprogramwing.h

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

#ifndef EPROGRAMWING_H
#define EPROGRAMWING_H

#include <QHostAddress>
#include <QByteArray>
#include <QObject>

#include "ewing.h"

/****************************************************************************
 * EProgramWing
 ****************************************************************************/

class QLC_DECLSPEC EProgramWing : public EWing
{
    Q_OBJECT

    /********************************************************************
     * Initialization
     ********************************************************************/
public:
    /**
     * Construct a new EProgramWing object. This object represents an
     * ENTTEC Program Wing at the given IP address.
     *
     * @param parent The parent object that owns the new wing object.
     * @param address The address of the physical wing board.
     * @param data A UDP datagram packet originating from a wing.
     */
    EProgramWing(QObject* parent, const QHostAddress& address,
                 const QByteArray& data);

    /**
     * Destructor.
     */
    ~EProgramWing();

    /** @reimp */
    QString name() const;

    /********************************************************************
     * Input data
     ********************************************************************/
public:
    /** @reimp */
    void parseData(const QByteArray& data);

protected:
    QMap <int,int> m_channelMap;
};

#endif
