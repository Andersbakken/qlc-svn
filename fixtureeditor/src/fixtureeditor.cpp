/*
  Q Light Controller - Fixture Editor
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

#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qevent.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qinputdialog.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qpopupmenu.h>

#include <errno.h>
#include <string.h>

#include "common/qlcfixturedef.h"
#include "common/qlcfixturemode.h"
#include "common/qlcchannel.h"
#include "common/qlccapability.h"
#include "common/qlcphysical.h"
#include "common/filehandler.h"

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
static const int KMenuClone ( 1 );
static const int KMenuPaste ( 3 );
static const int KMenuRemove ( 4 );

QLCFixtureEditor::QLCFixtureEditor(QWidget* parent, QLCFixtureDef* fixtureDef)
	: UI_FixtureEditor(parent, "Fixture Editor"),

	  m_fixtureDef(fixtureDef),
	  m_modified(false)
{
}

QLCFixtureEditor::~QLCFixtureEditor()
{
	delete m_fixtureDef;
}

void QLCFixtureEditor::init()
{
	setIcon(QPixmap(QString(PIXMAPS) + QString("/fixture.png")));

	/* Channel buttons */
	m_addChannelButton->setIconSet(QPixmap(QString(PIXMAPS) + 
				       QString("/edit_add.png")));
	m_removeChannelButton->setIconSet(QPixmap(QString(PIXMAPS) + 
					  QString("/edit_remove.png")));
	m_editChannelButton->setIconSet(QPixmap(QString(PIXMAPS) + 
					QString("/edit.png")));

	/* Mode buttons */
	m_removeModeButton->setIconSet(QPixmap(QString(PIXMAPS) + 
				       QString("/edit_remove.png")));
	m_addModeButton->setIconSet(QPixmap(QString(PIXMAPS) + 
				    QString("/edit_add.png")));
	m_editModeButton->setIconSet(QPixmap(QString(PIXMAPS) + 
				     QString("/edit.png")));

	m_manufacturerEdit->setText(m_fixtureDef->manufacturer());
	m_modelEdit->setText(m_fixtureDef->model());

	m_typeCombo->setCurrentText(m_fixtureDef->type());

	refreshChannelList();
	refreshModeList();
	setCaption();

	setModified(false);
}

void QLCFixtureEditor::closeEvent(QCloseEvent* e)
{
	int r = 0;

	if (m_modified)
	{
		r = QMessageBox::information(this,
					     "Closing...",
				"Do you want to save changes to fixture\n\""
				+ m_fixtureDef->name() + "\"\nbefore closing?",
				QMessageBox::Yes,
				QMessageBox::No,
				QMessageBox::Cancel);
		if (r == QMessageBox::Yes)
		{
			if (save())
			{
				e->accept();
				emit closed(this);
			}
		}
		else if (r == QMessageBox::No)
		{
			e->accept();
			emit closed(this);
		}
		else
		{
			e->ignore();
		}
	}
	else
	{
		e->accept();
		emit closed(this);
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
		m_tab->setCurrentPage(0);
		m_manufacturerEdit->setFocus();
		return false;
	}
	else if (m_fixtureDef->model().length() == 0)
	{
		QMessageBox::warning(this, 
				     "Missing important information",
				     "Missing fixture model name.\n" \
				     "Unable to save fixture.");
		m_tab->setCurrentPage(0);
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
		/* If the current filename has .deviceclass extension, remind
		   the user that the new extension & format will be .qxf */
		if (newExtensionReminder() == false)
			return false;

		/* Ensure that the file will have the .qxf extension */
		ensureNewExtension();
		
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

	if (checkManufacturerModel() == false)
		return false;

	if (m_fileName.length() == 0)
	{
		path = QString(FIXTURES) + QString("/");
		path += m_fixtureDef->manufacturer() + QString("-");
		path += m_fixtureDef->model() + QString(".qxf");
	}
	else
	{
		/* If the current filename has .deviceclass extension, remind
		   the user that the new extension & format will be .qxf */
		if (newExtensionReminder() == FALSE)
			return false;

		/* Ensure that the file will have the .qxf extension */
		ensureNewExtension();

		path = m_fileName;
	}

	path = QFileDialog::getSaveFileName(path, "Fixtures (*.qxf)", this);
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

	UI_FixtureEditor::setCaption(caption);
}

void QLCFixtureEditor::setModified(bool modified)
{
	m_modified = modified;
	setCaption();
}

void QLCFixtureEditor::ensureNewExtension()
{
	if (m_fileName.right(strlen(KExtLegacyDeviceClass))
		== KExtLegacyDeviceClass)
	{
		/* Rename the file extension to the new one */
		m_fileName = m_fileName.left(m_fileName.length() -
			strlen(KExtLegacyDeviceClass)) +
			QString(KExtFixture);
		setCaption();
	}
}

bool QLCFixtureEditor::newExtensionReminder()
{
	int res = QMessageBox::Ok;
	
	if (m_fileName.right(strlen(KExtLegacyDeviceClass))
		== KExtLegacyDeviceClass)
	{
		if (QMessageBox::question(this, "Convert file",
			"The old file format will be converted " \
			"to the new QXF format\nand the file extension " \
			"will be changed to " KExtFixture ". OK to continue?",
			QMessageBox::Ok, QMessageBox::Cancel) 
			== QMessageBox::Cancel)
		{
			return false;
		}
	}
	
	return true;
}

/*****************************************************************************
 * General tab functions
 *****************************************************************************/

void QLCFixtureEditor::slotManufacturerEditTextChanged(const QString &text)
{
	m_fixtureDef->setManufacturer(text);
	setModified();
}

void QLCFixtureEditor::slotModelEditTextChanged(const QString &text)
{
	m_fixtureDef->setModel(text);
	setModified();
}

void QLCFixtureEditor::slotTypeSelected(const QString &text)
{
	m_fixtureDef->setType(text);
	setModified();
}

/*****************************************************************************
 * Channels tab functions
 *****************************************************************************/

void QLCFixtureEditor::slotChannelListSelectionChanged(QListViewItem* item)
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

void QLCFixtureEditor::slotAddChannelClicked()
{
	EditChannel* ec = NULL;
	bool ok = false;
	
	ec = new EditChannel(this);
	ec->init();
	
	while (ok == false)
	{
		if (ec->exec() == QDialog::Accepted)
		{
			if (m_fixtureDef->channel(ec->channel()->name()) != NULL)
			{
				QMessageBox::warning(this, 
					QString("Channel already exists"),
					QString("A channel by the name \"") + 
					ec->channel()->name() + 
					QString("\" already exists!"));
				
				ok = false;
			}
			else if (ec->channel()->name().length() == 0)
			{
				QMessageBox::warning(this, 
					QString("Channel has no name"),
					QString("You must give the channel a descriptive name!"));
				
				ok = false;
			}
			else
			{
				/* Create a new channel to the fixture */
				m_fixtureDef->addChannel(new QLCChannel(ec->channel()));
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

	delete ec;
}

void QLCFixtureEditor::slotRemoveChannelClicked()
{
	QLCChannel* channel = currentChannel();
	
	if (QMessageBox::question(this, "Remove Channel",
		QString("Are you sure you wish to remove channel: ") + channel->name(),
			QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
	{
		// Remove the selected channel from the fixture (also deleted)
		m_fixtureDef->removeChannel(currentChannel());
		refreshChannelList();
		setModified();
	}
}

void QLCFixtureEditor::slotEditChannelClicked()
{
	EditChannel* ec = NULL;
	QLCChannel* real = NULL;
	QListViewItem* item = NULL;

	// Initialize the dialog with the selected logical channel or
	// bail out if there is no current selection
	real = currentChannel();
	if (real == NULL)
		return;
	
	ec = new EditChannel(this, real);
	ec->init();
	
	if (ec->exec() == QDialog::Accepted)
	{
		// Copy the channel's contents to the real channel
		*real = *ec->channel();

		item = m_channelList->currentItem();
		item->setText(KChannelsColumnName, real->name());
		item->setText(KChannelsColumnGroup, real->group());
		
		setModified();
	}

	delete ec;
}

void QLCFixtureEditor::refreshChannelList()
{
	QPtrListIterator <QLCChannel> it(*m_fixtureDef->channels());
	QLCChannel* ch = NULL;
	QListViewItem* item = NULL;
	QString str;

	m_channelList->clear();

	// Fill channels list
	while ( (ch = it.current()) != 0)
	{
		item = new QListViewItem(m_channelList);
		item->setText(KChannelsColumnName, ch->name());
		item->setText(KChannelsColumnGroup, ch->group());

		// Store the channel pointer to the listview as a string
		str.sprintf("%d", (unsigned long) ch);
		item->setText(KChannelsColumnPointer, str);
		
		++it;
	}
	
	slotChannelListSelectionChanged(m_channelList->currentItem());
}

QLCChannel* QLCFixtureEditor::currentChannel()
{
	QLCChannel* ch = NULL;
	QListViewItem* item = NULL;

	// Convert the string-form ulong to a QLCChannel pointer and return it
	item = m_channelList->currentItem();
	if (item != NULL)
		ch = (QLCChannel*) item->text(KChannelsColumnPointer).toULong();

	return ch;
}

void QLCFixtureEditor::slotChannelListContextMenuRequested(QListViewItem* item,
							   const QPoint& pos,
							   int col)
{
	QStringList::Iterator it;
	QStringList groups;
	QPopupMenu* menu = NULL;
	QPopupMenu* groupMenu = NULL;

	/* Master edit menu */
	menu = new QPopupMenu();
	menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/edit.png")),
			 "Edit...", KMenuEdit);
	menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editcopy.png")),
			 "Copy", KMenuCopy);
	menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editpaste.png")),
			 "Paste", KMenuPaste);
	menu->insertSeparator();
	menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editdelete.png")),
			 "Remove", KMenuRemove);

	/* Group menu */
	groupMenu = new QPopupMenu();
	groups = QLCChannel::groupList();
	for (it = groups.begin(); it != groups.end(); ++it)
		groupMenu->insertItem(*it,
				      10000 + QLCChannel::groupToIndex(*it));
	menu->insertSeparator();
	menu->insertItem("Set group", groupMenu);

	if (item == NULL)
	{
		menu->setItemEnabled(KMenuCopy, false);
		menu->setItemEnabled(KMenuRemove, false);
	}

	if (_app->copyChannel() == NULL)
	{
		menu->setItemEnabled(KMenuPaste, false);
	}

	connect(menu, SIGNAL(activated(int)),
		this, SLOT(slotChannelListMenuActivated(int)));
	connect(groupMenu, SIGNAL(activated(int)),
		this, SLOT(slotChannelListMenuActivated(int)));

	menu->exec(pos);

	delete groupMenu;
	delete menu;
}

void QLCFixtureEditor::slotChannelListMenuActivated(int item)
{
	switch (item)
	{

	case KMenuEdit:
		slotEditChannelClicked();
		break;

	case KMenuCopy:
		_app->setCopyChannel(currentChannel());
		break;

	case KMenuPaste:
		pasteChannel();
		break;

	case KMenuRemove:
		slotRemoveChannelClicked();
		break;

	default:
	{
		QString group;
		QLCChannel* ch = NULL;
		QListViewItem* node = NULL;

		group = QLCChannel::indexToGroup(item - 10000);
		ch = currentChannel();
		if (ch != NULL)
			ch->setGroup(group);
		node = m_channelList->currentItem();
		if (node != NULL)
			node->setText(KChannelsColumnGroup, group);
		setModified();
	}
	}
}

void QLCFixtureEditor::pasteChannel()
{
	QLCChannel* ch = _app->copyChannel();
	if (ch != NULL && m_fixtureDef != NULL)
	{
		m_fixtureDef->addChannel(new QLCChannel(ch));
		refreshChannelList();
		setModified();
	}
}

/*****************************************************************************
 * Modes tab functions
 *****************************************************************************/

void QLCFixtureEditor::slotModeListSelectionChanged(QListViewItem* item)
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

void QLCFixtureEditor::slotAddModeClicked()
{
	QLCFixtureMode* mode = NULL;
	EditMode* em = NULL;
	bool ok = false;
	
	em = new EditMode(_app, m_fixtureDef);
	em->init();

	while (ok == false)
	{
		if (em->exec() == QDialog::Accepted)
		{
			if (m_fixtureDef->mode(em->mode()->name()) != NULL)
			{
				QMessageBox::warning(this, 
					QString("Mode already exists"),
					QString("A mode by the name \"") + 
					em->mode()->name() + 
					QString("\" already exists!"));
				
				// User must rename the mode to continue
				ok = false;
			}
			else if (em->mode()->name().length() == 0)
			{
				QMessageBox::warning(this, 
					QString("Mode has no name"),
					QString("You must give the mode a name!"));
				
				ok = false;
			}
			else
			{
				ok = true;
				m_fixtureDef->addMode(new QLCFixtureMode(em->mode()));
				refreshModeList();
				setModified();
			}
		}
		else
		{
			ok = true;
		}
	}
	
	delete em;
}

void QLCFixtureEditor::slotRemoveModeClicked()
{
	QLCFixtureMode* mode = currentMode();
	
	if (QMessageBox::question(this, "Remove Mode",
		QString("Are you sure you wish to remove mode: ") + mode->name(),
			QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
	{
		m_fixtureDef->removeMode(mode);
		refreshModeList();
		setModified();
	}
}

void QLCFixtureEditor::slotEditModeClicked()
{
	QLCFixtureMode* mode = currentMode();
	EditMode* em = NULL;
	QString str;

	if (mode == NULL)
		return;
	
	em = new EditMode(_app, mode);
	em->init();

	if (em->exec() == QDialog::Accepted)
	{
		QListViewItem* item = NULL;
		*mode = *em->mode();
		
		item = m_modeList->currentItem();
		item->setText(KModesColumnName, mode->name());
		str.sprintf("%d", mode->channels());
		item->setText(KModesColumnChannels, str);
		
		setModified();
	}
	
	delete em;
}

void QLCFixtureEditor::slotModeListContextMenuRequested(QListViewItem* item,
							const QPoint& pos,
							int col)
{
	QPopupMenu* menu = NULL;

	menu = new QPopupMenu();
	menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/edit.png")),
			 "Edit...", KMenuEdit);
	menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editcopy.png")),
			 "Clone", KMenuClone);
	menu->insertSeparator();
	menu->insertItem(QPixmap(QString(PIXMAPS) + QString("/editdelete.png")),
			 "Remove", KMenuRemove);

	if (item == NULL)
	{
		menu->setItemEnabled(KMenuEdit, false);
		menu->setItemEnabled(KMenuClone, false);
		menu->setItemEnabled(KMenuRemove, false);
	}

	connect(menu, SIGNAL(activated(int)),
		this, SLOT(slotModeListMenuActivated(int)));

	menu->exec(pos);
	delete menu;
}

void QLCFixtureEditor::slotModeListMenuActivated(int item)
{
	switch (item)
	{

	case KMenuEdit:
		slotEditModeClicked();
		break;

	case KMenuClone:
		cloneCurrentMode();
		break;

	case KMenuRemove:
		slotRemoveModeClicked();
		break;

	default:
		break;
	}
}

void QLCFixtureEditor::refreshModeList()
{
	QPtrListIterator <QLCFixtureMode> it(*m_fixtureDef->modes());
	QLCFixtureMode* mode = NULL;
	QListViewItem* item = NULL;
	QString str;

	m_modeList->clear();

	// Fill channels list
	while ( (mode = it.current()) != 0)
	{
		item = new QListViewItem(m_modeList);
		item->setText(KModesColumnName, mode->name());
		str.sprintf("%d", mode->channels());
		item->setText(KModesColumnChannels, str);

		// Store the channel pointer to the listview as a string
		str.sprintf("%d", (unsigned long) mode);
		item->setText(KModesColumnPointer, str);
		
		++it;
	}
	
	slotModeListSelectionChanged(m_modeList->currentItem());
}

QLCFixtureMode* QLCFixtureEditor::currentMode()
{
	QLCFixtureMode* mode = NULL;
	QListViewItem* item = NULL;

	// Convert the string-form ulong to a QLCChannel pointer and return it
	item = m_modeList->currentItem();
	if (item != NULL)
		mode = (QLCFixtureMode*) item->text(KModesColumnPointer).toULong();

	return mode;
}

void QLCFixtureEditor::cloneCurrentMode()
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
		text = QInputDialog::getText("Rename new mode",
					     "Enter a UNIQUE name for the new mode",
					     QLineEdit::Normal,
					     "Copy of " + mode->name(),
					     &ok,
					     this);

		if (ok == true && text.isEmpty() == false)
		{
			/* User entered a name that is already found from
			   the fixture definition -> again */
			if (mode->fixtureDef()->mode(text) != NULL)
			{
				QMessageBox::information(this, "Invalid name",
					 "That name is already in use!\n" \
					 "Try something else...");
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
