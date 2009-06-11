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
#include <QColorDialog>
#include <QTreeWidget>
#include <QMessageBox>
#include <QToolButton>
#include <QTabWidget>
#include <QToolBar>
#include <QLayout>
#include <QLabel>

#include "common/qlcfixturedef.h"
#include "common/qlcchannel.h"

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

	m_currentTab = KTabGeneral;

	/* Create a copy of the original scene so that we can freely modify it.
	   Keep also a pointer to the original so that we can move the
	   contents from the copied chaser to the original when OK is clicked */
	m_scene = new Scene(this);
	m_scene->copyFrom(scene);
	Q_ASSERT(m_scene != NULL);

	setupUi(this);
	init();
	slotTabChanged(KTabGeneral);
}

SceneEditor::~SceneEditor()
{
	delete m_scene;
}

void SceneEditor::init()
{
	QToolBar* toolBar;

	/* Actions */
	m_enableCurrentAction = new QAction(QIcon(":/check.png"),
			tr("Enable all channels in current fixture"), this);
	m_disableCurrentAction = new QAction(QIcon(":/uncheck.png"),
			tr("Disable all channels in current fixture"), this);
	m_copyAction = new QAction(QIcon(":/editcopy.png"),
			tr("Copy current values to clipboard"), this);
	m_pasteAction = new QAction(QIcon(":/editpaste.png"),
			tr("Paste clipboard values to current fixture"), this);
	m_copyToAllAction = new QAction(QIcon(":/editcopyall.png"),
			tr("Copy current values to all fixtures"), this);
	m_colorToolAction = new QAction(QIcon(":/color.png"),
			tr("Color tool for CMY/RGB-capable fixtures"), this);

	connect(m_enableCurrentAction, SIGNAL(triggered(bool)),
		this, SLOT(slotEnableCurrent()));
	connect(m_disableCurrentAction, SIGNAL(triggered(bool)),
		this, SLOT(slotDisableCurrent()));
	connect(m_copyAction, SIGNAL(triggered(bool)),
		this, SLOT(slotCopy()));
	connect(m_pasteAction, SIGNAL(triggered(bool)),
		this, SLOT(slotPaste()));
	connect(m_copyToAllAction, SIGNAL(triggered(bool)),
		this, SLOT(slotCopyToAll()));
	connect(m_colorToolAction, SIGNAL(triggered(bool)),
		this, SLOT(slotColorTool()));

	/* Toolbar */
	toolBar = new QToolBar(this);
	layout()->setMenuBar(toolBar);
	toolBar->addAction(m_enableCurrentAction);
	toolBar->addAction(m_disableCurrentAction);
	toolBar->addSeparator();
	toolBar->addAction(m_copyAction);
	toolBar->addAction(m_pasteAction);
	toolBar->addAction(m_copyToAllAction);
	toolBar->addSeparator();
	toolBar->addAction(m_colorToolAction);

	/* Tab widget */
	connect(m_tab, SIGNAL(currentChanged(int)),
		this, SLOT(slotTabChanged(int)));

	/* Add & remove buttons */
	connect(m_addFixtureButton, SIGNAL(clicked()),
		this, SLOT(slotAddFixtureClicked()));
	connect(m_removeFixtureButton, SIGNAL(clicked()),
		this, SLOT(slotRemoveFixtureClicked()));

	m_nameEdit->setText(m_scene->name());
	connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
		this, SLOT(slotNameEdited(const QString&)));
	slotNameEdited(m_scene->name());

	/* Bus */
	connect(m_busCombo, SIGNAL(activated(int)),
		this, SLOT(slotBusComboActivated(int)));
	fillBusCombo();

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
}

void SceneEditor::fillBusCombo()
{
	m_busCombo->clear();
	m_busCombo->addItems(Bus::instance()->idNames());
	m_busCombo->setCurrentIndex(m_scene->busID());
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

/*****************************************************************************
 * Common
 *****************************************************************************/

void SceneEditor::slotNameEdited(const QString& name)
{
	setWindowTitle(tr("Scene - %1").arg(name));
}

void SceneEditor::slotBusComboActivated(int index)
{
	Q_ASSERT(m_scene != NULL);
	m_scene->setBus(index);
}

void SceneEditor::accept()
{
	m_scene->setName(m_nameEdit->text());

	/* Copy the contents of the modified scene over the original scene */
	m_original->copyFrom(m_scene);

	QDialog::accept();
}

void SceneEditor::slotTabChanged(int tab)
{
	FixtureConsole* fc;

	/* Disable external input from the previous console tab so that
	   only the curren tab is affected. */
	fc = qobject_cast<FixtureConsole*> (m_tab->widget(m_currentTab));
	if (fc != NULL)
		fc->enableExternalInput(false);
	m_currentTab = tab;

	if (tab == KTabGeneral)
	{
		m_enableCurrentAction->setEnabled(false);
		m_disableCurrentAction->setEnabled(false);

		m_copyAction->setEnabled(false);
		m_pasteAction->setEnabled(false);
		m_copyToAllAction->setEnabled(false);
		m_colorToolAction->setEnabled(false);
	}
	else
	{
		m_enableCurrentAction->setEnabled(true);
		m_disableCurrentAction->setEnabled(true);

		m_copyAction->setEnabled(true);
		if (m_copy.isEmpty() == false)
			m_pasteAction->setEnabled(true);

		m_copyToAllAction->setEnabled(true);
		m_colorToolAction->setEnabled(isColorToolAvailable());

		/* Enable external input on the current console tab */
		fc = qobject_cast<FixtureConsole*> (m_tab->widget(tab));
		Q_ASSERT(fc != NULL);
		fc->enableExternalInput(true);
	}
}

void SceneEditor::slotEnableAll()
{
	for (int i = KTabFirstFixture; i < m_tab->count(); i++)
	{
		FixtureConsole* fc;
		fc = qobject_cast<FixtureConsole*> (m_tab->widget(i));
		if (fc != NULL)
			fc->enableAllChannels(true);
	}
}

void SceneEditor::slotDisableAll()
{
	for (int i = KTabFirstFixture; i < m_tab->count(); i++)
	{
		FixtureConsole* fc;
		fc = qobject_cast<FixtureConsole*> (m_tab->widget(i));
		if (fc != NULL)
			fc->enableAllChannels(false);
	}
}

void SceneEditor::slotEnableCurrent()
{
	FixtureConsole* fc;

	/* QObject cast fails unless the widget is a FixtureConsole */
	fc = qobject_cast<FixtureConsole*> (m_tab->currentWidget());
	if (fc != NULL)
		fc->enableAllChannels(true);
}

void SceneEditor::slotDisableCurrent()
{
	FixtureConsole* fc;

	/* QObject cast fails unless the widget is a FixtureConsole */
	fc = qobject_cast<FixtureConsole*> (m_tab->currentWidget());
	if (fc != NULL)
		fc->enableAllChannels(false);
}

void SceneEditor::slotCopy()
{
	FixtureConsole* fc;

	/* QObject cast fails unless the widget is a FixtureConsole */
	fc = qobject_cast<FixtureConsole*> (m_tab->currentWidget());
	if (fc != NULL)
	{
		m_copy = fc->values();
		m_pasteAction->setEnabled(true);
	}
}

void SceneEditor::slotPaste()
{
	FixtureConsole* fc;

	/* QObject cast fails unless the widget is a FixtureConsole */
	fc = qobject_cast<FixtureConsole*> (m_tab->currentWidget());
	if (fc != NULL && m_copy.isEmpty() == false)
		fc->setValues(m_copy);
}

void SceneEditor::slotCopyToAll()
{
	slotCopy();

	for (int i = KTabFirstFixture; i < m_tab->count(); i++)
	{
		FixtureConsole* fc;
		fc = qobject_cast<FixtureConsole*> (m_tab->widget(i));
		if (fc != NULL)
			fc->setValues(m_copy);
	}

	m_copy.clear();
	m_pasteAction->setEnabled(false);
}

void SceneEditor::slotColorTool()
{
	FixtureConsole* fc;
	Fixture* fxi;
	QColor color;
	t_channel cyan, magenta, yellow;
	t_channel red, green, blue;

	/* QObject cast fails unless the widget is a FixtureConsole */
	fc = qobject_cast<FixtureConsole*> (m_tab->currentWidget());
	if (fc == NULL)
		return;

	fxi = _app->doc()->fixture(fc->fixture());
	Q_ASSERT(fxi != NULL);

	cyan = fxi->channel("cyan", Qt::CaseInsensitive, KQLCChannelGroupColour);
	magenta = fxi->channel("magenta", Qt::CaseInsensitive, KQLCChannelGroupColour);
	yellow = fxi->channel("yellow", Qt::CaseInsensitive, KQLCChannelGroupColour);
	red = fxi->channel("red", Qt::CaseInsensitive, KQLCChannelGroupColour);
	green = fxi->channel("green", Qt::CaseInsensitive, KQLCChannelGroupColour);
	blue = fxi->channel("blue", Qt::CaseInsensitive, KQLCChannelGroupColour);

	if (cyan != KChannelInvalid && magenta != KChannelInvalid &&
	    yellow != KChannelInvalid)
	{
		color.setCmyk(fc->channel(cyan)->sliderValue(),
			      fc->channel(magenta)->sliderValue(),
			      fc->channel(yellow)->sliderValue(), 0);

		color = QColorDialog::getColor(color);
		if (color.isValid() == true)
		{
			fc->channel(cyan)->setValue(color.cyan());
			fc->channel(magenta)->setValue(color.magenta());
			fc->channel(yellow)->setValue(color.yellow());

			fc->channel(cyan)->enable(true);
			fc->channel(magenta)->enable(true);
			fc->channel(yellow)->enable(true);
		}
	}
	else if (red != KChannelInvalid && green != KChannelInvalid &&
		 blue != KChannelInvalid)
	{
		color.setRgb(fc->channel(red)->sliderValue(),
			     fc->channel(green)->sliderValue(),
			     fc->channel(blue)->sliderValue(), 0);

		color = QColorDialog::getColor(color);
		if (color.isValid() == true)
		{
			fc->channel(red)->setValue(color.red());
			fc->channel(green)->setValue(color.green());
			fc->channel(blue)->setValue(color.blue());

			fc->channel(red)->enable(true);
			fc->channel(green)->enable(true);
			fc->channel(blue)->enable(true);
		}
	}
}

bool SceneEditor::isColorToolAvailable()
{
	FixtureConsole* fc;
	Fixture* fxi;
	QColor color;
	t_channel cyan, magenta, yellow;
	t_channel red, green, blue;

	/* QObject cast fails unless the widget is a FixtureConsole */
	fc = qobject_cast<FixtureConsole*> (m_tab->currentWidget());
	if (fc == NULL)
		return false;

	fxi = _app->doc()->fixture(fc->fixture());
	Q_ASSERT(fxi != NULL);

	cyan = fxi->channel("cyan", Qt::CaseInsensitive, KQLCChannelGroupColour);
	magenta = fxi->channel("magenta", Qt::CaseInsensitive, KQLCChannelGroupColour);
	yellow = fxi->channel("yellow", Qt::CaseInsensitive, KQLCChannelGroupColour);
	red = fxi->channel("red", Qt::CaseInsensitive, KQLCChannelGroupColour);
	green = fxi->channel("green", Qt::CaseInsensitive, KQLCChannelGroupColour);
	blue = fxi->channel("blue", Qt::CaseInsensitive, KQLCChannelGroupColour);

	if (cyan != KChannelInvalid && magenta != KChannelInvalid &&
	    yellow != KChannelInvalid)
	{
		return true;
	}
	else if (red != KChannelInvalid && green != KChannelInvalid &&
		 blue != KChannelInvalid)
	{
		return true;
	}
	else
	{
		return false;
	}
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

QList <Fixture*> SceneEditor::selectedFixtures() const
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

/*****************************************************************************
 * Fixture tabs
 *****************************************************************************/

FixtureConsole* SceneEditor::fixtureConsole(Fixture* fixture)
{
	Q_ASSERT(fixture != NULL);

	/* Start from the first fixture tab */
	for (int i = KTabFirstFixture; i < m_tab->count(); i++)
	{
		FixtureConsole* console;
		console = qobject_cast<FixtureConsole*> (m_tab->widget(i));
		if (console != NULL && console->fixture() == fixture->id())
			return console;
	}

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

	/* Start searching from the first fixture tab */
	for (int i = KTabFirstFixture; i < m_tab->count(); i++)
	{
		FixtureConsole* console;
		console = qobject_cast<FixtureConsole*> (m_tab->widget(i));
		if (console != NULL && console->fixture() == fixture->id())
		{
			/* First remove the tab because otherwise Qt might
			   remove two tabs -- undocumented feature, which
			   might be intended or it might not. */
			m_tab->removeTab(i);
			delete console;
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
