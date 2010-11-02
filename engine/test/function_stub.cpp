/*
  Q Light Controller
  function_stub.cpp

  Copyright (C) Heikki Junnila

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

#include "doc.h"
#include "fixture.h"
#include "function_stub.h"

Function_Stub::Function_Stub(Doc* doc) : Function(doc)
{
    m_writeCalls = 0;
    m_preRunCalls = 0;
    m_postRunCalls = 0;
    m_slotFixtureRemovedId = Fixture::invalidId();
    m_type = Function::Type(0xDEADBEEF);
}

Function_Stub::~Function_Stub()
{
}

Function* Function_Stub::createCopy(Doc* parent)
{
    Q_UNUSED(parent);
    return NULL;
}

Function::Type Function_Stub::type() const
{
    return m_type;
}

bool Function_Stub::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    Q_UNUSED(doc);
    Q_UNUSED(wksp_root);

    return false;
}

bool Function_Stub::loadXML(const QDomElement* root)
{
    Q_UNUSED(root);
    return false;
}

void Function_Stub::arm()
{
}

void Function_Stub::disarm()
{
}

void Function_Stub::preRun(MasterTimer* timer)
{
    Q_UNUSED(timer);
    m_preRunCalls++;
    Function::preRun(timer);
}

void Function_Stub::write(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer);
    Q_UNUSED(universes);

    incrementElapsed();
    m_writeCalls++;
}

void Function_Stub::postRun(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer);
    Q_UNUSED(universes);
    m_postRunCalls++;
    Function::postRun(timer, universes);
}

void Function_Stub::slotFixtureRemoved(t_fixture_id id)
{
    m_slotFixtureRemovedId = id;
    Function::slotFixtureRemoved(id);
}
