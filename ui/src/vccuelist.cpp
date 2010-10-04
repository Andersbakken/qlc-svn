/*
  Q Light Controller
  vccuelist.cpp

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
#include <QHeaderView>
#include <QVBoxLayout>
#include <QString>
#include <QDebug>
#include <QtXml>

#include "qlcfile.h"

#include "vccuelistproperties.h"
#include "virtualconsole.h"
#include "mastertimer.h"
#include "vccuelist.h"
#include "function.h"
#include "app.h"
#include "doc.h"

extern App* _app;

VCCueList::VCCueList(QWidget* parent) : VCWidget(parent)
{
    /* Set the class name "VCCueList" as the object name as well */
    setObjectName(VCCueList::staticMetaObject.className());

    /* Create a layout for this widget */
    new QVBoxLayout(this);

    /* Create a list for scenes (cues) */
    m_list = new QTreeWidget(this);
    layout()->addWidget(m_list);
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setAlternatingRowColors(true);
    m_list->setAllColumnsShowFocus(true);
    m_list->setRootIsDecorated(false);
    m_list->setItemsExpandable(false);
    m_list->header()->setSortIndicatorShown(false);
    m_list->header()->setClickable(false);
    m_list->header()->setMovable(false);
    m_list->header()->setResizeMode(QHeaderView::ResizeToContents);

    connect(m_list, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(slotItemActivated(QTreeWidgetItem*)));

    setFrameStyle(KVCFrameStyleSunken);
    setCaption(tr("Cue list"));
    resize(QSize(200, 200));

    m_current = NULL;

    slotModeChanged(mode());

    connect(_app->doc(), SIGNAL(functionRemoved(t_function_id)),
            this, SLOT(slotFunctionRemoved(t_function_id)));
}

VCCueList::~VCCueList()
{
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCCueList::createCopy(VCWidget* parent)
{
    Q_ASSERT(parent != NULL);

    VCCueList* cuelist = new VCCueList(parent);
    if (cuelist->copyFrom(this) == false)
    {
        delete cuelist;
        cuelist = NULL;
    }

    return cuelist;
}

bool VCCueList::copyFrom(VCWidget* widget)
{
    VCCueList* cuelist = qobject_cast<VCCueList*> (widget);
    if (cuelist == NULL)
        return false;

    /* Copy function list contents */
    m_list->clear();
    for (int i = 0; i < cuelist->m_list->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = cuelist->m_list->topLevelItem(i);
        append(item->text(KVCCueListColumnID).toInt());
    }

    /* Copy key sequence */
    setKeySequence(cuelist->keySequence());

    /* Copy common stuff */
    return VCWidget::copyFrom(widget);
}

/*****************************************************************************
 * Cue list
 *****************************************************************************/

void VCCueList::clear()
{
    m_list->clear();
}

void VCCueList::append(t_function_id fid)
{
    QTreeWidgetItem* item;
    Function* function;

    function = _app->doc()->function(fid);
    Q_ASSERT(function != NULL);

    item = new QTreeWidgetItem(m_list);
    item->setText(KVCCueListColumnNumber,
                  QString("%1").arg(m_list->indexOfTopLevelItem(item) + 1));
    item->setText(KVCCueListColumnName, function->name());
    item->setText(KVCCueListColumnID, QString("%1").arg(fid));
}

void VCCueList::slotFunctionRemoved(t_function_id fid)
{
    /* Find all items matching the destroyed function ID and remove them */
    for (int i = 0; i < m_list->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = m_list->topLevelItem(i);
        Q_ASSERT(item != NULL);

        if (item->text(KVCCueListColumnID).toInt() == fid)
            delete item;
    }
}

void VCCueList::slotNextCue()
{
    if (mode() != Doc::Operate)
        return;

    if (m_list->currentItem() == NULL)
    {
        m_list->setCurrentItem(m_list->topLevelItem(0));
    }
    else
    {
        m_list->setCurrentItem(
            m_list->itemBelow(m_list->currentItem()));
    }

    slotItemActivated(m_list->currentItem());
}

void VCCueList::slotFunctionStopped(t_function_id fid)
{
    if (m_current != NULL && m_current->id() == fid)
    {
        disconnect(m_current, SIGNAL(stopped(t_function_id)),
                   this, SLOT(slotFunctionStopped(t_function_id)));
    }
}

void VCCueList::slotItemActivated(QTreeWidgetItem* item)
{
    if (mode() != Doc::Operate)
        return;

    if (m_current != NULL)
        m_current->stop();

    if (item == NULL)
        return;

    m_current = _app->doc()->function(item->text(KVCCueListColumnID).toInt());
    if (m_current == NULL)
        return;

    connect(m_current, SIGNAL(stopped(t_function_id)),
            this, SLOT(slotFunctionStopped(t_function_id)));

    _app->masterTimer()->startFunction(m_current);
}

/*****************************************************************************
 * Key sequence handler
 *****************************************************************************/

void VCCueList::setKeySequence(const QKeySequence& keySequence)
{
    m_keySequence = QKeySequence(keySequence);
}

void VCCueList::slotKeyPressed(const QKeySequence& keySequence)
{
    if (m_keySequence == keySequence)
        slotNextCue();
}

/*****************************************************************************
 * Caption
 *****************************************************************************/

void VCCueList::setCaption(const QString& text)
{
    QStringList list;

    VCWidget::setCaption(text);
    m_list->setHeaderLabels(list << tr("Number") << text);
}

/*****************************************************************************
 * QLC Mode
 *****************************************************************************/

void VCCueList::slotModeChanged(Doc::Mode mode)
{
    if (mode == Doc::Operate)
        m_list->setEnabled(true);
    else
        m_list->setEnabled(false);

    /* Always start from the beginning */
    m_list->setCurrentItem(NULL);
    m_current = NULL;

    VCWidget::slotModeChanged(mode);
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCCueList::editProperties()
{
    VCCueListProperties prop(_app, this);
    if (prop.exec() == QDialog::Accepted)
        _app->doc()->setModified();
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCCueList::loader(const QDomElement* root, QWidget* parent)
{
    VCCueList* cuelist = NULL;

    Q_ASSERT(root != NULL);
    Q_ASSERT(parent != NULL);

    if (root->tagName() != KXMLQLCVCCueList)
    {
        qDebug() << "CueList node not found!";
        return false;
    }

    /* Create a new cuelist into its parent */
    cuelist = new VCCueList(parent);
    cuelist->show();

    /* Continue loading */
    return cuelist->loadXML(root);
}

bool VCCueList::loadXML(const QDomElement* root)
{
    bool visible = false;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    QDomNode node;
    QDomElement tag;
    QString str;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCCueList)
    {
        qDebug() << "CueList node not found!";
        return false;
    }

    /* Caption */
    setCaption(root->attribute(KXMLQLCVCCaption));

    /* Children */
    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();
        if (tag.tagName() == KXMLQLCWindowState)
        {
            loadXMLWindowState(&tag, &x, &y, &w, &h, &visible);
            setGeometry(x, y, w, h);
        }
        else if (tag.tagName() == KXMLQLCVCAppearance)
        {
            loadXMLAppearance(&tag);
        }
        else if (tag.tagName() == KXMLQLCVCCueListKey)
        {
            setKeySequence(QKeySequence(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCVCCueListFunction)
        {
            append(tag.text().toInt());
        }
        else if (tag.tagName() == "KeyBind") /* Legacy */
        {
            loadKeyBind(&tag);
        }
        else
        {
            qDebug() << "Unknown cuelist tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool VCCueList::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(vc_root != NULL);

    /* VC CueList entry */
    root = doc->createElement(KXMLQLCVCCueList);
    vc_root->appendChild(root);

    /* Caption */
    root.setAttribute(KXMLQLCVCCaption, caption());

    /* Cues */
    QTreeWidgetItemIterator it(m_list);
    while (*it != NULL)
    {
        tag = doc->createElement(KXMLQLCVCCueListFunction);
        root.appendChild(tag);

        text = doc->createTextNode((*it)->text(KVCCueListColumnID));
        tag.appendChild(text);

        ++it;
    }

    /* Key sequence */
    if (m_keySequence.isEmpty() == false)
    {
        tag = doc->createElement(KXMLQLCVCCueListKey);
        root.appendChild(tag);
        text = doc->createTextNode(m_keySequence.toString());
        tag.appendChild(text);
    }

    /* Window state */
    saveXMLWindowState(doc, &root);

    /* Appearance */
    saveXMLAppearance(doc, &root);

    return true;
}

bool VCCueList::loadKeyBind(const QDomElement* key_root)
{
    QDomElement tag;
    QDomNode node;

    if (key_root->tagName() != "KeyBind")
    {
        qWarning() << "Not a key bind node!";
        return false;
    }

    node = key_root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();
        if (tag.tagName() == "Key")
        {
            int mod = tag.attribute("Modifier").toInt();
            int key = tag.text().toUInt();

            if (key < Qt::Key_unknown)
                setKeySequence(QKeySequence(key | mod));
        }
        else
        {
            qWarning() << "Unknown key binding tag:"
            << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}
