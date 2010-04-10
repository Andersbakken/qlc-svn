/*
  Q Light Controller
  addvcbuttonmatrix.cpp

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

#include <QSettings>

#include "functionselection.h"
#include "addvcbuttonmatrix.h"
#include "function.h"
#include "app.h"
#include "doc.h"

extern App* _app;

#define KColumnFunction 0
#define KColumnType     1

#define HORIZONTAL_COUNT "addvcbuttonmatrix/horizontalcount"
#define VERTICAL_COUNT "addvcbuttonmatrix/verticalcount"
#define BUTTON_SIZE "addvcbuttonmatrix/buttonsize"

AddVCButtonMatrix::AddVCButtonMatrix(QWidget* parent) : QDialog(parent)
{
	QSettings settings;
	QVariant var;

	setupUi(this);

	var = settings.value(HORIZONTAL_COUNT, 5);
	if (var.isValid() == true)
		m_horizontalSpin->setValue(var.toInt());

	var = settings.value(VERTICAL_COUNT, 5);
	if (var.isValid() == true)
		m_verticalSpin->setValue(var.toInt());

	var = settings.value(BUTTON_SIZE, 50);
	if (var.isValid() == true)
		m_sizeSpin->setValue(var.toInt());
}

AddVCButtonMatrix::~AddVCButtonMatrix()
{
	QSettings settings;
	settings.setValue(HORIZONTAL_COUNT, horizontalCount());
	settings.setValue(VERTICAL_COUNT, verticalCount());
	settings.setValue(BUTTON_SIZE, buttonSize());
}

void AddVCButtonMatrix::slotAddClicked()
{
	FunctionSelection fs(this, true);
	fs.setDisabledFunctions(functions());
	if (fs.exec() == true)
	{
		QListIterator <t_function_id> it(fs.selection());
		while (it.hasNext() == true)
			addFunction(it.next());
	}
}

void AddVCButtonMatrix::slotRemoveClicked()
{
	QListIterator <QTreeWidgetItem*> it(m_tree->selectedItems());
	while (it.hasNext() == true)
	{
		QTreeWidgetItem* item(it.next());
		m_functions.removeAll(
			item->data(KColumnFunction, Qt::UserRole).toInt());
		delete item;
	}
}

void AddVCButtonMatrix::accept()
{
	m_horizontalCount = m_horizontalSpin->value();
	m_verticalCount = m_verticalSpin->value();
	m_buttonSize = m_sizeSpin->value();

	QDialog::accept();
}

void AddVCButtonMatrix::addFunction(t_function_id fid)
{
	Function* function = _app->doc()->function(fid);
	if (function == NULL)
		return;

	QTreeWidgetItem* item = new QTreeWidgetItem(m_tree);
	item->setText(KColumnFunction, function->name());
	item->setText(KColumnType, function->typeString());
	item->setData(KColumnFunction, Qt::UserRole, fid);

	m_functions << fid;
}
