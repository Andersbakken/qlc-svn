/*
  Q Light Controller
  collectioneditor.cpp

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
#include <QTreeWidget>
#include <QSettings>
#include <QLineEdit>
#include <QLabel>

#include "qlcfixturedef.h"

#include "functionselection.h"
#include "collectioneditor.h"
#include "collection.h"
#include "function.h"
#include "fixture.h"
#include "apputil.h"
#include "app.h"
#include "doc.h"

extern App* _app;

#define SETTINGS_GEOMETRY "collectioneditor/geometry"

#define KColumnFunction 0
#define KColumnFunctionID 1

CollectionEditor::CollectionEditor(QWidget* parent, Collection* fc)
        : QDialog(parent)
{
    Q_ASSERT(fc != NULL);
    m_original = fc;

    setupUi(this);

    connect(m_nameEdit, SIGNAL(textEdited(const QString&)),
            this, SLOT(slotNameEdited(const QString&)));
    connect(m_add, SIGNAL(clicked()), this, SLOT(slotAdd()));
    connect(m_remove, SIGNAL(clicked()), this, SLOT(slotRemove()));

    m_fc = new Collection(_app->doc());
    m_fc->copyFrom(fc);
    Q_ASSERT(m_fc != NULL);

    m_nameEdit->setText(m_fc->name());
    slotNameEdited(m_fc->name());

    updateFunctionList();

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
    AppUtil::ensureWidgetIsVisible(this);
}

CollectionEditor::~CollectionEditor()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());

    Q_ASSERT(m_fc != NULL);
    delete m_fc;
    m_fc = NULL;
}

void CollectionEditor::slotNameEdited(const QString& text)
{
    setWindowTitle(tr("Collection - %1").arg(text));
}

void CollectionEditor::slotAdd()
{
    FunctionSelection sel(this, true, m_original->id());
    if (sel.exec() == QDialog::Accepted)
    {
        t_function_id fid;

        QListIterator <t_function_id> it(sel.selection());
        while (it.hasNext() == true)
        {
            fid = it.next();
            m_fc->addFunction(fid);
        }

        updateFunctionList();
    }
}

void CollectionEditor::slotRemove()
{
    QTreeWidgetItem* item = m_tree->currentItem();
    if (item != NULL)
    {
        t_function_id id = item->text(KColumnFunctionID).toInt();
        m_fc->removeFunction(id);
        delete item;
    }
}

void CollectionEditor::accept()
{
    m_fc->setName(m_nameEdit->text());
    m_original->copyFrom(m_fc);
    _app->doc()->setModified();

    QDialog::accept();
}

void CollectionEditor::updateFunctionList()
{
    m_tree->clear();

    QListIterator <t_function_id> it(m_fc->functions());
    while (it.hasNext() == true)
    {
        QTreeWidgetItem* item;
        Function* function;
        t_function_id fid;
        QString s;

        fid = it.next();
        function = _app->doc()->function(fid);
        Q_ASSERT(function != NULL);

        item = new QTreeWidgetItem(m_tree);
        item->setText(KColumnFunction, function->name());
        item->setText(KColumnFunctionID, s.setNum(fid));
    }
}
