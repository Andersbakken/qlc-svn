/*
  Q Light Controller
  functionwizard.h

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

#ifndef FUNCTIONWIZARD_H
#define FUNCTIONWIZARD_H

#include <QDialog>
#include <QList>

#include "ui_functionwizard.h"
#include "qlctypes.h"

class QLCChannel;
class Fixture;
class Scene;

class FunctionWizard : public QDialog, public Ui_FunctionWizard
{
	Q_OBJECT

public:
	FunctionWizard(QWidget* parent = 0);
	~FunctionWizard();

protected slots:
	void slotAddClicked();
	void slotRemoveClicked();
	void accept();

	/********************************************************************
	 * Fixtures
	 ********************************************************************/
protected:
	/** Add a fixture to the tree widget */
	void addFixture(t_fixture_id fxi_id);

	/** Get a list of fixtures in the tree widget */
	QList <Fixture*> fixtures() const;

	/********************************************************************
	 * Function creation
	 ********************************************************************/
protected:
	/**
	 * Create functions for the given fixture for each capability found
	 * from the fixture's channels that belong to the given group.
	 *
	 * @param fxi The fixture to create functions for
	 * @param group The channel group to create functions from
	 */
	void createGroupFunctions(const Fixture* fxi, const QString& group);

	/**
	 * Find channels from the given fixture that belong to the given group.
	 *
	 * @param fixture The fixture to find the channels from
	 * @param group The channel group to find
	 * @return List of channel numbers (empty if none found)
	 */
	QList <t_channel> findChannels(const Fixture* fixture,
					const QString& group) const;

protected:
	QHash <QString,Scene*> m_scenes;
};

#endif

