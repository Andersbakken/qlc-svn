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

#include <QTreeWidgetItem>
#include <QMdiSubWindow>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QScrollArea>
#include <QMessageBox>
#include <QTabWidget>
#include <QSplitter>
#include <QMdiArea>
#include <QToolBar>
#include <QAction>
#include <QString>
#include <QDebug>
#include <QIcon>
#include <QMenu>
#include <QtXml>

#include "qlcfixturemode.h"
#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcchannel.h"
#include "qlcfile.h"

#include "fixtureconsole.h"
#include "fixturemanager.h"
#include "universearray.h"
#include "outputpatch.h"
#include "addfixture.h"
#include "collection.h"
#include "outputmap.h"
#include "fixture.h"
#include "apputil.h"
#include "app.h"
#include "doc.h"

extern App* _app;

#define SETTINGS_GEOMETRY "fixturemanager/geometry"
#define SETTINGS_SPLITTER "fixturemanager/splitterstate"

// List view column numbers
#define KColumnUniverse 0
#define KColumnAddress  1
#define KColumnName     2
#define KColumnID       3

// Tab widget tabs
#define KTabInformation 0
#define KTabConsole     1

// Default window size
#define KDefaultWidth  600
#define KDefaultHeight 300

FixtureManager* FixtureManager::s_instance = NULL;


/*****************************************************************************
 * Initialization
 *****************************************************************************/

FixtureManager::FixtureManager(QWidget* parent, Qt::WindowFlags flags)
        : QWidget(parent, flags)
{
    new QVBoxLayout(this);

    m_console = NULL;

    initActions();
    initToolBar();
    initDataView();
    updateView();

    /* Listen to document changes */
    connect(_app, SIGNAL(documentChanged(Doc*)),
            this, SLOT(slotDocumentChanged(Doc*)));
    /* Use the initial document object */
    slotDocumentChanged(_app->doc());
}

FixtureManager::~FixtureManager()
{
    QSettings settings;
    settings.setValue(SETTINGS_SPLITTER, m_splitter->saveState());
#ifdef __APPLE__
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
#else
    settings.setValue(SETTINGS_GEOMETRY, parentWidget()->saveGeometry());
#endif
    FixtureManager::s_instance = NULL;
}

void FixtureManager::create(QWidget* parent)
{
    QWidget* window;

    /* Must not create more than one instance */
    if (s_instance != NULL)
        return;

#ifdef __APPLE__
    /* Create a separate window for OSX */
    s_instance = new FixtureManager(parent, Qt::Window);
    window = s_instance;
#else
    /* Create an MDI window for X11 & Win32 */
    QMdiArea* area = qobject_cast<QMdiArea*> (_app->centralWidget());
    Q_ASSERT(area != NULL);
    s_instance = new FixtureManager(parent);
    window = area->addSubWindow(s_instance);
#endif

    /* Set some common properties for the window and show it */
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setWindowIcon(QIcon(":/fixture.png"));
    window->setWindowTitle(tr("Fixture Manager"));
    window->setContextMenuPolicy(Qt::CustomContextMenu);
    window->show();

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
    {
        window->restoreGeometry(var.toByteArray());
        AppUtil::ensureWidgetIsVisible(window);
    }
    else
    {
        /* Backwards compatibility */
        QVariant w = settings.value("fixturemanager/width");
        QVariant h = settings.value("fixturemanager/height");
        if (w.isValid() == true && h.isValid() == true)
            window->resize(w.toInt(), h.toInt());
        else
            window->resize(600, 400);
    }
}

/*****************************************************************************
 * Doc signal handlers
 *****************************************************************************/

void FixtureManager::slotDocumentChanged(Doc* doc)
{
    Q_ASSERT(doc != NULL);

    /* Connect fixture list change signals from the new document object */
    connect(doc, SIGNAL(fixtureAdded(t_fixture_id)),
            this, SLOT(slotFixtureAdded(t_fixture_id)));

    connect(doc, SIGNAL(fixtureRemoved(t_fixture_id)),
            this, SLOT(slotFixtureRemoved(t_fixture_id)));

    connect(doc, SIGNAL(modeChanged(Doc::Mode)),
            this, SLOT(slotModeChanged(Doc::Mode)));
}

void FixtureManager::slotFixtureAdded(t_fixture_id id)
{
    Fixture* fxi = _app->doc()->fixture(id);
    if (fxi != NULL)
    {
        // Create a new list view item and fill it with fixture info
        QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
        updateItem(item, fxi);
    }
}

void FixtureManager::slotFixtureRemoved(t_fixture_id id)
{
    // Find a matching fixture item and destroy it
    QTreeWidgetItem* item = fixtureItem(id);
    if (item != NULL)
        delete item;
}

void FixtureManager::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Design)
    {
        m_addAction->setEnabled(true);
        if (m_tree->selectedItems().isEmpty() == false)
        {
            m_removeAction->setEnabled(true);
            m_propertiesAction->setEnabled(true);
        }
    }
    else
    {
        m_addAction->setEnabled(false);
        m_removeAction->setEnabled(false);
        m_propertiesAction->setEnabled(false);
    }
}

/*****************************************************************************
 * Data view
 *****************************************************************************/

void FixtureManager::initDataView()
{
    // Create a splitter to divide list view and text view
    m_splitter = new QSplitter(Qt::Horizontal, this);
    layout()->addWidget(m_splitter);
    m_splitter->setSizePolicy(QSizePolicy::Expanding,
                              QSizePolicy::Expanding);

    /* Create a tree widget to the left part of the splitter */
    m_tree = new QTreeWidget(this);
    m_splitter->addWidget(m_tree);

    QStringList labels;
    labels << tr("Universe") << tr("Address") << tr("Name");
    m_tree->setHeaderLabels(labels);
    m_tree->setRootIsDecorated(false);
    m_tree->setSortingEnabled(true);
    m_tree->setAllColumnsShowFocus(true);
    m_tree->sortByColumn(KColumnAddress, Qt::AscendingOrder);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tree->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(m_tree, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotSelectionChanged()));

    connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(slotDoubleClicked(QTreeWidgetItem*)));

    connect(m_tree, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(slotContextMenuRequested(const QPoint&)));

    /* Create a tab widget to the right part of the splitter */
    m_tab = new QTabWidget(this);
    m_splitter->addWidget(m_tab);

    /* Create the text view */
    m_info = new QTextBrowser(this);
    m_tab->addTab(m_info, tr("Information"));

    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 0);

    QSettings settings;
    QVariant var = settings.value(SETTINGS_SPLITTER);
    if (var.isValid() == true)
        m_splitter->restoreState(var.toByteArray());

    slotSelectionChanged();
}

void FixtureManager::updateView()
{
    QTreeWidgetItem* item;
    t_fixture_id currentId = Fixture::invalidId();
    t_fixture_id id = Fixture::invalidId();
    Fixture* fxt = NULL;

    // Store the currently selected fixture's ID
    item = m_tree->currentItem();
    if (item != NULL)
        currentId = item->text(KColumnID).toInt();

    // Clear the view
    m_tree->clear();

    // Add all fixtures
    for (id = 0; id < KFixtureArraySize; id++)
    {
        fxt = _app->doc()->fixture(id);
        if (fxt == NULL)
            continue;

        item = new QTreeWidgetItem(m_tree);

        // Update fixture information to the item
        updateItem(item, fxt);

        // Select this if it was selected before update
        if (currentId == id)
            m_tree->setCurrentItem(item);
    }

    /* Select the first fixture unless something else is wanted */
    if (currentId == Fixture::invalidId())
        m_tree->setCurrentItem(m_tree->topLevelItem(0));
}

QTreeWidgetItem* FixtureManager::fixtureItem(t_fixture_id id) const
{
    QTreeWidgetItemIterator it(m_tree);
    while (*it != NULL)
    {
        if ((*it)->text(KColumnID).toInt() == id)
            return (*it);
        ++it;
    }
    return NULL;
}

void FixtureManager::updateItem(QTreeWidgetItem* item, Fixture* fxi)
{
    QString s;

    Q_ASSERT(item != NULL);
    Q_ASSERT(fxi != NULL);

    // Universe column
    item->setText(KColumnUniverse, QString("%1").arg(fxi->universe() + 1));

    // Address column, show 0-based or 1-based DMX addresses
    OutputPatch* op = _app->outputMap()->patch(fxi->universe());
    if (op != NULL && op->isDMXZeroBased() == true)
        s.sprintf("%.3d - %.3d", fxi->address(),
                  fxi->address() + fxi->channels() - 1);
    else
        s.sprintf("%.3d - %.3d", fxi->address() + 1,
                  fxi->address() + fxi->channels());

    item->setText(KColumnAddress, s);

    // Name column
    item->setText(KColumnName, fxi->name());

    // ID column
    item->setText(KColumnID, QString("%1").arg(fxi->id()));
}

void FixtureManager::slotSelectionChanged()
{
    int selectedCount = m_tree->selectedItems().size();
    if (selectedCount == 1)
    {
        QScrollArea* scrollArea;
        t_fixture_id id;
        Fixture* fxi;
        int page;

        QTreeWidgetItem* item = m_tree->selectedItems().first();
        Q_ASSERT(item != NULL);

        // Set the text view's contents
        id = item->text(KColumnID).toInt();
        fxi = _app->doc()->fixture(id);
        Q_ASSERT(fxi != NULL);
        m_info->setText(QString("%1<BODY>%2</BODY></HTML>")
                        .arg(fixtureInfoStyleSheetHeader())
                        .arg(fxi->status()));

        /* Mark the current tab widget page */
        page = m_tab->currentIndex();

        /* Delete existing scroll area and console */
        delete m_console;
        delete m_tab->widget(KTabConsole);

        /* Create a new console for the selected fixture */
        m_console = new FixtureConsole(this);
        m_console->setFixture(id);
        m_console->setChannelsCheckable(false);

        /* Put the console inside a scroll area */
        scrollArea = new QScrollArea(this);
        scrollArea->setWidget(m_console);
        scrollArea->setWidgetResizable(true);
        m_tab->addTab(scrollArea, tr("Console"));

        /* Recall the same tab widget page */
        m_tab->setCurrentIndex(page);

        // Enable/disable actions
        if (_app->doc()->mode() == Doc::Design)
        {
            m_addAction->setEnabled(true);
            m_removeAction->setEnabled(true);
            m_propertiesAction->setEnabled(true);
        }
        else
        {
            m_addAction->setEnabled(false);
            m_removeAction->setEnabled(false);
            m_propertiesAction->setEnabled(false);
        }
    }
    else
    {
        // More than one or less than one selected

        QString info;

        // Add is not available in operate mode
        if (_app->doc()->mode() == Doc::Design)
            m_addAction->setEnabled(true);
        else
            m_addAction->setEnabled(false);

        // Disable properties action because its target is ambiguous
        m_propertiesAction->setEnabled(false);

        if (selectedCount > 1)
        {
            // Enable removal of multiple items in design mode
            if (_app->doc()->mode() == Doc::Design)
            {
                m_removeAction->setEnabled(true);
                info = tr("<HTML><BODY><H1>Multiple fixtures selected</H1>" \
                          "<P>Click <IMG SRC=\"" ":/edit_remove.png\">" \
                          " to remove the selected fixtures.</P></BODY></HTML>");
            }
            else
            {
                m_removeAction->setEnabled(false);
                info = tr("<HTML><BODY><H1>Multiple fixtures selected</H1>" \
                          "<P>Fixture list modification is not permitted" \
                          " in operate mode.</P></BODY></HTML>");
            }
        }
        else
        {
            // Disable remove since nothing is selected
            m_removeAction->setEnabled(false);
            if (m_tree->topLevelItemCount() <= 0)
            {
                info = tr("<HTML><BODY><H1>No fixtures</H1>" \
                          "<P>Click <IMG SRC=\"" ":/edit_add.png\">" \
                          " to add fixtures.</P></BODY></HTML>");
            }
            else
            {
                info = tr("<HTML><BODY><H1>Nothing selected</H1>" \
                          "<P>Select a fixture from the list or " \
                          "click <IMG SRC=\"" ":/edit_add.png\">" \
                          " to add fixtures.</P></BODY></HTML>");
            }
        }

        m_info->setText(info);

        delete m_console;
        m_console = NULL;
        m_tab->removeTab(KTabConsole);
    }
}

void FixtureManager::slotDoubleClicked(QTreeWidgetItem* item)
{
    if (item != NULL && _app->doc()->mode() != Doc::Operate)
        slotProperties();
}

QString FixtureManager::fixtureInfoStyleSheetHeader()
{
    QString info;

    QPalette pal;
    QColor hlBack(pal.color(QPalette::Highlight));
    QColor hlText(pal.color(QPalette::HighlightedText));

    info += "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">";
    info += "<HTML><HEAD></HEAD><STYLE>";
    info += QString(".hilite {" \
                    "	background-color: %1;" \
                    "	color: %2;" \
                    "	font-size: x-large;" \
                    "}").arg(hlBack.name()).arg(hlText.name());
    info += QString(".subhi {" \
                    "	background-color: %1;" \
                    "	color: %2;" \
                    "	font-weight: bold;" \
                    "}").arg(hlBack.name()).arg(hlText.name());
    info += QString(".emphasis {" \
                    "	font-weight: bold;" \
                    "}");
    info += "</STYLE>";
    return info;
}

/*****************************************************************************
 * Menu, toolbar and actions
 *****************************************************************************/

void FixtureManager::initActions()
{
    m_addAction = new QAction(QIcon(":/edit_add.png"),
                              tr("Add fixture..."), this);
    connect(m_addAction, SIGNAL(triggered(bool)),
            this, SLOT(slotAdd()));

    m_removeAction = new QAction(QIcon(":/edit_remove.png"),
                                 tr("Delete fixture"), this);
    connect(m_removeAction, SIGNAL(triggered(bool)),
            this, SLOT(slotRemove()));

    m_propertiesAction = new QAction(QIcon(":/configure.png"),
                                     tr("Configure fixture..."), this);
    connect(m_propertiesAction, SIGNAL(triggered(bool)),
            this, SLOT(slotProperties()));
}

void FixtureManager::initToolBar()
{
    QToolBar* toolbar = new QToolBar(tr("Fixture manager"), this);
    toolbar->setFloatable(false);
    toolbar->setMovable(false);
    layout()->setMenuBar(toolbar);
    toolbar->addAction(m_addAction);
    toolbar->addAction(m_removeAction);
    toolbar->addSeparator();
    toolbar->addAction(m_propertiesAction);
}

void FixtureManager::slotAdd()
{
    AddFixture af(this, _app->fixtureDefCache(), *_app->doc(),
                  *_app->outputMap());
    if (af.exec() == QDialog::Rejected)
        return;

    t_fixture_id latestFxi = Fixture::invalidId();

    QString name = af.name();
    quint32 address = af.address();
    quint32 universe = af.universe();
    quint32 channels = af.channels();
    int gap = af.gap();

    const QLCFixtureDef* fixtureDef = af.fixtureDef();
    const QLCFixtureMode* mode = af.mode();

    QString modname;

    /* If an empty name was given use the model instead */
    if (name.simplified().isEmpty())
    {
        if (fixtureDef != NULL)
            name = fixtureDef->model();
        else
            name = tr("Generic Dimmer");
    }

    /* If we're adding more than one fixture,
       append a number to the end of the name */
    if (af.amount() > 1)
        modname = QString("%1 #1").arg(name);
    else
        modname = name;

    /* Create the fixture */
    Fixture* fxi = new Fixture(_app->doc());

    /* Add the first fixture without gap, at the given address */
    fxi->setAddress(address);
    fxi->setUniverse(universe);
    fxi->setName(modname);

    /* Set a fixture definition & mode if they were selected.
       Otherwise assign channels to a generic dimmer. */
    if (fixtureDef != NULL && mode != NULL)
        fxi->setFixtureDefinition(fixtureDef, mode);
    else
        fxi->setChannels(channels);

    /* Attempt to add the fixture to doc. If the first one fails,
       it is very likely that others would, too. */
    if (_app->doc()->addFixture(fxi) == false)
    {
        /* Adding failed. Display error and bail out. */
        addFixtureErrorMessage();
        delete fxi;
        fxi = NULL;
        return;
    }
    else
    {
        latestFxi = fxi->id();
    }

    /* Add the rest (if any) WITH address gap */
    for (int i = 1; i < af.amount(); i++)
    {
        /* If we're adding more than one fixture,
           append a number to the end of the name */
        if (af.amount() > 1)
            modname = QString("%1 #%2").arg(name).arg(i +1);
        else
            modname = name;

        /* Create the fixture */
        Fixture* fxi = new Fixture(_app->doc());

        /* Assign the next address AFTER the previous fixture
           address space plus gap. */
        fxi->setAddress(address + (i * channels) + gap);
        fxi->setUniverse(universe);
        fxi->setName(modname);
        /* Set a fixture definition & mode if they were
           selected. Otherwise assign channels to a generic
           dimmer. */
        if (fixtureDef != NULL && mode != NULL)
            fxi->setFixtureDefinition(fixtureDef, mode);
        else
            fxi->setChannels(channels);

        /* Attempt to add the fixture to doc. If one fails,
        it is very likely that others would, too. */
        if (_app->doc()->addFixture(fxi) == false)
        {
            /* Adding failed. Display error and bail out. */
            addFixtureErrorMessage();
            delete fxi;
            fxi = NULL;
            break;
        }
        else
        {
            latestFxi = fxi->id();
        }
    }

    QTreeWidgetItem* selectItem = fixtureItem(latestFxi);
    if (selectItem != NULL)
        m_tree->setCurrentItem(selectItem);
}

void FixtureManager::addFixtureErrorMessage()
{
    if (_app->doc()->fixtures() >= KFixtureArraySize)
    {
        QMessageBox::critical(this, tr("Too many fixtures"),
                              tr("You can't create more than %1 fixtures.")
                              .arg(KFixtureArraySize));
    }
    else
    {
        QMessageBox::critical(this, tr("Fixture creation failed"),
                              tr("Unable to create new fixture."));
    }
}

void FixtureManager::slotRemove()
{
    // Ask before deletion
    if (QMessageBox::question(this, tr("Delete Fixtures"),
                              tr("Do you want to DELETE the selected fixtures?"),
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
    {
        return;
    }

    QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item(it.next());
        Q_ASSERT(item != NULL);

        t_fixture_id id = item->text(KColumnID).toInt();

        /** @todo This is REALLY bogus here, since Fixture or Doc should do
            this. However, FixtureManager is the only place to destroy fixtures,
            so it's rather safe to reset the fixture's address space here. */
        Fixture* fxi = _app->doc()->fixture(id);
        Q_ASSERT(fxi != NULL);
        UniverseArray* ua = _app->outputMap()->claimUniverses();
        ua->reset(fxi->address(), fxi->channels());
        _app->outputMap()->releaseUniverses();

        _app->doc()->deleteFixture(id);
    }
}

void FixtureManager::slotProperties()
{
    QTreeWidgetItem* item = m_tree->currentItem();
    if (item == NULL)
        return;

    t_fixture_id id = item->text(KColumnID).toInt();
    Fixture* fxi = _app->doc()->fixture(id);
    if (fxi == NULL)
        return;

    QString manuf;
    QString model;
    QString mode;

    if (fxi->fixtureDef() != NULL)
    {
        manuf = fxi->fixtureDef()->manufacturer();
        model = fxi->fixtureDef()->model();
        mode = fxi->fixtureMode()->name();
    }
    else
    {
        manuf = KXMLFixtureGeneric;
        model = KXMLFixtureGeneric;
    }

    AddFixture af(this, _app->fixtureDefCache(), *(_app->doc()),
                  *(_app->outputMap()), manuf, model, mode,
                  fxi->name(), fxi->universe(), fxi->address(),
                  fxi->channels());
    af.setWindowTitle(tr("Change fixture properties"));
    if (af.exec() == QDialog::Accepted)
    {
        if (fxi->name() != af.name())
            fxi->setName(af.name());
        if (fxi->universe() != af.universe())
            fxi->setUniverse(af.universe());
        if (fxi->address() != af.address())
            fxi->setAddress(af.address());

        if (af.fixtureDef() != NULL && af.mode() != NULL)
        {
            if (fxi->fixtureDef() != af.fixtureDef() ||
                    fxi->fixtureMode() != af.mode())
            {
                fxi->setFixtureDefinition(af.fixtureDef(),
                                          af.mode());
            }
        }
        else
        {
            /* Generic dimmer */
            fxi->setFixtureDefinition(NULL, NULL);
            fxi->setChannels(af.channels());
        }

        updateItem(item, fxi);
        slotSelectionChanged();
    }
}

void FixtureManager::slotContextMenuRequested(const QPoint&)
{
    QMenu menu(this);
    menu.addAction(m_addAction);
    menu.addAction(m_propertiesAction);
    menu.addSeparator();
    menu.addAction(m_removeAction);
    menu.exec(QCursor::pos());
}
