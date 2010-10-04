/*
  Q Light Controller
  fixtureselection.h

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

#ifndef FIXTURESELECTION_H
#define FIXTURESELECTION_H

#include <QDialog>
#include <QList>

#include "qlctypes.h"
#include "ui_fixtureselection.h"

class QTreeWidgetItem;
class QWidget;

class Fixture;
class Doc;

class FixtureSelection : public QDialog, public Ui_FixtureSelection
{
    Q_OBJECT
public:
    /**
     * Constructor
     *
     * @param parent The parent widget
     * @param doc The Doc* object holding all fixtures
     * @param multiple Set true to enable multiple selection
     * @param disabled List of fixture IDs to disable (already present
     *                 fixtures etc...)
     */
    FixtureSelection(QWidget* parent, Doc* doc, bool multiple,
                     QList <t_fixture_id> disabled);

    /**
     * Destructor
     */
    ~FixtureSelection();

    /**
     * List of selected fixture IDs
     */
    QList <t_fixture_id> selection;

private:
    Q_DISABLE_COPY(FixtureSelection)

protected slots:
    /**
     * Item double clicks
     */
    void slotItemDoubleClicked(QTreeWidgetItem* item);

    /**
     * OK button click
     */
    void accept();
};

#endif
