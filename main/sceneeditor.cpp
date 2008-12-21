/*
  Q Light Controller
  sceneeditor.cpp

  Copyright (c) Heikki Junnila, Stefan Krumm

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

#include <QTreeWidgetItem>
#include <QInputDialog>
#include <QTreeWidget>
#include <QMessageBox>
#include <QToolButton>
#include <QTabWidget>
#include <QLayout>
#include <QLabel>
#include <QMenu>

#include "common/qlcfixturedef.h"

#include "fixtureselection.h"
#include "fixtureconsole.h"
#include "sceneeditor.h"
#include "fixture.h"
#include "scene.h"
#include "app.h"
#include "doc.h"

extern App* _app;

#define KColumnName         0
#define KColumnManufacturer 1
#define KColumnModel        2
#define KColumnID           3

#define KTabGeneral         0
#define KTabFirstFixture    1

SceneEditor::SceneEditor(QWidget* parent, Scene* scene) : QDialog(parent)
{
	Q_ASSERT(scene != NULL);
	m_original = scene;

	/* Create a copy of the original scene so that we can freely modify it.
	   Keep also a pointer to the original so that we can move the
	   contents from the copied chaser to the original when OK is clicked */
	m_scene = new Scene(this);
	m_scene->copyFrom(scene);
	Q_ASSERT(m_scene != NULL);

	setupUi(this);
	init();
}

SceneEditor::~SceneEditor()
{
	delete m_scene;
}

void SceneEditor::init()
{
	connect(m_addFixtureButton, SIGNAL(clicked()),
		this, SLOT(slotAddFixtureClicked()));
	connect(m_removeFixtureButton, SIGNAL(clicked()),
		this, SLOT(slotRemoveFixtureClicked()));

	m_nameEdit->setText(m_scene->name());

	m_initializing = true;

	QListIterator <SceneValue> it(*m_scene->values());
	while (it.hasNext() == true)
	{
		SceneValue scv(it.next());

		if (fixtureItem(scv.fxi) == NULL)
		{
			Fixture* fixture = _app->doc()->fixture(scv.fxi);
			if (fixture == NULL)
				continue;

			addFixtureItem(fixture);
			addFixtureTab(fixture);
		}

		setSceneValue(scv);
	}

	m_initializing = false;

	m_tree->sortItems(KColumnName, Qt::AscendingOrder);
}

void SceneEditor::setSceneValue(const SceneValue& scv)
{
	FixtureConsole* fc;
	Fixture* fixture;

	fixture = _app->doc()->fixture(scv.fxi);
	Q_ASSERT(fixture != NULL);

	fc = fixtureConsole(fixture);
	Q_ASSERT(fc != NULL);

	fc->setSceneValue(scv);
}

void SceneEditor::accept()
{
	m_scene->setName(m_nameEdit->text());

	/* Copy the contents of the modified scene over the original scene */
	m_original->copyFrom(m_scene);

	QDialog::accept();
}

/*****************************************************************************
 * General page
 *****************************************************************************/

QTreeWidgetItem* SceneEditor::fixtureItem(t_fixture_id fxi_id)
{
	QTreeWidgetItemIterator it(m_tree);
	while (*it != NULL)
	{
		QTreeWidgetItem* item = *it;
		if (item->text(KColumnID).toInt() == fxi_id)
			return item;
		++it;
	}

	return NULL;
}

QList <Fixture*> SceneEditor::selectedFixtures()
{
	QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
	QList <Fixture*> list;

	while (it.hasNext() == true)
	{
		QTreeWidgetItem* item;
		t_fixture_id fxi_id;
		Fixture* fixture;

		item = it.next();
		fxi_id = item->text(KColumnID).toInt();
		fixture = _app->doc()->fixture(fxi_id);
		Q_ASSERT(fixture != NULL);

		list.append(fixture);
	}

	return list;
}

void SceneEditor::addFixtureItem(Fixture* fixture)
{
	QTreeWidgetItem* item;

	Q_ASSERT(fixture != NULL);

	item = new QTreeWidgetItem(m_tree);
	item->setText(KColumnName, fixture->name());
	item->setText(KColumnID, QString("%1").arg(fixture->id()));

	if (fixture->fixtureDef() == NULL)
	{
		item->setText(KColumnManufacturer, tr("Generic"));
		item->setText(KColumnModel, tr("Generic"));
	}
	else
	{
		item->setText(KColumnManufacturer,
			      fixture->fixtureDef()->manufacturer());
		item->setText(KColumnModel, fixture->fixtureDef()->model());
	}

	/* Select newly-added fixtures so that their channels can be
	   quickly disabled/enabled */
	item->setSelected(true);
}

void SceneEditor::removeFixtureItem(Fixture* fixture)
{
	QTreeWidgetItem* item;

	Q_ASSERT(fixture != NULL);

	item = fixtureItem(fixture->id());
	delete item;
}

void SceneEditor::slotAddFixtureClicked()
{
	/* Put all fixtures already present into a list of fixtures that
	   will be disabled in the fixture selection dialog */
	QList <t_fixture_id> disabled;
	QTreeWidgetItemIterator twit(m_tree);
	while (*twit != NULL)
	{
		disabled.append((*twit)->text(KColumnID).toInt());
		twit++;
	}

	/* Get a list of new fixtures to add to the scene */
	FixtureSelection fs(this, _app->doc(), true, disabled);
	if (fs.exec() == QDialog::Accepted)
	{
		Fixture* fixture;

		QListIterator <t_fixture_id> it(fs.selection);
		while (it.hasNext() == true)
		{
			fixture = _app->doc()->fixture(it.next());
			Q_ASSERT(fixture != NULL);

			addFixtureItem(fixture);
			addFixtureTab(fixture);
		}
	}
}

void SceneEditor::slotRemoveFixtureClicked()
{
	int r = QMessageBox::question(
		this, tr("Remove fixtures"),
		tr("Do you want to remove the selected fixture(s)?"),
		QMessageBox::Yes, QMessageBox::No);

	if (r == QMessageBox::Yes)
	{
		QListIterator <Fixture*> it(selectedFixtures());
		while (it.hasNext() == true)
		{
			Fixture* fixture = it.next();
			Q_ASSERT(fixture != NULL);

			removeFixtureTab(fixture);
			removeFixtureItem(fixture);

			/* Remove all values associated to the fixture */
			for (int i = 0; i < fixture->channels(); i++)
				m_scene->unsetValue(fixture->id(), i);
		}
	}
}

void SceneEditor::slotEnableChannelsClicked()
{
	QListIterator <Fixture*> it(selectedFixtures());
	while (it.hasNext() == true)
	{
		Fixture* fixture;
		FixtureConsole* fc;

		fixture = it.next();
		Q_ASSERT(fixture != NULL);

		fc = fixtureConsole(fixture);
		Q_ASSERT(fc != NULL);

		fc->enableAllChannels(true);
	}
}

void SceneEditor::slotDisableChannelsClicked()
{
	QListIterator <Fixture*> it(selectedFixtures());
	while (it.hasNext() == true)
	{
		Fixture* fixture;
		FixtureConsole* fc;

		fixture = it.next();
		Q_ASSERT(fixture != NULL);

		fc = fixtureConsole(fixture);
		Q_ASSERT(fc != NULL);

		fc->enableAllChannels(false);
	}
}

/*****************************************************************************
 * Fixture tabs
 *****************************************************************************/

FixtureConsole* SceneEditor::fixtureConsole(Fixture* fixture)
{
	Q_ASSERT(fixture != NULL);

	/* Start from the first fixture tab */
	for (int i = KTabFirstFixture; i < m_tab->count(); i++)
		if (m_tab->tabText(i) == fixture->name())
			return qobject_cast<FixtureConsole*> (m_tab->widget(i));
	return NULL;
}

void SceneEditor::addFixtureTab(Fixture* fixture)
{
	FixtureConsole* console;

	Q_ASSERT(fixture != NULL);

	console = new FixtureConsole(this);
	console->setChannelsCheckable(true);
	console->setFixture(fixture->id());
	m_tab->addTab(console, fixture->name());

	/* Start off with all channels disabled */
	console->enableAllChannels(false);

	connect(console,
		SIGNAL(valueChanged(t_fixture_id,t_channel,t_value,bool)),
		this,
		SLOT(slotValueChanged(t_fixture_id,t_channel,t_value,bool)));
}

void SceneEditor::removeFixtureTab(Fixture* fixture)
{
	Q_ASSERT(fixture != NULL);

	/* Start from the first fixture tab */
	for (int i = KTabFirstFixture; i < m_tab->count(); i++)
	{
		if (m_tab->tabText(i) == fixture->name())
		{
			QWidget* console = m_tab->widget(i);
			Q_ASSERT(console != NULL);
			delete console;
			m_tab->removeTab(i);
			break;
		}
	}
}

void SceneEditor::slotValueChanged(t_fixture_id fxi_id, t_channel channel,
				   t_value value, bool enabled)
{
	/* Don't accept any changes during initialization */
	if (m_initializing == true)
		return;

	if (enabled == true)
		m_scene->setValue(fxi_id, channel, value);
	else
		m_scene->unsetValue(fxi_id, channel);
}
