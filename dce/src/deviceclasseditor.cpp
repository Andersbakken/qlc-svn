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

#include <errno.h>
#include <string.h>

#include "common/settings.h"
#include "common/qlcfixture.h"
#include "common/qlcfixturemode.h"
#include "common/qlcchannel.h"
#include "common/qlcphysical.h"
#include "common/filehandler.h"

#include "app.h"
#include "configkeys.h"
#include "deviceclasseditor.h"
#include "editpresetvalue.h"
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

QLCFixtureEditor::QLCFixtureEditor(QWidget* parent, QLCFixture* fixture)
	: UI_QLCFixtureEditor(parent),
	  m_fixture(fixture),
	  m_modified(false)
{
}

QLCFixtureEditor::~QLCFixtureEditor()
{
	delete m_fixture;
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

	m_manufacturerEdit->setText(m_fixture->manufacturer());
	m_modelEdit->setText(m_fixture->model());

	m_typeCombo->setCurrentText(m_fixture->type());

	refreshChannelList();
	refreshModeList();

	setModified(false);
}

void QLCFixtureEditor::closeEvent(QCloseEvent* e)
{
	int r = 0;

	if (m_modified)
	{
		r = QMessageBox::information(this, KApplicationNameShort,
				"Do you want to save changes to fixture\n\""
				+ m_fixture->name() + "\"\nbefore closing?",
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
	if (m_fixture->manufacturer().length() == 0)
	{
		QMessageBox::warning(this, 
				     "Missing important information",
				     "Missing manufacturer name.\n" \
				     "Unable to save fixture.");
		m_tab->setCurrentPage(0);
		m_manufacturerEdit->setFocus();
		return false;
	}
	else if (m_fixture->model().length() == 0)
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
		
		if (m_fixture->saveXML(m_fileName) == true)
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
		path += m_fixture->manufacturer() + QString("-");
		path += m_fixture->model() + QString(".qxf");
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
	if (path.length() == 0)
	{
		if (path.right(strlen(KExtFixture)) != QString(KExtFixture))
			path += QString(KExtFixture);

		if (m_fixture->saveXML(path) == true)
		{
			m_fileName = path;
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
	/* If the document is modified, append an asterisk after the
	   filename. Otherwise the caption is just the current filename */
	QString caption;
	if (m_modified == true)
		caption = m_fileName + QString(" *");
	else
		caption = m_fileName;
	
	UI_QLCFixtureEditor::setCaption(caption);
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
	m_fixture->setManufacturer(text);
	setModified();
}

void QLCFixtureEditor::slotModelEditTextChanged(const QString &text)
{
	m_fixture->setModel(text);
	setModified();
}

void QLCFixtureEditor::slotTypeSelected(const QString &text)
{
	m_fixture->setType(text);
	setModified();
}

/*****************************************************************************
 * Channels tab functions
 *****************************************************************************/

void QLCFixtureEditor::slotChannelListSelectionChanged(QListViewItem* item)
{
}

void QLCFixtureEditor::slotAddChannelClicked()
{
	EditChannel* ec = NULL;

	ec = new EditChannel(this);
	ec->init();
	ec->exec();

	delete ec;
}

void QLCFixtureEditor::slotRemoveChannelClicked()
{
	// Remove the selected channel from the fixture (also deleted)
	m_fixture->removeChannel(currentChannel());
	
	// Remove the selected listview item
	delete m_channelList->currentItem();
}

void QLCFixtureEditor::slotEditChannelClicked()
{
	EditChannel* ec = NULL;
	QLCChannel* real = NULL;
	QListViewItem* item = NULL;

	// Initialize the dialog with the selected logical channel
	real = currentChannel();
	ec = new EditChannel(this, real);
	ec->init();
	
	if (ec->exec() == QDialog::Accepted)
	{
		// Copy the channel's contents to the real channel
		*real = *ec->channel();

		item = m_channelList->currentItem();
		item->setText(KChannelsColumnName, real->name());
		item->setText(KChannelsColumnGroup, real->group());
	}

	delete ec;
}

void QLCFixtureEditor::refreshChannelList()
{
	QPtrListIterator <QLCChannel> it(*m_fixture->channels());
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

/*****************************************************************************
 * Modes tab functions
 *****************************************************************************/

void QLCFixtureEditor::slotAddModeClicked()
{
	QLCFixtureMode* mode = NULL;
	EditMode* em = NULL;
	bool ok = false;
	
	em = new EditMode(_app);
	em->init();

	while (ok == false)
	{
		if (em->exec() == QDialog::Accepted)
		{
			if (m_fixture->searchMode(em->mode()->name()) != NULL)
			{
				QMessageBox::warning(this, 
					QString("Mode already exists"),
					QString("A mode by the name \"") + 
					em->mode()->name() + 
					QString("\" already exists!"));
				
				// User must rename the mode to continue
				ok = false;
			}
			else
			{
				ok = true;
				m_fixture->addMode(new QLCFixtureMode(em->mode()));
				refreshModeList();
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
	
	if (QMessageBox::question(this, "Remove Mode?",
		QString("Are you sure you wish to remove mode: ") + mode->name(),
			QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
	{
		m_fixture->removeMode(mode);
		refreshModeList();
	}
}

void QLCFixtureEditor::slotEditModeClicked()
{
	QLCFixtureMode* mode = currentMode();
	EditMode* em = NULL;
	QString str;
	
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
	}
	
	delete em;
}

void QLCFixtureEditor::refreshModeList()
{
	QPtrListIterator <QLCFixtureMode> it(*m_fixture->modes());
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
