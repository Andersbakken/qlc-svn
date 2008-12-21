/*
  Q Light Controller
  inputpatcheditor.cpp

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
#include <QTreeWidget>
#include <QToolButton>
#include <QMessageBox>
#include <QComboBox>
#include <QGroupBox>
#include <QVariant>
#include <QDebug>
#include <QIcon>
#include <QFile>
#include <QDir>

#include <common/qlcinputdevice.h>
#include <common/qlctypes.h>
#include <common/qlcfile.h>

#include "inputtemplateeditor.h"
#include "inputpatcheditor.h"
#include "inputpatch.h"
#include "inputmap.h"
#include "app.h"

extern App* _app;

/* Plugin column structure */
#define KMapColumnName  0
#define KMapColumnInput 1

/* Template column structure */
#define KTemplateColumnName 0

InputPatchEditor::InputPatchEditor(QWidget* parent, t_input_universe universe,
				   InputPatch* inputPatch) : QDialog(parent)
{
	Q_ASSERT(universe < _app->inputMap()->universes());
	m_universe = universe;

	Q_ASSERT(inputPatch != NULL);
	m_inputPatch = inputPatch;

	m_pluginName = inputPatch->pluginName();
	m_input = inputPatch->input();
	m_templateName = inputPatch->templateName();

	/* Setup UI controls */
	setupUi(this);
	setupMappingPage();
	setupTemplatePage();
}

InputPatchEditor::~InputPatchEditor()
{
}

void InputPatchEditor::accept()
{
	_app->inputMap()->setPatch(m_universe, m_pluginName, m_input,
				   m_templateName);

	QDialog::accept();
}

/****************************************************************************
 * Mapping page
 ****************************************************************************/

void InputPatchEditor::setupMappingPage()
{
	/* Plugin tree */
	connect(m_mapTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,
						     QTreeWidgetItem*)),
		this, SLOT(slotMappingCurrentItemChanged(QTreeWidgetItem*)));

	/* Plugin/input information */
	m_configureButton->setIcon(QIcon(":/configure.png"));
	connect(m_configureButton, SIGNAL(clicked()),
		this, SLOT(slotConfigureInputClicked()));

	/* Input universe that is being edited */
	setWindowTitle(tr("Mapping properties for input universe %1")
			.arg(m_universe + 1));

	/* Fill the map tree with available plugins */
	fillMappingTree();
}

void InputPatchEditor::fillMappingTree()
{
	QTreeWidgetItem* pitem;
	QTreeWidgetItem* iitem;
	QString pluginName;
	int i;

	m_mapTree->clear();

	/* Add an empty item so that user can choose not to assign any plugin
	   to an input universe */
	pitem = new QTreeWidgetItem(m_mapTree);
	pitem->setText(KMapColumnName, KInputNone);
	pitem->setText(KMapColumnInput, QString("%1").arg(KInputInvalid));

	/* Set "Nothing" selected if there is no valid input selected */
	if (m_input == KInputInvalid)
		m_mapTree->setCurrentItem(pitem);

	/* Go thru available plugins and put them as the tree's root nodes. */
	QStringListIterator pit(_app->inputMap()->pluginNames());
	while (pit.hasNext() == true)
	{
		i = 0;

		pluginName = pit.next();
		pitem = new QTreeWidgetItem(m_mapTree);
		pitem->setText(KMapColumnName, pluginName);
		pitem->setText(KMapColumnInput, QString("%1").arg(KInputInvalid));

		/* Go thru available inputs provided by each plugin and put
		   them as their parent plugin's leaf nodes. */
		QStringListIterator iit(_app->inputMap()->pluginInputs(
								pluginName));
		while (iit.hasNext() == true)
		{
			iitem = new QTreeWidgetItem(pitem);
			iitem->setText(KMapColumnName, iit.next());
			iitem->setText(KMapColumnInput, QString("%1").arg(i));

			/* Select the currently mapped plugin input */
			if (m_pluginName == pluginName && m_input == i)
				m_mapTree->setCurrentItem(iitem);

			i++;
		}

		/* If no inputs were appended to the plugin node, put a
		   "Nothing" node there. */
		if (i == 0)
		{
			iitem = new QTreeWidgetItem(pitem);
			iitem->setText(KMapColumnName, KInputNone);
			iitem->setText(KMapColumnInput,
				       QString("%1").arg(KInputInvalid));
		}
	}
}

void InputPatchEditor::slotMappingCurrentItemChanged(QTreeWidgetItem* item)
{
	if (item == NULL)
	{
		m_input = KInputInvalid;
		m_inputName = QString::null;
		m_pluginName = QString::null;
		m_configureButton->setEnabled(false);
	}
	else
	{
		m_input = item->text(KMapColumnInput).toInt();
		if (m_input != KInputInvalid)
		{
			m_inputName = item->text(KMapColumnName);
			m_pluginName = item->parent()->text(KMapColumnName);
		}
		else
		{
			m_inputName = QString::null;
			m_pluginName = item->text(KMapColumnName);
		}

		if (m_pluginName == KInputNone)
			m_configureButton->setEnabled(false);
		else
			m_configureButton->setEnabled(true);
	}

	/* Update plugin/input information */
	QString info;
	info = _app->inputMap()->pluginStatus(m_pluginName, m_input);
	m_infoBrowser->setText(info);
}

void InputPatchEditor::slotConfigureInputClicked()
{
	_app->inputMap()->configurePlugin(m_pluginName);

	/* Refill the mapping tree in case configuration changed something */
	fillMappingTree();
}

/****************************************************************************
 * Template tree
 ****************************************************************************/
 
void InputPatchEditor::setupTemplatePage()
{
	/* Buttons */
	m_addTemplateButton->setIcon(QIcon(":/edit_add.png"));
	m_removeTemplateButton->setIcon(QIcon(":/edit_remove.png"));
	m_editTemplateButton->setIcon(QIcon(":/edit.png"));

	connect(m_addTemplateButton, SIGNAL(clicked()),
		this, SLOT(slotAddTemplateClicked()));
	connect(m_removeTemplateButton, SIGNAL(clicked()),
		this, SLOT(slotRemoveTemplateClicked()));
	connect(m_editTemplateButton, SIGNAL(clicked()),
		this, SLOT(slotEditTemplateClicked()));

	/* Fill the template tree with available templates */
	fillTemplateTree();

	/* Listen to itemChanged() signals to catch check state changes */
	connect(m_templateTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
		this, SLOT(slotTemplateItemChanged(QTreeWidgetItem*,int)));

	/* Double click acts as edit button click */
	connect(m_templateTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotEditTemplateClicked()));
}

void InputPatchEditor::fillTemplateTree()
{
	QTreeWidgetItem* item;

	m_templateTree->clear();

	/* Add an option for having no template at all */
	item = new QTreeWidgetItem(m_templateTree);
	updateTemplateItem(KInputNone, item);

	/* Insert available input device templates to the tree */
	QStringListIterator it(_app->inputMap()->deviceTemplateNames());
	while (it.hasNext() == true)
	{
		item = new QTreeWidgetItem(m_templateTree);
		updateTemplateItem(it.next(), item);
	}
}

void InputPatchEditor::updateTemplateItem(const QString& name,
					  QTreeWidgetItem* item)
{
	Q_ASSERT(item != NULL);

	item->setText(KTemplateColumnName, name);

	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	if (m_templateName == name)
		item->setCheckState(KTemplateColumnName, Qt::Checked);
	else
		item->setCheckState(KTemplateColumnName, Qt::Unchecked);
}

void InputPatchEditor::slotTemplateItemChanged(QTreeWidgetItem* item,
					       int column)
{
	Q_UNUSED(column);

	if (item->checkState(KTemplateColumnName) == Qt::Checked)
	{
		/* Temporarily disable this signal to prevent an endless loop */
		disconnect(m_templateTree,
			   SIGNAL(itemChanged(QTreeWidgetItem*,int)),
			   this,
			   SLOT(slotTemplateItemChanged(QTreeWidgetItem*,int)));

		/* Set all other items unchecked... */
		for (int i = 0; i < m_templateTree->topLevelItemCount(); i++)
		{
			QTreeWidgetItem* another;
			another = m_templateTree->topLevelItem(i);

			if (another != item)
			{
				/* ...except the one that was just checked */
				another->setCheckState(KTemplateColumnName,
							Qt::Unchecked);
			}
		}

		/* Store the selected template name */
		m_templateName = item->text(KTemplateColumnName);

		/* Start listening to this signal once again */
		connect(m_templateTree,
			SIGNAL(itemChanged(QTreeWidgetItem*,int)),
			this,
			SLOT(slotTemplateItemChanged(QTreeWidgetItem*,int)));
	}
	else
	{
		/* Don't allow unchecking an item by clicking it. Only allow
		   changing the check state by checking another item. */
		item->setCheckState(KTemplateColumnName, Qt::Checked);
	}
}

void InputPatchEditor::slotAddTemplateClicked()
{
	/* Create a new input template and start editing it */
	InputTemplateEditor ite(this, NULL);
edit:
	if (ite.exec() == QDialog::Accepted)
	{
		QLCInputDevice* dt;
		QString path;
		QDir dir;

#ifdef Q_WS_X11
		/* If the current user is root, use the system template dir
		   for saving templates. Otherwise use the user's home dir.
		   This is done on Linux only, because Win32 & OSX save
		   system templates in a user-writable directory. */
		if (geteuid() == 0)
		{
			dir = QDir(INPUTTEMPLATEDIR);
		}
		else
		{
			path = QString("%1/%2").arg(getenv("HOME"))
					       .arg(USERINPUTTEMPLATEDIR);
			dir = QDir(path);
		}

		/* Ensure that the selected template directory exists */
		if (dir.exists() == false)
			dir.mkpath(".");
#else
		/* Use the system input template dir for Win32/OSX */
		dir = QDir(INPUTTEMPLATEDIR);
#endif
		/* Construct a descriptive file name for the template */
		path = QString("%1/%2-%3")
				.arg(dir.absolutePath())
				.arg(ite.deviceTemplate()->manufacturer())
				.arg(ite.deviceTemplate()->model());

		/* Ensure that creating a new input template won't overwrite
		   an existing file. */
		if (QFile::exists(path + KExtInputDevice) == true)
		{
			for (int i = 1; i < INT_MAX; i++)
			{
				/* Start adding a number suffix to the file
				   name and stop when a unique file name is
				   found. */
				QString s;
				s = QString("%1-%2%3").arg(path).arg(i)
						      .arg(KExtInputDevice);
				if (QFile::exists(s) == false)
				{
					path = s;
					break;
				}
			}
		}

		/* Create a new non-const copy of the template and
		   reparent it to the input map */
		dt = new QLCInputDevice(*ite.deviceTemplate());

		/* Save it to a file, go back to edit if save failed */
		if (dt->saveXML(path) == false)
		{
			QMessageBox::warning(this, tr("Saving failed"),
				tr("Unable to save the template to %1")
				.arg(QDir::toNativeSeparators(path)));
			delete dt;
			goto edit;
		}
		else
		{
			/* Add the new template to input map */
			_app->inputMap()->addDeviceTemplate(dt);

			/* Add the new template to our tree widget */
			QTreeWidgetItem* item;
			item = new QTreeWidgetItem(m_templateTree);
			updateTemplateItem(dt->name(), item);
		}
	}
}

void InputPatchEditor::slotRemoveTemplateClicked()
{
	QLCInputDevice* deviceTemplate;
	QTreeWidgetItem* item;
	QString name;
	int r;

	/* Find out the currently selected item */
	item = m_templateTree->currentItem();
	if (item == NULL)
		return;

	/* Get the currently selected template object by its name */
	name = item->text(KTemplateColumnName);
	deviceTemplate = _app->inputMap()->deviceTemplate(name);
	if (deviceTemplate == NULL)
		return;

	/* Ask for user confirmation */
	r = QMessageBox::question(this, tr("Delete template"),
		tr("Do you wish to permanently delete template \"%1\"?")
		.arg(deviceTemplate->name()),
		QMessageBox::Yes, QMessageBox::No);
	if (r == QMessageBox::Yes)
	{
		/* Attempt to delete the file first */
		QFile file(deviceTemplate->path());
		if (file.remove() == true)
		{
			/* Successful deletion. Remove the template from
			   input map and our tree widget */
			_app->inputMap()->removeDeviceTemplate(name);
			delete item;
		}
		else
		{
			/* Annoy the user even more after deletion failure */
			QMessageBox::warning(this, tr("File deletion failed"),
						tr("Unable to delete file %1")
						.arg(file.errorString()));
		}
	}
}

void InputPatchEditor::slotEditTemplateClicked()
{
	QLCInputDevice* deviceTemplate;
	QTreeWidgetItem* item;
	QString name;

	/* Get the currently selected item and bail out if nothing or "None"
	   is selected */
	item = m_templateTree->currentItem();
	if (item == NULL || item->text(KTemplateColumnName) == KInputNone)
		return;

	/* Get the currently selected template by its name */
	name = item->text(KTemplateColumnName);
	deviceTemplate = _app->inputMap()->deviceTemplate(name);
	if (deviceTemplate == NULL)
		return;

	/* Edit the template and update the item if OK was pressed */
	InputTemplateEditor ite(this, deviceTemplate);
edit:
	if (ite.exec() == QDialog::Rejected)
		return;

	QString path;

	/* Copy the channel's contents from the editor's copy to
	   the actual object (with QLCInputDevice::operator=()). */
	*deviceTemplate = *ite.deviceTemplate();

#ifdef Q_WS_X11
	/* If the current user is root, save the template to its old path.
	   Otherwise use the user's home dir and generate a new file name
	   if necessary. This is done on Linux only, because Win32 & OSX save
	   templates always in a user-writable directory. */
	if (geteuid() == 0)
	{
		path = deviceTemplate->path();
	}
	else
	{
		/* Ensure that user template directory exists */
		path = QString("%1/%2").arg(getenv("HOME"))
				       .arg(USERINPUTTEMPLATEDIR);
		QDir dir = QDir(path);
		if (dir.exists() == false)
			dir.mkpath(".");

		/* Check, whether the template was originally saved
		   in the system directory. If it is, construct a
		   new name for it into the user's template dir. */
		path = deviceTemplate->path();
		if (path.contains(getenv("HOME")) == false)
		{
			/* Construct a descriptive file name for
			   the template under user's HOME dir */
			path = QString("%1/%2-%3")
				.arg(dir.absolutePath())
				.arg(deviceTemplate->manufacturer())
				.arg(deviceTemplate->model());

			/* Ensure that creating a new file won't
			   overwrite an existing file. */
			if (QFile::exists(path + KExtInputDevice))
			{
				/* Add an increasing number suffix to the name
				   and stop when a unique name is found. */
				for (int i = 1; i < INT_MAX; i++)
				{
					QString s;
					s = QString("%1-%2%3").arg(path)
						.arg(i).arg(KExtInputDevice);
					if (QFile::exists(s) == false)
					{
						path = s;
						break;
					}
				}
			}
		}
	}
#else
	/* Win32 & OSX save input templates in a user-writable directory,
	   so we can use that directly. */
	path = deviceTemplate->path();
#endif
	/* Save the template */
	if (deviceTemplate->saveXML(path) == true)
	{
		/* Get the template's name from the template itself
		   since it may have changed making local variable
		   "name" invalid */
		updateTemplateItem(deviceTemplate->name(), item);
	}
	else
	{
		QMessageBox::warning(this, tr("Saving failed"),
			tr("Unable to save %1 to %2")
			.arg(deviceTemplate->name())
			.arg(QDir::toNativeSeparators(path)));
		goto edit;
	}
}
