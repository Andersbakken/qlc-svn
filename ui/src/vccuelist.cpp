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
#include "inputmap.h"
#include "app.h"
#include "doc.h"

#define HYSTERESIS 3 // Hysteresis for next/previous external input

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
    connect(_app->doc(), SIGNAL(functionChanged(t_function_id)),
            this, SLOT(slotFunctionChanged(t_function_id)));

    setNextInputSource(InputMap::invalidUniverse(), KInputChannelInvalid);
    setPreviousInputSource(InputMap::invalidUniverse(), KInputChannelInvalid);
    m_nextLatestValue = 0;
    m_previousLatestValue = 0;
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

    /* Function list contents */
    m_list->clear();
    for (int i = 0; i < cuelist->m_list->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item = cuelist->m_list->topLevelItem(i);
        append(item->text(KVCCueListColumnID).toInt());
    }

    /* Key sequence */
    setNextKeySequence(cuelist->nextKeySequence());
    setPreviousKeySequence(cuelist->previousKeySequence());

    /* Input source */
    setNextInputSource(cuelist->nextInputUniverse(), cuelist->nextInputChannel());
    setPreviousInputSource(cuelist->previousInputUniverse(), cuelist->previousInputChannel());

    /* Common stuff */
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

void VCCueList::slotFunctionChanged(t_function_id fid)
{
	QTreeWidgetItem* item;
	Function* function;

    function = _app->doc()->function(fid);

    /* Find all items matching the changed function ID and update them */
    for (int i = 0; i < m_list->topLevelItemCount(); i++)
    {
        item = m_list->topLevelItem(i);
        Q_ASSERT(item != NULL);

        if (item->text(KVCCueListColumnID).toInt() == fid)
        {
            item->setText(KVCCueListColumnName, function->name());
        }
    }
}

void VCCueList::slotNextCue()
{
    if (mode() != Doc::Operate)
        return;

    if (m_list->currentItem() == NULL)
    {
        int count = m_list->topLevelItemCount();
        if (count == 0)
            return;
        m_list->setCurrentItem(m_list->topLevelItem(0));
    }
    else
    {
        m_list->setCurrentItem(m_list->itemBelow(m_list->currentItem()));
    }

    slotItemActivated(m_list->currentItem());
}

void VCCueList::slotPreviousCue()
{
    if (mode() != Doc::Operate)
        return;

    if (m_list->currentItem() == NULL)
    {
        int count = m_list->topLevelItemCount();
        if (count == 0)
            return;
        m_list->setCurrentItem(m_list->topLevelItem(count - 1));
    }
    else
    {
        m_list->setCurrentItem(m_list->itemAbove(m_list->currentItem()));
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

    //! @todo not sure how a cuelist should behave in a solopanel...
    _app->masterTimer()->startFunction(m_current, true);
}

/*****************************************************************************
 * Key Sequences
 *****************************************************************************/

void VCCueList::setNextKeySequence(const QKeySequence& keySequence)
{
    m_nextKeySequence = QKeySequence(keySequence);
}

void VCCueList::setPreviousKeySequence(const QKeySequence& keySequence)
{
    m_previousKeySequence = QKeySequence(keySequence);
}

void VCCueList::slotKeyPressed(const QKeySequence& keySequence)
{
    if (m_nextKeySequence == keySequence)
        slotNextCue();
    else if (m_previousKeySequence == keySequence)
        slotPreviousCue();
}

/*****************************************************************************
 * External Input
 *****************************************************************************/

void VCCueList::setNextInputSource(quint32 uni, quint32 ch)
{
    disconnect(_app->inputMap(), SIGNAL(inputValueChanged(quint32, quint32, uchar)),
               this, SLOT(slotNextInputValueChanged(quint32, quint32, uchar)));
    m_nextInputUniverse = uni;
    m_nextInputChannel = ch;
    if (uni != InputMap::invalidUniverse() && ch != KInputChannelInvalid)
        connect(_app->inputMap(), SIGNAL(inputValueChanged(quint32, quint32, uchar)),
                this, SLOT(slotNextInputValueChanged(quint32, quint32, uchar)));
}

void VCCueList::setPreviousInputSource(quint32 uni, quint32 ch)
{
    disconnect(_app->inputMap(), SIGNAL(inputValueChanged(quint32, quint32, uchar)),
               this, SLOT(slotPreviousInputValueChanged(quint32, quint32, uchar)));
    m_previousInputUniverse = uni;
    m_previousInputChannel = ch;
    if (uni != InputMap::invalidUniverse() && ch != KInputChannelInvalid)
        connect(_app->inputMap(), SIGNAL(inputValueChanged(quint32, quint32, uchar)),
                this, SLOT(slotPreviousInputValueChanged(quint32, quint32, uchar)));
}

void VCCueList::slotNextInputValueChanged(quint32 universe, quint32 channel,
                                          uchar value)
{
    if (universe == m_nextInputUniverse && channel == m_nextInputChannel)
    {
        // Use hysteresis for values, in case the cue list is being controlled
        // by a slider. The value has to go to zero before the next non-zero
        // value is accepted as input. And the non-zero values have to visit
        // above $HYSTERESIS before a zero is accepted again.
        if (m_nextLatestValue == 0 && value > 0)
        {
            slotNextCue();
            m_nextLatestValue = value;
        }
        else if (m_nextLatestValue > HYSTERESIS && value == 0)
        {
            m_nextLatestValue = 0;
        }

        if (value > HYSTERESIS)
            m_nextLatestValue = value;
    }
}

void VCCueList::slotPreviousInputValueChanged(quint32 universe, quint32 channel,
                                              uchar value)
{
    if (universe == m_previousInputUniverse && channel == m_previousInputChannel)
    {
        // Use hysteresis for values, in case the cue list is being controlled
        // by a slider. The value has to go to zero before the next non-zero
        // value is accepted as input. And the non-zero values have to visit
        // above $HYSTERESIS before a zero is accepted again.
        if (m_previousLatestValue == 0 && value > 0)
        {
            slotPreviousCue();
            m_previousLatestValue = value;
        }
        else if (m_previousLatestValue > HYSTERESIS && value == 0)
        {
            m_previousLatestValue = 0;
        }

        if (value > HYSTERESIS)
            m_previousLatestValue = value;
    }
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
        qWarning() << Q_FUNC_INFO << "CueList node not found";
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
        qWarning() << Q_FUNC_INFO << "CueList node not found";
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
        else if (tag.tagName() == KXMLQLCVCCueListNext)
        {
            QDomNode subNode = tag.firstChild();
            while (subNode.isNull() == false)
            {
                QDomElement subTag = subNode.toElement();
                if (subTag.tagName() == KXMLQLCVCWidgetInput)
                {
                    quint32 uni = 0;
                    quint32 ch = 0;
                    if (loadXMLInput(subTag, &uni, &ch) == true)
                        setNextInputSource(uni, ch);
                }
                else if (subTag.tagName() == KXMLQLCVCCueListKey)
                {
                    m_nextKeySequence = QKeySequence(subTag.text());
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown CueList Next tag" << subTag.tagName();
                }

                subNode = subNode.nextSibling();
            }
        }
        else if (tag.tagName() == KXMLQLCVCCueListPrevious)
        {
            QDomNode subNode = tag.firstChild();
            while (subNode.isNull() == false)
            {
                QDomElement subTag = subNode.toElement();
                if (subTag.tagName() == KXMLQLCVCWidgetInput)
                {
                    quint32 uni = 0;
                    quint32 ch = 0;
                    if (loadXMLInput(subTag, &uni, &ch) == true)
                        setPreviousInputSource(uni, ch);
                }
                else if (subTag.tagName() == KXMLQLCVCCueListKey)
                {
                    m_previousKeySequence = QKeySequence(subTag.text());
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Unknown CueList Previous tag" << subTag.tagName();
                }

                subNode = subNode.nextSibling();
            }
        }
        else if (tag.tagName() == KXMLQLCVCCueListKey) /* Legacy */
        {
            setNextKeySequence(QKeySequence(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCVCCueListFunction)
        {
            append(tag.text().toInt());
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown cuelist tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool VCCueList::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
    QDomElement root;
    QDomElement tag;
    QDomElement subtag;
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

    /* Next cue */
    tag = doc->createElement(KXMLQLCVCCueListNext);
    root.appendChild(tag);
    subtag = doc->createElement(KXMLQLCVCCueListKey);
    tag.appendChild(subtag);
    text = doc->createTextNode(m_nextKeySequence.toString());
    subtag.appendChild(text);
    saveXMLInput(doc, &tag, nextInputUniverse(), nextInputChannel());

    /* Previous cue */
    tag = doc->createElement(KXMLQLCVCCueListPrevious);
    root.appendChild(tag);
    subtag = doc->createElement(KXMLQLCVCCueListKey);
    tag.appendChild(subtag);
    text = doc->createTextNode(m_previousKeySequence.toString());
    subtag.appendChild(text);
    saveXMLInput(doc, &tag, previousInputUniverse(), previousInputChannel());

    /* Window state */
    saveXMLWindowState(doc, &root);

    /* Appearance */
    saveXMLAppearance(doc, &root);

    return true;
}
