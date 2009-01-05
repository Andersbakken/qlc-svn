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
#include <QButtonGroup>
#include <QTreeWidget>
#include <QToolButton>
#include <QMessageBox>
#include <QComboBox>
#include <QGroupBox>
#include <QVariant>
#include <QDebug>
#include <QFile>
#include <QDir>

#include <common/qlcinputdevice.h>
#include <common/qlctypes.h>
#include <common/qlcfile.h>

#include "inputdeviceeditor.h"
#include "inputpatcheditor.h"
#include "inputpatch.h"
#include "inputmap.h"
#include "app.h"

extern App* _app;

/* Plugin column structure */
#define KMapColumnName  0
#define KMapColumnInput 1

/* Device column structure */
#define KDeviceColumnName 0

InputPatchEditor::InputPatchEditor(QWidget* parent, t_input_universe universe,
				   const InputPatch* inputPatch)
	: QDialog(parent)
{
	Q_ASSERT(universe < _app->inputMap()->universes());
	Q_ASSERT(inputPatch != NULL);

	setupUi(this);

	m_universe = universe;
	setWindowTitle(tr("Mapping properties for input universe %1")
			.arg(m_universe + 1));

	/* Copy these so they can be applied if the user cancels */
	m_originalPluginName = inputPatch->pluginName();
	m_currentPluginName = inputPatch->pluginName();

	m_originalInput = inputPatch->input();
	m_currentInput = inputPatch->input();

	m_originalDeviceName = inputPatch->deviceName();
	m_currentDeviceName = inputPatch->deviceName();

	/* Setup UI controls */
	setupMappingPage();
	setupDevicePage();
}

InputPatchEditor::~InputPatchEditor()
{
}

void InputPatchEditor::reject()
{
	_app->inputMap()->setPatch(m_universe, m_originalPluginName,
				   m_originalInput, m_originalDeviceName);

	QDialog::reject();
}

void InputPatchEditor::accept()
{
	if (m_editorUniverseRadio->isChecked() == true)
		_app->inputMap()->setEditorUniverse(m_universe);

	QDialog::accept();
}

/****************************************************************************
 * Mapping page
 ****************************************************************************/

void InputPatchEditor::setupMappingPage()
{
	/* Fill the map tree with available plugins */
	fillMappingTree();

	/* Selection changes */
	connect(m_mapTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,
						     QTreeWidgetItem*)),
		this, SLOT(slotMapCurrentItemChanged(QTreeWidgetItem*)));

	/* Configure button */
	connect(m_configureButton, SIGNAL(clicked()),
		this, SLOT(slotConfigureInputClicked()));

	/* Prevent the editor uni radio button from being unchecked manually */
	QButtonGroup* group = new QButtonGroup(this);
	group->addButton(m_editorUniverseRadio);

	/* Set checked if the current universe is also the editor universe */
	if (_app->inputMap()->editorUniverse() == m_universe)
		m_editorUniverseRadio->setChecked(true);
}

void InputPatchEditor::fillMappingTree()
{
	QTreeWidgetItem* iitem = NULL;
	QTreeWidgetItem* pitem = NULL;
	QString pluginName;
	int i;

	/* Disable check state change tracking when the tree is filled */
	disconnect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
		   this, SLOT(slotMapItemChanged(QTreeWidgetItem*)));

	m_mapTree->clear();

	/* Add an empty item so that user can choose not to assign any plugin
	   to an input universe */
	pitem = new QTreeWidgetItem(m_mapTree);
	pitem->setText(KMapColumnName, KInputNone);
	pitem->setText(KMapColumnInput, QString("%1").arg(KInputInvalid));
	pitem->setFlags(pitem->flags() | Qt::ItemIsUserCheckable);

	/* Set "Nothing" selected if there is no valid input selected */
	if (m_currentInput == KInputInvalid)
		pitem->setCheckState(KMapColumnName, Qt::Checked);
	else
		pitem->setCheckState(KMapColumnName, Qt::Unchecked);

	/* Go thru available plugins and put them as the tree's root nodes. */
	QStringListIterator pit(_app->inputMap()->pluginNames());
	while (pit.hasNext() == true)
	{
		i = 0;

		pluginName = pit.next();
		pitem = new QTreeWidgetItem(m_mapTree);
		pitem->setText(KMapColumnName, pluginName);
		pitem->setText(KMapColumnInput, QString("%1")
							.arg(KInputInvalid));

		/* Go thru available inputs provided by each plugin and put
		   them as their parent plugin's leaf nodes. */
		QStringListIterator iit(
			_app->inputMap()->pluginInputs(pluginName));
		while (iit.hasNext() == true)
		{
			iitem = new QTreeWidgetItem(pitem);
			iitem->setText(KMapColumnName, iit.next());
			iitem->setText(KMapColumnInput, QString("%1").arg(i));
			iitem->setFlags(iitem->flags() |
					Qt::ItemIsUserCheckable);

			/* Select the currently mapped plugin input and expand
			   its parent node. */
			if (m_currentPluginName == pluginName &&
			    m_currentInput == i)
			{
				iitem->setCheckState(KMapColumnName,
						     Qt::Checked);
				pitem->setExpanded(true);
			}
			else
			{
				iitem->setCheckState(KMapColumnName,
						     Qt::Unchecked);
			}

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
			iitem->setFlags(iitem->flags() & ~Qt::ItemIsEnabled);
			iitem->setFlags(iitem->flags() & ~Qt::ItemIsSelectable);
			iitem->setCheckState(KMapColumnName, Qt::Unchecked);
		}
	}

	/* Enable check state change tracking after the tree has been filled */
	connect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
		this, SLOT(slotMapItemChanged(QTreeWidgetItem*)));
}

void InputPatchEditor::slotMapCurrentItemChanged(QTreeWidgetItem* item)
{
	QString info;

	if (item == NULL)
	{
		/* Nothing selected */
		info = tr("No information");
	}
	else
	{
		QString plugin;
		t_input input;

		if (item->parent() != NULL)
		{
			/* Input node selected */
			plugin = item->parent()->text(KMapColumnName);
			input = item->text(KMapColumnInput).toInt();
		}
		else
		{
			/* Plugin node selected */
			plugin = item->text(KMapColumnName);
			input = KInputInvalid;
		}

		info = _app->inputMap()->pluginStatus(plugin, input);
	}

	/* Display information for the selected plugin or input */
	m_infoBrowser->setText(info);
}

void InputPatchEditor::slotMapItemChanged(QTreeWidgetItem* item)
{
	if (item == NULL)
		return;

	if (item->checkState(KMapColumnName) == Qt::Checked)
	{
		/* Temporarily disable this signal to prevent an endless loop */
		disconnect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
			   this, SLOT(slotMapItemChanged(QTreeWidgetItem*)));

		/* Set all other items unchecked... */
		QTreeWidgetItemIterator it(m_mapTree);
		while ((*it) != NULL)
		{
			/* Don't touch the item that was just checked nor
			   any parent nodes. */
			if (*it != item && (*it)->childCount() == 0)
			{
				/* Set all the rest of the nodes unchecked */
				(*it)->setCheckState(KMapColumnName,
						     Qt::Unchecked);
			}

			/* Next one */
			++it;
		}

		/* Start listening to this signal once again */
		connect(m_mapTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
			this, SLOT(slotMapItemChanged(QTreeWidgetItem*)));
	}
	else
	{
		/* Don't allow unchecking an item by clicking it. Only allow
		   changing the check state by checking another item. */
		item->setCheckState(KMapColumnName, Qt::Checked);
	}

	/* Store the selected plugin name & input */
	if (item->parent() != NULL)
	{
		m_currentPluginName = item->parent()->text(KMapColumnName);
		m_currentInput = item->text(KMapColumnInput).toInt();
	}
	else
	{
		m_currentPluginName = KInputNone;
		m_currentInput = KInputInvalid;
	}

	/* Apply the patch immediately so that input data can be used in the
	   input device editor */
	_app->inputMap()->setPatch(m_universe, m_currentPluginName,
				   m_currentInput, m_currentDeviceName);
}

void InputPatchEditor::slotConfigureInputClicked()
{
	QTreeWidgetItem* item;
	QString plugin;

	/* Find out the currently selected plugin */
	item = m_mapTree->currentItem();
	if (item == NULL)
		return;
	else if (item->parent() != NULL)
		plugin = item->parent()->text(KMapColumnName);
	else
		plugin = item->text(KMapColumnName);

	/* Configure the plugin */
	_app->inputMap()->configurePlugin(plugin);

	/* Refill the mapping tree in case configuration changed something */
	fillMappingTree();
}

/****************************************************************************
 * Device tree
 ****************************************************************************/

void InputPatchEditor::setupDevicePage()
{
	connect(m_addDeviceButton, SIGNAL(clicked()),
		this, SLOT(slotAddDeviceClicked()));
	connect(m_removeDeviceButton, SIGNAL(clicked()),
		this, SLOT(slotRemoveDeviceClicked()));
	connect(m_editDeviceButton, SIGNAL(clicked()),
		this, SLOT(slotEditDeviceClicked()));

	/* Fill the device tree with available device names */
	fillDeviceTree();

	/* Listen to itemChanged() signals to catch check state changes */
	connect(m_deviceTree, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
		this, SLOT(slotDeviceItemChanged(QTreeWidgetItem*)));

	/* Double click acts as edit button click */
	connect(m_deviceTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(slotEditDeviceClicked()));
}

void InputPatchEditor::fillDeviceTree()
{
	QTreeWidgetItem* item;

	m_deviceTree->clear();

	/* Add an option for having no device at all */
	item = new QTreeWidgetItem(m_deviceTree);
	updateDeviceItem(KInputNone, item);

	/* Insert available input devices to the tree */
	QStringListIterator it(_app->inputMap()->deviceNames());
	while (it.hasNext() == true)
	{
		item = new QTreeWidgetItem(m_deviceTree);
		updateDeviceItem(it.next(), item);
	}
}

void InputPatchEditor::updateDeviceItem(const QString& name,
					QTreeWidgetItem* item)
{
	Q_ASSERT(item != NULL);

	item->setText(KDeviceColumnName, name);

	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	if (m_currentDeviceName == name)
		item->setCheckState(KDeviceColumnName, Qt::Checked);
	else
		item->setCheckState(KDeviceColumnName, Qt::Unchecked);
}

void InputPatchEditor::slotDeviceItemChanged(QTreeWidgetItem* item)
{
	if (item->checkState(KDeviceColumnName) == Qt::Checked)
	{
		/* Temporarily disable this signal to prevent an endless loop */
		disconnect(m_deviceTree,
			   SIGNAL(itemChanged(QTreeWidgetItem*,int)),
			   this,
			   SLOT(slotDeviceItemChanged(QTreeWidgetItem*)));

		/* Set all other items unchecked... */
		QTreeWidgetItemIterator it(m_deviceTree);
		while (*it != NULL)
		{
			/* ...except the one that was just checked */
			if (*it != item)
				(*it)->setCheckState(KDeviceColumnName,
						     Qt::Unchecked);
			++it;
		}

		/* Start listening to this signal once again */
		connect(m_deviceTree,
			SIGNAL(itemChanged(QTreeWidgetItem*,int)),
			this,
			SLOT(slotDeviceItemChanged(QTreeWidgetItem*)));
	}
	else
	{
		/* Don't allow unchecking an item by clicking it. Only allow
		   changing the check state by checking another item. */
		item->setCheckState(KDeviceColumnName, Qt::Checked);
	}

	/* Store the selected device name */
	m_currentDeviceName = item->text(KDeviceColumnName);

	/* Apply the patch immediately */
	_app->inputMap()->setPatch(m_universe, m_currentPluginName,
				   m_currentInput, m_currentDeviceName);
}

void InputPatchEditor::slotAddDeviceClicked()
{
	/* Create a new input device and start editing it */
	InputDeviceEditor ite(this, NULL);
edit:
	if (ite.exec() == QDialog::Accepted)
	{
		QLCInputDevice* dev;
		QString path;
		QDir dir;
		QString manufacturer;
		QString model;

		/* Remove spaces from these */
		manufacturer = ite.device()->manufacturer().remove(QChar(' '));
		model = ite.device()->model().remove(QChar(' '));

#ifdef Q_WS_X11
		/* If the current user is root, use the system device dir
		   for saving devices. Otherwise use the user's home dir.
		   This is done on Linux only, because Win32 & OSX save
		   system devices in a user-writable directory. */
		if (geteuid() == 0)
		{
			dir = QDir(INPUTDEVICEDIR);
		}
		else
		{
			path = QString("%1/%2").arg(getenv("HOME"))
					       .arg(USERINPUTDEVICEDIR);
			dir = QDir(path);
		}

		/* Ensure that the selected device directory exists */
		if (dir.exists() == false)
			dir.mkpath(".");
#else
		/* Use the system input device dir for Win32/OSX */
		dir = QDir(INPUTDEVICEDIR);
#endif
		/* Construct a descriptive file name for the device */
		path = QString("%1/%2-%3%4").arg(dir.absolutePath())
				.arg(manufacturer).arg(model)
				.arg(KExtInputDevice);

		/* Ensure that creating a new input device won't overwrite
		   an existing file. */
		if (QFile::exists(path + KExtInputDevice) == true)
		{
			for (int i = 1; i < INT_MAX; i++)
			{
				/* Start adding a number suffix to the file
				   name and stop when a unique file name is
				   found. */
				path = QString("%1/%2-%3-%4%5")
						.arg(dir.absolutePath())
						.arg(manufacturer).arg(model)
						.arg(i).arg(KExtInputDevice);

				if (QFile::exists(path) == false)
					break;
			}
		}

		/* Create a new non-const copy of the device and
		   reparent it to the input map */
		dev = new QLCInputDevice(*ite.device());

		/* Save it to a file, go back to edit if save failed */
		if (dev->saveXML(path) == false)
		{
			QMessageBox::warning(this, tr("Saving failed"),
				tr("Unable to save the device to %1")
				.arg(QDir::toNativeSeparators(path)));
			delete dev;
			goto edit;
		}
		else
		{
			/* Add the new device to input map */
			_app->inputMap()->addDevice(dev);

			/* Add the new device to our tree widget */
			QTreeWidgetItem* item;
			item = new QTreeWidgetItem(m_deviceTree);
			updateDeviceItem(dev->name(), item);
		}
	}
}

void InputPatchEditor::slotRemoveDeviceClicked()
{
	QLCInputDevice* device;
	QTreeWidgetItem* item;
	QString name;
	int r;

	/* Find out the currently selected item */
	item = m_deviceTree->currentItem();
	if (item == NULL)
		return;

	/* Get the currently selected device object by its name */
	name = item->text(KDeviceColumnName);
	device = _app->inputMap()->device(name);
	if (device == NULL)
		return;

	/* Ask for user confirmation */
	r = QMessageBox::question(this, tr("Delete device"),
		tr("Do you wish to permanently delete device \"%1\"?")
		.arg(device->name()),
		QMessageBox::Yes, QMessageBox::No);
	if (r == QMessageBox::Yes)
	{
		/* Attempt to delete the file first */
		QFile file(device->path());
		if (file.remove() == true)
		{
			if (item->checkState(KDeviceColumnName) == Qt::Checked)
			{
				/* The currently assigned device is removed,
				   so select "None" next. */
				QTreeWidgetItem* none;
				none = m_deviceTree->topLevelItem(0);
				Q_ASSERT(none != NULL);
				none->setCheckState(KDeviceColumnName,
						    Qt::Checked);
			}

			/* Successful deletion. Remove the device from
			   input map and our tree widget */
			_app->inputMap()->removeDevice(name);
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

void InputPatchEditor::slotEditDeviceClicked()
{
	QLCInputDevice* device;
	QTreeWidgetItem* item;
	QString name;

	/* Get the currently selected item and bail out if nothing or "None"
	   is selected */
	item = m_deviceTree->currentItem();
	if (item == NULL || item->text(KDeviceColumnName) == KInputNone)
		return;

	/* Get the currently selected device by its name */
	name = item->text(KDeviceColumnName);
	device = _app->inputMap()->device(name);
	if (device == NULL)
		return;

	/* Edit the device and update the item if OK was pressed */
	InputDeviceEditor ite(this, device);
edit:
	if (ite.exec() == QDialog::Rejected)
		return;

	QString path;

	/* Copy the channel's contents from the editor's copy to
	   the actual object (with QLCInputDevice::operator=()). */
	*device = *ite.device();

#ifdef Q_WS_X11
	/* If the current user is root, save the device to its old path.
	   Otherwise use the user's home dir and generate a new file name
	   if necessary. This is done on Linux only, because Win32 & OSX save
	   devices always in a user-writable directory. */
	if (geteuid() == 0)
	{
		path = device->path();
	}
	else
	{
		QString manufacturer;
		QString model;

		/* Remove spaces from these */
		manufacturer = device->manufacturer().remove(QChar(' '));
		model = device->model().remove(QChar(' '));

		/* Ensure that user device directory exists */
		path = QString("%1/%2").arg(getenv("HOME"))
				       .arg(USERINPUTDEVICEDIR);
		QDir dir = QDir(path);
		if (dir.exists() == false)
			dir.mkpath(".");

		/* Check, whether the device was originally saved
		   in the system directory. If it is, construct a
		   new name for it into the user's device dir. */
		path = device->path();
		if (path.contains(getenv("HOME")) == false)
		{
			/* Construct a descriptive file name for
			   the device under user's HOME dir */
			path = QString("%1/%2-%3%4").arg(dir.absolutePath())
				.arg(manufacturer).arg(model)
				.arg(KExtInputDevice);
		}
	}
#else
	/* Win32 & OSX save input devices in a user-writable directory,
	   so we can use that directly. */
	path = device->path();
#endif
	/* Save the device */
	if (device->saveXML(path) == true)
	{
		/* Get the device's name from the device itself
		   since it may have changed making local variable
		   "name" invalid */
		updateDeviceItem(device->name(), item);
	}
	else
	{
		QMessageBox::warning(this, tr("Saving failed"),
			tr("Unable to save %1 to %2")
			.arg(device->name())
			.arg(QDir::toNativeSeparators(path)));
		goto edit;
	}
}
