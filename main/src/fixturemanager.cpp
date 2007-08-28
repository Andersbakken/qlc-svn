/*
  Q Light Controller
  fixturemanager.cpp

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

#include <qwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qevent.h>
#include <qdockarea.h>
#include <qtextview.h>
#include <qsplitter.h>
#include <qlistview.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qheader.h>
#include <qpopupmenu.h>
#include <qtooltip.h>
#include <assert.h>

#include "common/qlcfixturedef.h"
#include "common/qlcfixturemode.h"
#include "common/qlccapability.h"
#include "common/qlcchannel.h"
#include "common/settings.h"
#include "common/logicalchannel.h"
#include "common/capability.h"
#include "common/filehandler.h"

#include "fixturemanager.h"
#include "fixtureproperties.h"
#include "app.h"
#include "doc.h"
#include "configkeys.h"
#include "addfixture.h"
#include "consolechannel.h"
#include "fixture.h"

#include "scene.h"
#include "chaser.h"
#include "functioncollection.h"
#include "efx.h"

extern App* _app;

// List view column numbers
const int KColumnUniverse ( 0 );
const int KColumnAddress  ( 1 );
const int KColumnName     ( 2 );
const int KColumnID       ( 3 );

// List view item menu callback id's
const int KMenuItemAdd          ( 0 );
const int KMenuItemRemove       ( 1 );
const int KMenuItemProperties   ( 2 );
const int KMenuItemConsole      ( 3 );
const int KMenuItemClone        ( 4 );
const int KMenuItemAutoFunction ( 5 );

const int KDefaultWidth  ( 600 );
const int KDefaultHeight ( 300 );

FixtureManager::FixtureManager(QWidget* parent)
	: QWidget(parent, "Fixture Manager"),

	  m_layout   ( NULL ),
	  m_dockArea ( NULL ),
	  m_toolbar  ( NULL ),
	  m_splitter ( NULL ),
	  m_listView ( NULL ),
	  m_textView ( NULL )
{
}

FixtureManager::~FixtureManager()
{
	QString config;

	if (isShown())
	{
		config = Settings::trueValue();
	}
	else
	{
		config = Settings::falseValue();
	}

	_app->settings()->set(KEY_FIXTURE_MANAGER_OPEN, config);

	//
	// Save rect
	//
	_app->settings()->set(KEY_FIXTURE_MANAGER_X, rect().x());
	_app->settings()->set(KEY_FIXTURE_MANAGER_Y, rect().y());
	_app->settings()->set(KEY_FIXTURE_MANAGER_W, rect().width());
	_app->settings()->set(KEY_FIXTURE_MANAGER_H, rect().height());

	// Save the splitter position
	config.truncate(0);
	QValueList<int> list = m_splitter->sizes();
	QValueList<int>::Iterator it = list.begin();
	_app->settings()->set(KEY_FIXTURE_MANAGER_SPLITTER, *it);
}

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void FixtureManager::initView()
{
	// Create a vertical layout to this widget
	m_layout = new QVBoxLayout(this);

	// Init the title and icon
	initTitle();

	// Set up toolbar
	initToolBar();

	// Init the fixture view and text view
	initDataView();

	// Update the list of fixtures
	updateView();

	//
	// Set widget proportions
	//
	QString x, y, w, h;

	if (_app->settings()->get(KEY_FIXTURE_MANAGER_X, x) == -1
	    || x.toInt() <= 0 || x.toInt() >= _app->width())
	{
		x.setNum(0);
	}

	if (_app->settings()->get(KEY_FIXTURE_MANAGER_Y, y) == -1
	    || y.toInt() <= 0 || y.toInt() >= _app->height())
	{
		y.setNum(0);
	}

	if (_app->settings()->get(KEY_FIXTURE_MANAGER_W, w) == -1
	    || w.toInt() <= 0 || w.toInt() >= _app->width())
	{
		w.setNum(KDefaultWidth);
	}

	if (_app->settings()->get(KEY_FIXTURE_MANAGER_H, h) == -1
	    || h.toInt() <= 0 || w.toInt() >= _app->height())
	{
		h.setNum(KDefaultHeight);
	}

	setGeometry(x.toInt(), y.toInt(), w.toInt(), h.toInt());

	// Set the splitter position
	QValueList<int> list;
	if (_app->settings()->get(KEY_FIXTURE_MANAGER_SPLITTER, w) != -1
	    && w.toInt() >= 0)
	{
		list.append(w.toInt());
		list.append(width() - w.toInt());
	}
	else
	{
		list.append(width() / 2);
		list.append(width() / 2);
	}

	m_splitter->setSizes(list);
}

void FixtureManager::initTitle()
{
	// Set the name
	setCaption(QString("Fixture Manager"));

	// Set an icon
	setIcon(QString(PIXMAPS) + QString("/fixture.png"));
}

void FixtureManager::initToolBar()
{
	// Create a dock area for the toolbar
	m_dockArea = new QDockArea(Horizontal, QDockArea::Normal, this);
	m_dockArea->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

	// Add the dock area to the top of the vertical layout
	m_layout->addWidget(m_dockArea);

	//
	// Add a toolbar to the dock area
	//
	m_toolbar = new QToolBar("Fixture Manager", _app, m_dockArea);
	m_toolbar->setMovingEnabled(false);

	m_addButton = new QToolButton(
		QIconSet(QPixmap(QString(PIXMAPS) + QString("/wizard.png"))),
		"Add", 0, this,
		SLOT(slotAdd()), m_toolbar);
	m_addButton->setUsesTextLabel(true);
	QToolTip::add(m_addButton,  "Add fixture");

	m_cloneButton = new QToolButton(
		QIconSet(QPixmap(QString(PIXMAPS) + QString("/editcopy.png"))),
		"Clone", 0, this,
		SLOT(slotClone()), m_toolbar);
	m_cloneButton->setUsesTextLabel(true);
	QToolTip::add(m_cloneButton, "Clone a fixture and its functions" );

	m_removeButton = new QToolButton(
		QIconSet(QPixmap(QString(PIXMAPS) + QString("/editdelete.png"))),
		"Remove", 0, this,
		SLOT(slotRemove()), m_toolbar);
	m_removeButton->setUsesTextLabel(true);
	QToolTip::add(m_removeButton, "Remove fixture");

	m_toolbar->addSeparator();

	m_propertiesButton = new QToolButton(
		QIconSet(QPixmap(QString(PIXMAPS) + QString("/configure.png"))),
		"Properties", 0, this,
		SLOT(slotProperties()), m_toolbar);
	m_propertiesButton->setUsesTextLabel(true);
	QToolTip::add(m_propertiesButton, "Fixture properties");

	m_consoleButton = new QToolButton(
		QIconSet(QPixmap(QString(PIXMAPS) + QString("/console.png"))),
		"Console", 0, this,
		SLOT(slotConsole()), m_toolbar);
	m_consoleButton->setUsesTextLabel(true);
	QToolTip::add(m_consoleButton, "View fixture console");
}

void FixtureManager::initDataView()
{
	// Create a splitter to divide list view and text view
	m_splitter = new QSplitter(this);
	m_splitter->setSizePolicy(QSizePolicy::Expanding,
				  QSizePolicy::Expanding);
	m_layout->addWidget(m_splitter);
	
	// Create the list view
	m_listView = new QListView(m_splitter);
	m_splitter->setResizeMode(m_listView, QSplitter::Auto);
	
	m_listView->setMultiSelection(false);
	m_listView->setAllColumnsShowFocus(true);
	m_listView->setSorting(KColumnAddress, true);
	m_listView->setShowSortIndicator(true);
	m_listView->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	
	m_listView->header()->setClickEnabled(true);
	m_listView->header()->setResizeEnabled(true);
	m_listView->header()->setMovingEnabled(false);
	
	m_listView->addColumn("Universe");
	m_listView->addColumn("Address");
	m_listView->addColumn("Name");
	m_listView->setResizeMode(QListView::LastColumn);
	
	connect(m_listView, SIGNAL(selectionChanged(QListViewItem*)),
		this, SLOT(slotSelectionChanged(QListViewItem*)));
	
	connect(m_listView, SIGNAL(doubleClicked(QListViewItem*)),
		this, SLOT(slotDoubleClicked(QListViewItem*)));
	
	connect(m_listView, SIGNAL(rightButtonClicked(QListViewItem*,
						      const QPoint&, int)),
		this, SLOT(slotRightButtonClicked(QListViewItem*,
						  const QPoint&, int)));

	// Create the text view
	m_textView = new QTextView(m_splitter);
	m_splitter->setResizeMode(m_textView, QSplitter::Auto);

	slotSelectionChanged(NULL);
}

void FixtureManager::updateView()
{
	t_fixture_id currentId = KNoID;
	t_fixture_id id = KNoID;
	QListViewItem* item = NULL;
	Fixture* fxt = NULL;

	// Store the currently selected fixture's ID
	if (m_listView->currentItem() != NULL)
	{
		currentId = m_listView->currentItem()->text(KColumnID).toInt();
	}

	// Clear the view
	m_listView->clear();

	// Add all fixtures
	for (id = 0; id < KFixtureArraySize; id++)
	{
		fxt = _app->doc()->fixture(id);
		if (fxt == NULL)
		{
			continue;
		}
		else
		{
			item = new QListViewItem(m_listView);
			
			// Update fixture information to the item
			updateItem(item, fxt);

			// Select this if it was selected before update
			if (currentId == id)
			{
				m_listView->setSelected(item, true);
			}
		}
	}

	slotSelectionChanged(m_listView->currentItem());
}

void FixtureManager::updateItem(QListViewItem* item, Fixture* fxi)
{
	QString s;
	
	Q_ASSERT(item != NULL);
	Q_ASSERT(fxi != NULL);
	
	// Universe column
	s.sprintf("%d", fxi->universe() + 1);
	item->setText(KColumnUniverse, s);

	// Address column
	s.sprintf("%.3d - %.3d", fxi->address() + 1,
		  fxi->address() + fxi->channels());
	item->setText(KColumnAddress, s);
	
	// Name column
	item->setText(KColumnName, fxi->name());

	// ID column
	s.setNum(fxi->id());
	item->setText(KColumnID, s);
}

void FixtureManager::copyFunction(Function* function, Fixture* fxi)
{
	switch(function->type())
	{
	case Function::Scene:
	{
		Scene* scene = static_cast<Scene*>
			(_app->doc()->newFunction(Function::Scene, 
						  fxi->id()));

		scene->copyFrom(static_cast<Scene*> (function), fxi->id());
	}
	break;

	case Function::Chaser:
	{
		Chaser* chaser = static_cast<Chaser*>
			(_app->doc()->newFunction(Function::Chaser, KNoID));

		chaser->copyFrom(static_cast<Chaser*> (function));
	}
	break;

	case Function::Collection:
	{
		FunctionCollection* fc = static_cast<FunctionCollection*>
			(_app->doc()->newFunction(Function::Collection, KNoID));

		fc->copyFrom(static_cast<FunctionCollection*> (function));
	}
	break;

	case Function::EFX:
	{
		EFX* efx = static_cast<EFX*>
			(_app->doc()->newFunction(Function::EFX, KNoID));

		efx->copyFrom(static_cast<EFX*> (function), fxi->id());
	}
	break;

	default:
		break;
	}
}

/*****************************************************************************
 * Load & save
 *****************************************************************************/

void FixtureManager::loader(QDomDocument* doc, QDomElement* root)
{
	_app->createFixtureManager();
	_app->fixtureManager()->loadXML(doc, root);
}

bool FixtureManager::loadXML(QDomDocument* doc, QDomElement* root)
{
	bool visible = false;
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
	
	QDomNode node;
	QDomElement tag;
	
	Q_ASSERT(doc != NULL);
	Q_ASSERT(root != NULL);
	
	if (root->tagName() != KXMLQLCFixtureManager)
	{
		qWarning("Fixture Manager node not found!");
		return false;
	}

	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCWindowState)
		{
			FileHandler::loadXMLWindowState(&tag, &x, &y, &w, &h,
							&visible);
		}
		else
		{
			qDebug("Unknown fixture manager tag: %s",
			       (const char*) tag.tagName());
		}
		
		node = node.nextSibling();
	}

	hide();
	setGeometry(x, y, w, h);
	if (visible == false)
		showMinimized();
	else
		showNormal();

	return true;
}

bool FixtureManager::saveXML(QDomDocument* doc, QDomElement* fxi_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(fxi_root != NULL);

	/* Fixture Manager entry */
	root = doc->createElement(KXMLQLCFixtureManager);
	fxi_root->appendChild(root);

	/* Save window state. parentWidget() should be used for all
	   widgets within the workspace. */
	return FileHandler::saveXMLWindowState(doc, &root, parentWidget());
}

/*****************************************************************************
 * Public slots
 *****************************************************************************/

void FixtureManager::slotFixtureAdded(t_fixture_id id)
{
	Fixture* fxi = NULL;
	QListViewItem* item = NULL;

	fxi = _app->doc()->fixture(id);
	if (fxi != NULL)
	{
		// Create a new list view item
		item = new QListViewItem(m_listView);

		// Fill fixture information to the item
		updateItem(item, fxi);
	}
}

void FixtureManager::slotFixtureRemoved(t_fixture_id id)
{
	QListViewItemIterator it(m_listView);
	QListViewItem* item = NULL;
	QListViewItem* nextItem = NULL;

	while ((item = it.current()) != NULL)
	{
		if (item->text(KColumnID).toInt() == id)
		{
			if (item->isSelected())
			{
				// Try to select the closest neighbour
				if (item->itemAbove())
					nextItem = item->itemAbove();
				else
					nextItem = item->itemBelow();

				// Select the neighbour
				m_listView->setSelected(nextItem, true);
				slotSelectionChanged(nextItem);
			}

			delete item;
		}

		++it;
	}
}

void FixtureManager::slotModeChanged()
{
	slotSelectionChanged(m_listView->currentItem());
}

/*****************************************************************************
 * Protected slots
 *****************************************************************************/

void FixtureManager::slotAdd()
{
	AddFixture* af = NULL;
	int i = 0;
	
	af = new AddFixture(_app);
	af->init();
	
	if (af->exec() == QDialog::Accepted)
	{
		QString name = af->name();
		t_channel address = af->address();
		t_channel universe = af->universe();
		int gap = af->addressGap();
		t_channel channels = af->channels();

		QLCFixtureDef* fixtureDef = af->fixtureDef();
		QLCFixtureMode* mode = af->mode();
		
		if (fixtureDef != NULL && mode != NULL)
		{
			/* Add a normal fixture with an existing definition */

			/* If an empty name was given use the model name instead */
			if (name.stripWhiteSpace() == QString::null)
				name = fixtureDef->model();
			
			// Add the first fixture without gap
			_app->doc()->newFixture(fixtureDef, mode, address,
						universe, name);
			
			// Add the rest (if any) with address gap
			for (i = 1; i < af->multipleNumber(); i++)
			{
				_app->doc()->newFixture(
					fixtureDef, mode, 
					address + (i * channels) + gap,
					universe, name);
			}
		}
		else
		{
			/* Add a generic fixture without definition */

			/* If an empty name was given use Generic instead */
			if (name.stripWhiteSpace() == QString::null)
				name = KXMLFixtureGeneric;

			// Add the first fixture without gap
			_app->doc()->newGenericFixture(address, universe,
						       channels, name);

			// Add the rest (if any) with address gap
			for (i = 1; i < af->multipleNumber(); i++)
			{
				_app->doc()->newGenericFixture(
					address + (i * channels) + gap,
					universe, channels, name);
			}
		}
	}

	delete af;
}

void FixtureManager::slotRemove()
{
	QListViewItem* item = m_listView->currentItem();

	// Get the fixture id
	t_fixture_id id = item->text(KColumnID).toInt();

	// Display a warning
	QString msg;
	msg = ("Do you want to remove \"");
	msg += item->text(KColumnName) + QString("\"?");
	if (QMessageBox::warning(this, KApplicationNameShort, msg,
				 QMessageBox::Yes, QMessageBox::No)
	    == QMessageBox::No)
	{
		return;
	}
	else
	{
		_app->doc()->deleteFixture(id);
	}
}

void FixtureManager::slotClone()
{
	QListViewItem* item = NULL;
	QLCFixtureMode* fixtureMode = NULL;
	Fixture* old_fxi = NULL;
	Fixture* new_fxi = NULL;
	QLCFixtureDef* fixtureDef = NULL;
	t_fixture_id old_id = 0;
	QString new_name;

	/* Get the selected listview item */
	item = m_listView->currentItem();
	Q_ASSERT(item != NULL);

	/* Get the old fixture instance */
	old_id = item->text(KColumnID).toInt();
	old_fxi = _app->doc()->fixture(old_id);
	Q_ASSERT(old_fxi != NULL);

	/* Get the old fixture instance's fixture definition */
	fixtureDef = old_fxi->fixtureDef();
	Q_ASSERT(fixtureDef != NULL);

	/* Get the old fixture instance's mode */
	fixtureMode = old_fxi->fixtureMode();
	Q_ASSERT(fixtureMode != NULL);

	new_name = "Copy of ";
	new_name += item->text(KColumnName);

	// Add new fixture
	new_fxi = _app->doc()->newFixture(fixtureDef, fixtureMode, 0, 0, new_name);
	if (new_fxi != NULL)
	{
		for (t_function_id id = 0; id < KFunctionArraySize; id++)
		{
			Function* function = _app->doc()->function(id);
			if (function == NULL)
			{
				continue;
			}
			
			// Copy only functions that belong to the parent fixture
			if (function->fixture() == old_id)
			{
				copyFunction(function, new_fxi);
			}
		}

		QString newid;
		newid.setNum(new_fxi->id());
		m_listView->setCurrentItem(m_listView->findItem(newid, KColumnID));
		slotProperties();
	}
}

void FixtureManager::slotProperties()
{
	QListViewItem* item = NULL;
	Fixture* fixture = NULL;
	FixtureProperties* prop = NULL;
	t_fixture_id id = 0;

	item = m_listView->currentItem();
	if (item != NULL)
	{
		id = item->text(KColumnID).toInt();
		fixture = _app->doc()->fixture(id);
		Q_ASSERT(fixture != NULL);
		
		// View properties dialog
		prop = new FixtureProperties(_app, id);
		prop->init();

		if (prop->exec() == QDialog::Accepted)
		{
			// Update changes to view
			updateItem(item, fixture);
		
			// Update changes to the info view
			slotSelectionChanged(item);
		}

		delete prop;
	}
}

void FixtureManager::slotConsole()
{
	QListViewItem* item = m_listView->currentItem();
	t_fixture_id id = item->text(KColumnID).toInt();

	Fixture* fxi = _app->doc()->fixture(id);
	Q_ASSERT(fxi != NULL);

	fxi->viewConsole();
}

//
// Autocreate functions
//
void FixtureManager::slotAutoFunction()
{
	Fixture* fxi = NULL;
	Scene* sc = NULL;
	QLCChannel* channel = NULL;
	QLCCapability* cap = NULL;
	QListViewItem* item = NULL;
	t_fixture_id fxi_id = KNoID;
	unsigned int i = 0;

	item = m_listView->currentItem();
	if (item == NULL)
		return;

	fxi_id = item->text(KColumnID).toInt();

	fxi = _app->doc()->fixture(fxi_id);
	Q_ASSERT(fxi != NULL);
  
	// Loop over all channels
	for (int i = 0; i < fxi->channels(); i++)
	{
		channel = fxi->channel(i);
		Q_ASSERT(channel != NULL);

		QPtrListIterator<QLCCapability> cap_it(*channel->capabilities());

		// Loop over all capabilities
		while ((cap = cap_it.current()) != NULL)
		{
			sc = static_cast<Scene*> 
				(_app->doc()->newFunction(Function::Scene, fxi_id));
	  
			sc->setName(channel->name() + " - " + cap->name());

			// Set the unused channels to NoSet and zero.
			for (int j = 0; j < fxi->channels(); j++)
			{
				sc->set(j, 0, Scene::NoSet);
			}

			// Set only the capability
			sc->set(i, (t_value) ((cap->min() + cap->max()) / 2),
				Scene::Set);

			++cap_it;
		}
	}

	fxi->slotConsoleClosed();
	fxi->viewConsole();
}

void FixtureManager::slotSelectionChanged(QListViewItem* item)
{
	Fixture* fxi = NULL;
	t_fixture_id id = 0;
	QString info;

	if (item == NULL)
	{
		// Disable all
		if (_app->mode() == App::Design)
		{
			m_addButton->setEnabled(true);
		}
		else
		{
			m_addButton->setEnabled(false);
		}

		m_removeButton->setEnabled(false);
		m_propertiesButton->setEnabled(false);
		m_consoleButton->setEnabled(false);
		m_cloneButton->setEnabled(false);

		info = QString("<HTML><BODY>");
		info += QString("<H1>No fixtures</H1>");
		info += QString("Click \"Add\" on the ");
		info += QString("toolbar to add a new fixture.");
		info += QString("</BODY></HTML>");

		m_textView->setText(info);
	}
	else
	{
		// Set the text view's contents
		id = item->text(KColumnID).toInt();
		fxi = _app->doc()->fixture(id);
		Q_ASSERT(fxi != NULL);

		info = fxi->status();
		m_textView->setText(info);

		// Enable console always
		m_consoleButton->setEnabled(true);

		if (_app->mode() == App::Design)
		{
			m_addButton->setEnabled(true);
			m_removeButton->setEnabled(true);
			m_propertiesButton->setEnabled(true);
			m_cloneButton->setEnabled(true);
		}
		else
		{
			m_addButton->setEnabled(false);
			m_removeButton->setEnabled(false);
			m_propertiesButton->setEnabled(false);
			m_cloneButton->setEnabled(false);
		}
	}
}

void FixtureManager::slotDoubleClicked(QListViewItem* item)
{
	if (item != NULL)
		slotProperties();
}

void FixtureManager::slotRightButtonClicked(QListViewItem* item,
					    const QPoint& point, int col)
{
	QPopupMenu* menu = new QPopupMenu();
	menu->setCheckable(false);

	menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/wizard.png")),
			 "Add...", KMenuItemAdd);
	menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editcopy.png")),
			 "Clone...", KMenuItemClone);
	menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editdelete.png")),
			 "Remove", KMenuItemRemove);
	menu->insertSeparator();
	menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/configure.png")),
			 "Properties...", KMenuItemProperties);
	menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/console.png")),
			 "View Console...", KMenuItemConsole);
	menu->insertSeparator();
	menu->insertItem( QPixmap(QString(PIXMAPS) + QString("/function.png")),
			  "Autocreate Functions", KMenuItemAutoFunction);

	if (_app->mode() == App::Operate)
	{
		// Operate mode, remove and edit impossible
		menu->setItemEnabled(KMenuItemAdd, false);
		menu->setItemEnabled(KMenuItemRemove, false);
		menu->setItemEnabled(KMenuItemProperties, false);
		menu->setItemEnabled(KMenuItemClone, false);
		menu->setItemEnabled(KMenuItemAutoFunction, false);
	}

	// No item selected, unable to do other things either
	if (!item)
	{
		menu->setItemEnabled(KMenuItemRemove, false);
		menu->setItemEnabled(KMenuItemConsole, false);
		menu->setItemEnabled(KMenuItemProperties, false);
		menu->setItemEnabled(KMenuItemClone, false);
		menu->setItemEnabled(KMenuItemAutoFunction, false);
	}

	connect(menu, SIGNAL(activated(int)), 
		this, SLOT(slotMenuCallBack(int)));
	menu->exec(point, 0);
	delete menu;
}

void FixtureManager::slotMenuCallBack(int item)
{
	switch (item)
	{
	case KMenuItemAdd:
		slotAdd();
		break;

	case KMenuItemRemove:
		slotRemove();
		break;

	case KMenuItemClone:
		slotClone();
		break;

	case KMenuItemProperties:
		slotProperties();
		break;

	case KMenuItemConsole:
		slotConsole();
		break;

	case KMenuItemAutoFunction:
		slotAutoFunction();
		break;
	default:
		break;
	}
}

void FixtureManager::closeEvent(QCloseEvent* e)
{
	e->accept();
	emit closed();
}


