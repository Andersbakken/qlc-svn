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
#include <QDebug>

#include "addvcbuttonmatrix.h"
#include "functionselection.h"
#include "vcbutton.h"
#include "function.h"
#include "app.h"
#include "doc.h"

extern App* _app;

#define KColumnFunction 0
#define KColumnType     1

#define HORIZONTAL_COUNT "addvcbuttonmatrix/horizontalcount"
#define VERTICAL_COUNT "addvcbuttonmatrix/verticalcount"
#define BUTTON_SIZE "addvcbuttonmatrix/buttonsize"
#define FRAME_STYLE "addvcbuttonmatrix/framestyle"

AddVCButtonMatrix::AddVCButtonMatrix(QWidget* parent) : QDialog(parent)
{
    QSettings settings;
    QVariant var;

    setupUi(this);

    var = settings.value(HORIZONTAL_COUNT);
    if (var.isValid() == true)
        m_horizontalSpin->setValue(var.toInt());
    else
        m_horizontalSpin->setValue(5);
    m_horizontalCount = m_horizontalSpin->value();

    var = settings.value(VERTICAL_COUNT);
    if (var.isValid() == true)
        m_verticalSpin->setValue(var.toInt());
    else
        m_verticalSpin->setValue(5);
    m_verticalCount = m_verticalSpin->value();

    var = settings.value(BUTTON_SIZE);
    if (var.isValid() == true)
        m_sizeSpin->setValue(var.toInt());
    else
        m_sizeSpin->setValue(VCButton::defaultSize.width());
    m_buttonSize = m_sizeSpin->value();

    var = settings.value(FRAME_STYLE);
    if (var.isValid() == true)
        setFrameStyle(AddVCButtonMatrix::FrameStyle(var.toInt()));
    else
        setFrameStyle(AddVCButtonMatrix::NormalFrame);

    setAllocationText();
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

    setAllocationText();
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

    setAllocationText();
}

void AddVCButtonMatrix::slotHorizontalChanged()
{
    m_horizontalCount = m_horizontalSpin->value();
    setAllocationText();
}

void AddVCButtonMatrix::slotVerticalChanged()
{
    m_verticalCount = m_verticalSpin->value();
    setAllocationText();
}

void AddVCButtonMatrix::slotButtonSizeChanged()
{
    m_buttonSize = m_sizeSpin->value();
}

void AddVCButtonMatrix::slotNormalFrameToggled(bool toggled)
{
    if (toggled == true)
        setFrameStyle(AddVCButtonMatrix::NormalFrame);
    else
        setFrameStyle(AddVCButtonMatrix::SoloFrame);
}

void AddVCButtonMatrix::accept()
{
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

void AddVCButtonMatrix::setAllocationText()
{
    QString text("%1 / %2");
    m_allocationEdit->setText(text.arg(m_tree->topLevelItemCount())
                              .arg(horizontalCount() * verticalCount()));
}

void AddVCButtonMatrix::setFrameStyle(AddVCButtonMatrix::FrameStyle style)
{
    switch (style)
    {
    default:
    case NormalFrame:
        m_frameNormalRadio->setChecked(true);
        m_frameStyle = NormalFrame;
        break;
    case SoloFrame:
        m_frameSoloRadio->setChecked(true);
        m_frameStyle = SoloFrame;
        break;
    }
}
