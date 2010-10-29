/*
  Q Light Controller - Unit test
  scene_stub.h

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

#ifndef SCENE_STUB_H
#define SCENE_STUB_H

#include <QObject>
#include <QMap>

#include "fixture.h"
#include "scene.h"

class Doc;

/****************************************************************************
 * Scene Stub
 ****************************************************************************/

class SceneStub : public Scene
{
    Q_OBJECT

public:
    SceneStub(Doc* doc);
    ~SceneStub();

    void setValue(quint32 address, uchar value);
    void writeValues(QByteArray* universes,
                     t_fixture_id fxi_id = Fixture::invalidId());

    QMap <quint32,uchar> m_values;
};

#endif

