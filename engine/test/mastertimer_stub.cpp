/*
  Q Light Controller - Unit test
  mastertimer_stub.cpp

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

#include "mastertimer_stub.h"
#include "function.h"

/****************************************************************************
 * MasterTimer Stub
 ****************************************************************************/

MasterTimerStub::MasterTimerStub(QObject* parent, OutputMap* outputMap,
                                 UniverseArray& universes)
        : MasterTimer(parent, outputMap),
        m_universes(universes)
{
}

MasterTimerStub::~MasterTimerStub()
{
}

void MasterTimerStub::startFunction(Function* function, bool initiatedByOtherFunction)
{
    m_functionList.append(function);
    function->setInitiatedByOtherFunction(initiatedByOtherFunction);
    function->preRun(this);
}

void MasterTimerStub::stopFunction(Function* function)
{
    m_functionList.removeAll(function);
    function->postRun(this, &m_universes);
}

void MasterTimerStub::registerDMXSource(DMXSource* source)
{
    m_dmxSourceList.append(source);
}

void MasterTimerStub::unregisterDMXSource(DMXSource* source)
{
    m_dmxSourceList.removeAll(source);
}

