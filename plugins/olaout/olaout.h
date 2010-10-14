/*
  Q Light Controller
  olaout.h

  Copyright (c) Heikki Junnila
                Simon Newton

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

#ifndef OLAOUT_H
#define OLAOUT_H

#include <QObject>
#include <QDebug>
#include <QList>
#include <ola/Logging.h>
#include "qlcoutplugin.h"
#include "qlctypes.h"
#include "olaoutthread.h"

class ConfigureOlaOut;

// Number of output universes
enum { K_UNIVERSE_COUNT = 4 };

typedef QList<unsigned int> OutputList;

// The OLA Output plugin
class OLAOut : public QLCOutPlugin
{
    Q_OBJECT
    Q_INTERFACES(QLCOutPlugin)
    friend class ConfigureOlaOut;

public:
    ~OLAOut();
    void init();
    void open(quint32 output=0);
    void close(quint32 output=0);
    QStringList outputs();
    QString name();
    void configure();
    bool canConfigure();
    QString infoText(quint32 output = KOutputInvalid);

    void outputDMX(quint32 output, const QByteArray& universe);

    const OutputList outputMapping() const;
    void setOutputUniverse(quint32 output, unsigned int universe);
    bool isServerEmbedded();
    void setServerEmbedded(bool embedServer);

protected:
    QString m_configDir;
    OlaOutThread *m_thread;
    OutputList m_output_list;
    ola::LogDestination *m_log_destination;
    bool m_embedServer;
};

#endif
