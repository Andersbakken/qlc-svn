/*
  Q Light Controller - Unit test
  scene_stub.cpp

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

#include "doc.h"
#include "scene_stub.h"
#include "universearray.h"

/****************************************************************************
 * Scene Stub
 ****************************************************************************/

SceneStub::SceneStub(Doc* doc) : Scene(doc)
{
}

SceneStub::~SceneStub()
{
}

void SceneStub::setValue(quint32 address, uchar value)
{
    m_values[address] = value;
}

void SceneStub::writeValues(UniverseArray* array, t_fixture_id fxi_id)
{
    Q_UNUSED(fxi_id);

    QMapIterator <quint32,uchar> it(m_values);
    while (it.hasNext() == true)
    {
        it.next();
        array->write(it.key(), it.value(), QLCChannel::Intensity);
    }
}
