/*
  Q Light Controller - Fixture Definition Editor
  qlcfixtureeditor.cpp

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

#include <QTreeWidgetItem>
#include <QInputDialog>
#include <QHeaderView>
#include <QTreeWidget>
#include <QMessageBox>
#include <QToolButton>
#include <QFileDialog>
#include <QCloseEvent>
#include <QSettings>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QTabWidget>
#include <QIcon>
#include <QMenu>
#include <QList>
#include <QUrl>

#include <common/qlcfixturemode.h>
#include <common/qlcfixturedef.h>
#include <common/qlccapability.h>
#include <common/qlcphysical.h>
#include <common/qlcchannel.h>
#include <common/qlctypes.h>
#include <common/qlcfile.h>

#ifdef Q_WS_X11
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#endif

#include "app.h"
#include "fixtureeditor.h"
#include "editcapability.h"
#include "editchannel.h"
#include "editmode.h"

extern App* _app;
extern int errno;

static const int KChannelsColumnName     ( 0 );
static const int KChannelsColumnGroup    ( 1 );
static const int KChannelsColumnPointer  ( 2 );

static const int KModesColumnName     ( 0 );
static const int KModesColumnChannels ( 1 );
static const int KModesColumnPointer  ( 2 );

static const int KMenuEdit ( 0 );
static const int KMenuCopy ( 1 );
static const int KMenuPaste ( 3 );
static const int KMenuRemove ( 4 );

QLCFixtureEditor::QLCFixtureEditor(QWidget* parent,
				   QLCFixtureDef* fixtureDef,
				   const QString& fileName) : QWidget(parent)
{
	m_fixtureDef = fixtureDef;
	m_fileName = fileName;
	m_modified = false;

	setupUi(this);
	init();
	setCaption();

	setModified(false);
}

QLCFixtureEditor::~QLCFixtureEditor()
{
	delete m_fixtureDef;
}

void QLCFixtureEditor::init()
{
	/* General page */
	m_manufacturerEdit->setText(m_fixtureDef->manufacturer());
	connect(m_manufacturerEdit, SIGNAL(textEdited(const QString&)),
		this, SLOT(slotManufacturerTextEdited(const QString&)));

	m_modelEdit->setText(m_fixtureDef->model());
	connect(m_modelEdit, SIGNAL(textEdited(const QString&)),
		this, SLOT(slotModelTextEdited(const QString&)));

	m_typeCombo->setCurrentIndex(
		m_typeCombo->findText(m_fixtureDef->type()));
	connect(m_typeCombo, SIGNAL(activated(const QString&)),
		this, SLOT(slotTypeActivated(const QString&)));

	/* Channel page */
	connect(m_addChannelButton, SIGNAL(clicked()),
		this, SLOT(slotAddChannel()));
	connect(m_removeChannelButton, SIGNAL(clicked()),
		this, SLOT(slotRemoveChannel()));
	connect(m_editChannelButton, SIGNAL(clicked()),
		this, SLOT(slotEditChannel()));

	connect(m_channelList, SIGNAL(currentItemChanged(QTreeWidgetItem*,
							 QTreeWidgetItem*)),
		this, SLOT(slotChannelListSelectionChanged(QTreeWidgetItem*)));
	connect(m_channelList,
		SIGNAL(customContextMenuRequested(const QPoint&)),
		this,
		SLOT(slotChannelListContextMenuRequested(const QPoint&)));
	connect(m_channelList,
		SIGNAL(itemActivated(QTreeWidgetItem*,int)),
		this,
		SLOT(slotEditChannel()));

	m_channelList->header()->setResizeMode(QHeaderView::ResizeToContents);
	refreshChannelList();

	/* Mode page */
	connect(m_addModeButton, SIGNAL(clicked()),
		this, SLOT(slotAddMode()));
	connect(m_removeModeButton, SIGNAL(clicked()),
		this, SLOT(slotRemoveMode()));
	connect(m_editModeButton, SIGNAL(clicked()),
		this, SLOT(slotEditMode()));

	connect(m_modeList, SIGNAL(currentItemChanged(QTreeWidgetItem*,
						      QTreeWidgetItem*)),
		this, SLOT(slotModeListSelectionChanged(QTreeWidgetItem*)));
	connect(m_modeList,
		SIGNAL(customContextMenuRequested(const QPoint&)),
		this,
		SLOT(slotModeListContextMenuRequested(const QPoint&)));
	connect(m_modeList,
		SIGNAL(itemActivated(QTreeWidgetItem*,int)),
		this,
		SLOT(slotEditMode()));

	m_modeList->header()->setResizeMode(QHeaderView::ResizeToContents);
	refreshModeList();
}

void QLCFixtureEditor::closeEvent(QCloseEvent* e)
{
	int r = 0;

	if (m_modified)
	{
		r = QMessageBox::information(this, tr("Close"),
				"Do you want to save changes to fixture\n\""
				+ m_fixtureDef->name() + "\"\nbefore closing?",
				QMessageBox::Yes,
				QMessageBox::No,
				QMessageBox::Cancel);

		if (r == QMessageBox::Yes)
			if (save())
				e->accept();
			else
				e->ignore();
		else if (r == QMessageBox::No)
			e->accept();
		else
			e->ignore();
	}
	else
	{
		e->accept();
	}
}

bool QLCFixtureEditor::checkManufacturerModel()
{
	/* Check that the fixture has a manufacturer and a model for
	   unique identification */
	if (m_fixtureDef->manufacturer().length() == 0)
	{
		QMessageBox::warning(this, 
				     "Missing important information",
				     "Missing manufacturer name.\n" \
				     "Unable to save fixture.");
		m_tab->setCurrentIndex(0);
		m_manufacturerEdit->setFocus();
		return false;
	}
	else if (m_fixtureDef->model().length() == 0)
	{
		QMessageBox::warning(this, 
				     "Missing important information",
				     "Missing fixture model name.\n" \
				     "Unable to save fixture.");
		m_tab->setCurrentIndex(0);
		m_modelEdit->setFocus();
		return false;
	}

	return true;
}

bool QLCFixtureEditor::save()
{
	if (checkManufacturerModel() == false)
		return false;

	if (m_fileName.length() == 0)
	{
		return saveAs();
	}
	else
	{
		if (m_fixtureDef->saveXML(m_fileName) == true)
		{
			setModified(false);
			return true;
		}
		else
		{
			QMessageBox::critical(this, "Unable to save file",
					      QString("Error: ") +
					      strerror(errno));
			return false;
		}
	}
}

bool QLCFixtureEditor::saveAs()
{
	QString path;
	QDir dir;

	/* Bail out if there is no manufacturer or model */
	if (checkManufacturerModel() == false)
		return false;

	/* Create a file save dialog */
	QFileDialog dialog(this);
	dialog.setWindowTitle(tr("Save fixture definition"));
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setNameFilter(KFixtureFilter);

#ifdef Q_WS_X11
	/* Set the dialog's default directory to system fixture directory if
	   user is root (UID == 0). If UID != 0 set it to user fixture dir */
	uid_t uid = geteuid();
	if (uid == 0)
	{
		/* User is root. Use the system fixture directory. */
		path = QString(FIXTUREDIR);
		dir = QDir(path);
	}
	else
	{
		path = QString("%1/%2").arg(getenv("HOME")).arg(USERFIXTUREDIR);
		
		/* Ensure there is a directory for user fixtures */
		dir = QDir(path);
		if (dir.exists() == false)
			dir.mkpath(".");

		/* Append the system and user fixture dirs to the sidebar. This
		   is done on Linux only, because WIN32 & OSX ports save
		   fixtures in a user-writable directory. */
		QList <QUrl> sidebar;
		sidebar.append(QUrl::fromLocalFile(FIXTUREDIR));
		sidebar.append(QUrl::fromLocalFile(path));
		dialog.setSidebarUrls(sidebar);
	}
#else
	/* Win32 & OSX keep fixtures in a user-writable directory. Use that. */
	path = QString(FIXTUREDIR);
	dir = QDir(path);
#endif

	if (m_fileName.isEmpty() == true)
	{
		/* Construct a new path for the (yet) unnamed file */
		path = QString("%1-%2%3").arg(m_fixtureDef->manufacturer())
					 .arg(m_fixtureDef->model())
					 .arg(KExtFixture);
		dialog.setDirectory(dir);
		dialog.selectFile(path);
	}
	else
	{
		/* The fixture already has a file name. Use that then. */
		dialog.setDirectory(QDir(m_fileName));
		dialog.selectFile(m_fileName);
	}

	/* Execute the dialog */
	if (dialog.exec() != QDialog::Accepted)
		return false;
		
	path = dialog.selectedFiles().first();
	if (path.length() != 0)
	{
		if (path.right(strlen(KExtFixture)) != QString(KExtFixture))
			path += QString(KExtFixture);

		if (m_fixtureDef->saveXML(path) == true)
		{
			m_fileName = path;
			setCaption();
			setModified(false);
			return true;
		}
		else
		{
			QMessageBox::critical(this,"Unable to save file: ",
					      QString("Error: ") +
					      strerror(errno));
			return false;
		}
	}
	else
	{
		return false;
	}
}

void QLCFixtureEditor::setCaption()
{
	QString caption;
	QString fileName;
	
	fileName = m_fileName;
	if (fileName == QString::null)
		fileName = QString("New Fixture");
	
	/* If the document is modified, append an asterisk after the
	   filename. Otherwise the caption is just the current filename */
	if (m_modified == true)
		caption = fileName + QString(" *");
	else
		caption = fileName;

	parentWidget()->setWindowTitle(caption);
}

void QLCFixtureEditor::setModified(bool modified)
{
	m_modified = modified;
	setCaption();
}

/*****************************************************************************
 * General tab functions
 *****************************************************************************/

void QLCFixtureEditor::slotManufacturerTextEdited(const QString &text)
{
	m_fixtureDef->setManufacturer(text);
	setModified();
}

void QLCFixtureEditor::slotModelTextEdited(const QString &text)
{
	m_fixtureDef->setModel(text);
	setModified();
}

void QLCFixtureEditor::slotTypeActivated(const QString &text)
{
	m_fixtureDef->setType(text);
	setModified();
}

/*****************************************************************************
 * Channels tab functions
 *****************************************************************************/

void QLCFixtureEditor::slotChannelListSelectionChanged(QTreeWidgetItem* item)
{
	if (item == NULL)
	{
		m_removeChannelButton->setEnabled(false);
		m_editChannelButton->setEnabled(false);
	}
	else
	{
		m_removeChannelButton->setEnabled(true);
		m_editChannelButton->setEnabled(true);
	}
}

void QLCFixtureEditor::slotAddChannel()
{
	EditChannel ec(this);
	
	bool ok = false;
	while (ok == false)
	{
		if (ec.exec() == QDialog::Accepted)
		{
			if (m_fixtureDef->channel(ec.channel()->name()) != NULL)
			{
				QMessageBox::warning(this, 
					QString("Channel already exists"),
					QString("A channel by the name \"") + 
					ec.channel()->name() + 
					QString("\" already exists!"));
				
				ok = false;
			}
			else if (ec.channel()->name().length() == 0)
			{
				QMessageBox::warning(this, 
					QString("Channel has no name"),
					QString("You must give the channel a descriptive name!"));
				
				ok = false;
			}
			else
			{
				/* Create a new channel to the fixture */
				m_fixtureDef->addChannel(
					new QLCChannel(ec.channel()));
				refreshChannelList();
				setModified();
				
				ok = true;
			}
		}
		else
		{
			ok = true;
		}
	}
}

void QLCFixtureEditor::slotRemoveChannel()
{
	QLCChannel* channel = currentChannel();
	Q_ASSERT(channel != NULL);

	if (QMessageBox::question(this, "Remove Channel",
		QString("Are you sure you wish to remove channel: ") + channel->name(),
			QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
	{
		QTreeWidgetItem* item;
		QTreeWidgetItem* next;
		
		item = m_channelList->currentItem();
		if (m_channelList->itemBelow(item) != NULL)
			next = m_channelList->itemBelow(item);
		else if (m_channelList->itemAbove(item) != NULL)
			next = m_channelList->itemAbove(item);
		else
			next = NULL;
		
		// Remove the selected channel from the fixture (also deleted)
		m_fixtureDef->removeChannel(currentChannel());
		delete item;
		m_channelList->setCurrentItem(next);
		setModified();
	}
}

void QLCFixtureEditor::slotEditChannel()
{
	QLCChannel* real = NULL;
	QTreeWidgetItem* item = NULL;

	// Initialize the dialog with the selected logical channel or
	// bail out if there is no current selection
	real = currentChannel();
	if (real == NULL)
		return;

	EditChannel ec(this, real);
	if (ec.exec() == QDialog::Accepted)
	{
		// Copy the channel's contents to the real channel
		*real = *(ec.channel());

		item = m_channelList->currentItem();
		item->setText(KChannelsColumnName, real->name());
		item->setText(KChannelsColumnGroup, real->group());

		setModified();
	}
}

void QLCFixtureEditor::slotCopyChannel()
{
	_app->setCopyChannel(currentChannel());
}

void QLCFixtureEditor::slotPasteChannel()
{
	QLCChannel* ch = _app->copyChannel();
	if (ch != NULL && m_fixtureDef != NULL)
	{
		m_fixtureDef->addChannel(new QLCChannel(ch));
		refreshChannelList();
		setModified();
	}
}

void QLCFixtureEditor::refreshChannelList()
{
	QLCChannel* ch = NULL;
	QTreeWidgetItem* item = NULL;
	QString str;

	m_channelList->clear();

	// Fill channels list
	QListIterator <QLCChannel*> it(*m_fixtureDef->channels());
	while (it.hasNext() == true)
	{
		ch = it.next();

		item = new QTreeWidgetItem(m_channelList);
		item->setText(KChannelsColumnName, ch->name());
		item->setText(KChannelsColumnGroup, ch->group());

		// Store the channel pointer to the listview as a string
		str.sprintf("%lu", (unsigned long) ch);
		item->setText(KChannelsColumnPointer, str);
	}

	slotChannelListSelectionChanged(m_channelList->currentItem());
}

void QLCFixtureEditor::slotChannelListContextMenuRequested(const QPoint& pos)
{
	QAction editAction(QIcon(":/edit.png"), tr("Edit"), this);
	QAction copyAction(QIcon(":/editcopy.png"), tr("Copy"), this);
	QAction pasteAction(QIcon(":/editpaste.png"), tr("Paste"), this);
	QAction removeAction(QIcon(":/editdelete.png"), tr("Remove"), this);

	/* Group menu */
	QMenu groupMenu;
	groupMenu.setTitle("Set group");
	QStringListIterator it(QLCChannel::groupList());
	while (it.hasNext() == true)
		groupMenu.addAction(it.next());

	/* Master edit menu */
	QMenu menu;
	menu.setTitle(tr("Channels"));
	menu.addAction(&editAction);
	menu.addAction(&copyAction);
	menu.addAction(&pasteAction);
	menu.addSeparator();
	menu.addAction(&removeAction);
	menu.addSeparator();
	menu.addMenu(&groupMenu);

	if (m_channelList->currentItem() == NULL)
	{
		copyAction.setEnabled(false);
		removeAction.setEnabled(false);
	}

	if (_app->copyChannel() == NULL)
		pasteAction.setEnabled(false);

	QAction* selectedAction = menu.exec(pos);
	if (selectedAction == NULL)
		return;
	else if (selectedAction->text() == tr("Edit"))
		slotEditChannel();
	else if (selectedAction->text() == tr("Copy"))
		slotCopyChannel();
	else if (selectedAction->text() == tr("Paste"))
		slotPasteChannel();
	else if (selectedAction->text() == tr("Remove"))
		slotRemoveChannel();
	else
	{
		/* Group menu hook */
		QLCChannel* ch = NULL;
		QTreeWidgetItem* node = NULL;

		ch = currentChannel();
		if (ch != NULL)
			ch->setGroup(selectedAction->text());
		node = m_channelList->currentItem();
		if (node != NULL)
			node->setText(KChannelsColumnGroup,
				      selectedAction->text());
	}

	setModified();
}

QLCChannel* QLCFixtureEditor::currentChannel()
{
	QLCChannel* ch = NULL;
	QTreeWidgetItem* item = NULL;

	// Convert the string-form ulong to a QLCChannel pointer and return it
	item = m_channelList->currentItem();
	if (item != NULL)
		ch = (QLCChannel*) item->text(KChannelsColumnPointer).toULong();

	return ch;
}

/*****************************************************************************
 * Modes tab functions
 *****************************************************************************/

void QLCFixtureEditor::slotModeListSelectionChanged(QTreeWidgetItem* item)
{
	if (item == NULL)
	{
		m_removeModeButton->setEnabled(false);
		m_editModeButton->setEnabled(false);
	}
	else
	{
		m_removeModeButton->setEnabled(true);
		m_editModeButton->setEnabled(true);
	}
}

void QLCFixtureEditor::slotAddMode()
{
	EditMode em(_app, m_fixtureDef);
	bool ok = false;
	while (ok == false)
	{
		if (em.exec() == QDialog::Accepted)
		{
			if (m_fixtureDef->mode(em.mode()->name()) != NULL)
			{
				QMessageBox::warning(
					this, 
					tr("Unable to add mode"),
					tr("Another mode by that name already exists"));
				
				// User must rename the mode to continue
				ok = false;
			}
			else if (em.mode()->name().length() == 0)
			{
				QMessageBox::warning(
					this, 
					tr("Unable to add mode"),
					tr("You must give a name to the mode"));
				
				ok = false;
			}
			else
			{
				ok = true;
				m_fixtureDef->addMode(
					new QLCFixtureMode(em.mode()));
				refreshModeList();
				setModified();
			}
		}
		else
		{
			ok = true;
		}
	}
}

void QLCFixtureEditor::slotRemoveMode()
{
	QLCFixtureMode* mode = currentMode();
	
	if (QMessageBox::question(this, "Remove Mode",
		QString("Are you sure you wish to remove mode: ") + mode->name(),
			QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
	{
		QTreeWidgetItem* item;
		QTreeWidgetItem* next;
		
		item = m_modeList->currentItem();
		if (m_modeList->itemBelow(item) != NULL)
			next = m_modeList->itemBelow(item);
		else if (m_modeList->itemAbove(item) != NULL)
			next = m_modeList->itemAbove(item);
		else
			next = NULL;

		m_fixtureDef->removeMode(mode);
		delete item;
		setModified();
	}
}

void QLCFixtureEditor::slotEditMode()
{
	QLCFixtureMode* mode = currentMode();
	QString str;

	if (mode == NULL)
		return;
	
	EditMode em(this, mode);
	if (em.exec() == QDialog::Accepted)
	{
		QTreeWidgetItem* item = NULL;
		*mode = *(em.mode());
		
		item = m_modeList->currentItem();
		item->setText(KModesColumnName, mode->name());
		str.sprintf("%d", mode->channels());
		item->setText(KModesColumnChannels, str);
		
		setModified();
	}
}

void QLCFixtureEditor::slotCloneMode()
{
	QLCFixtureMode* mode = NULL;
	QLCFixtureMode* clone = NULL;
	bool ok = false;
	QString text;
	
	mode = currentMode();
	if (mode == NULL)
		return;
	
	while (1)
	{
		text = QInputDialog::getText(this, tr("Rename new mode"),
					     tr("Give a unique name for the mode"),
					     QLineEdit::Normal,
					     "Copy of " + mode->name(),
					     &ok);

		if (ok == true && text.isEmpty() == false)
		{
			/* User entered a name that is already found from
			   the fixture definition -> again */
			if (mode->fixtureDef()->mode(text) != NULL)
			{
				QMessageBox::information(
					this,
					tr("Invalid name"),
					tr("Another mode by that name already exists."));
				ok = false;
				continue;
			}

			clone = new QLCFixtureMode(mode);
			clone->setName(text);
			mode->fixtureDef()->addMode(clone);
			refreshModeList();
			break;
		}
		else
		{
			// User pressed cancel
			break;
		}
	}
}

void QLCFixtureEditor::slotModeListContextMenuRequested(const QPoint& pos)
{
	QAction editAction(QIcon(":/edit.png"), tr("Edit"), this);
	connect(&editAction, SIGNAL(triggered(bool)),
		this, SLOT(slotEditMode()));
	QAction cloneAction(QIcon(":/editcopy.png"), tr("Clone"), this);
	connect(&cloneAction, SIGNAL(triggered(bool)),
		this, SLOT(slotCloneMode()));
	QAction removeAction(QIcon(":/editdelete.png"), tr("Remove"), this);
	connect(&removeAction, SIGNAL(triggered(bool)),
		this, SLOT(slotRemoveMode()));

	QMenu menu;
	menu.setTitle(tr("Modes"));
	menu.addAction(&editAction);
	menu.addAction(&cloneAction);
	menu.addSeparator();
	menu.addAction(&removeAction);

	if (m_channelList->currentItem() == NULL)
	{
		editAction.setEnabled(false);
		cloneAction.setEnabled(false);
		removeAction.setEnabled(false);
	}

	menu.exec(pos);
}

void QLCFixtureEditor::refreshModeList()
{
	QTreeWidgetItem* item;
	QLCFixtureMode* mode;
	QString str;

	m_modeList->clear();

	// Fill channels list
	QListIterator <QLCFixtureMode*> it(*m_fixtureDef->modes());
	while (it.hasNext() == true)
	{
		mode = it.next();

		item = new QTreeWidgetItem(m_modeList);
		item->setText(KModesColumnName, mode->name());
		str.sprintf("%d", mode->channels());
		item->setText(KModesColumnChannels, str);

		// Store the channel pointer to the listview as a string
		str.sprintf("%lu", (unsigned long) mode);
		item->setText(KModesColumnPointer, str);
	}
	
	slotModeListSelectionChanged(m_modeList->currentItem());
}

QLCFixtureMode* QLCFixtureEditor::currentMode()
{
	QLCFixtureMode* mode = NULL;
	QTreeWidgetItem* item = NULL;

	// Convert the string-form ulong to a QLCChannel pointer and return it
	item = m_modeList->currentItem();
	if (item != NULL)
		mode = (QLCFixtureMode*) item->text(KModesColumnPointer).toULong();

	return mode;
}
