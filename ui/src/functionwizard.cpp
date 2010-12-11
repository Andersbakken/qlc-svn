/*
  Q Light Controller
  functionwizard.cpp

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

#include <QString>
#include <QDebug>
#include <QHash>

#include <cstdlib>

#include "intensitygenerator.h"
#include "palettegenerator.h"
#include "fixtureselection.h"
#include "functionwizard.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "app.h"
#include "doc.h"

#include "qlccapability.h"
#include "qlcchannel.h"

#define KColumnName 0
#define KColumnCaps 1
#define KColumnID   2

extern App* _app;

FunctionWizard::FunctionWizard(QWidget* parent) : QDialog(parent)
{
    setupUi(this);
    m_fixtureTree->sortItems(KColumnName, Qt::AscendingOrder);
}

FunctionWizard::~FunctionWizard()
{
}

void FunctionWizard::slotAddClicked()
{
    FixtureSelection fs(this, _app->doc(), true, fixtureIds());
    if (fs.exec() == QDialog::Accepted)
    {
        QListIterator <t_fixture_id> it(fs.selection);
        while (it.hasNext() == true)
            addFixture(it.next());
    }
}

void FunctionWizard::slotRemoveClicked()
{
    QListIterator <QTreeWidgetItem*> it(m_fixtureTree->selectedItems());
    while (it.hasNext() == true)
        delete it.next();
}

void FunctionWizard::accept()
{
    PaletteGenerator pal(_app->doc(), fixtures());

    if (m_coloursCheck->isChecked() == true)
        pal.createColours();
    if (m_goboCheck->isChecked() == true)
        pal.createGobos();
    if (m_shutterCheck->isChecked() == true)
        pal.createShutters();

    if (m_intensityCheck->isChecked() == true)
    {
        IntensityGenerator gen(_app->doc(), fixtures());
        gen.createOddEvenChaser();
        gen.createFullZeroChaser();
        gen.createSequenceChasers();
        gen.createRandomChaser();
    }

    QDialog::accept();
}

/****************************************************************************
 * Fixtures
 ****************************************************************************/

void FunctionWizard::addFixture(t_fixture_id fxi_id)
{
    Fixture* fxi = _app->doc()->fixture(fxi_id);
    Q_ASSERT(fxi != NULL);

    QTreeWidgetItem* item = new QTreeWidgetItem(m_fixtureTree);
    item->setText(KColumnName, fxi->name());
    item->setData(KColumnID, Qt::UserRole, fxi_id);

    QStringList caps;
    if (!IntensityGenerator::findChannels(fxi, QLCChannel::Colour).isEmpty())
        caps << QLCChannel::groupToString(QLCChannel::Colour);

    if (!IntensityGenerator::findChannels(fxi, QLCChannel::Gobo).isEmpty())
        caps << QLCChannel::groupToString(QLCChannel::Gobo);

    if (!IntensityGenerator::findChannels(fxi, QLCChannel::Shutter).isEmpty())
        caps << QLCChannel::groupToString(QLCChannel::Shutter);

    if (!IntensityGenerator::findChannels(fxi, QLCChannel::Intensity).isEmpty())
        caps << QLCChannel::groupToString(QLCChannel::Intensity);

    item->setText(KColumnCaps, caps.join(", "));
}

QList <Fixture*> FunctionWizard::fixtures() const
{
    QList <Fixture*> list;
    for (int i = 0; i < m_fixtureTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item(m_fixtureTree->topLevelItem(i));
        Q_ASSERT(item != NULL);

        t_fixture_id id = item->data(KColumnID, Qt::UserRole).toInt();
        Fixture* fxi = _app->doc()->fixture(id);
        Q_ASSERT(fxi != NULL);

        list << fxi;
    }

    return list;
}

QList <t_fixture_id> FunctionWizard::fixtureIds() const
{
    QList <t_fixture_id> list;
    for (int i = 0; i < m_fixtureTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item(m_fixtureTree->topLevelItem(i));
        Q_ASSERT(item != NULL);

        list << item->data(KColumnID, Qt::UserRole).toInt();
    }

    return list;
}
