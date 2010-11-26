/*
  Q Light Controller
  vcframe.cpp

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

#include <QStyleOptionFrameV2>
#include <QMetaObject>
#include <QMessageBox>
#include <QPainter>
#include <QAction>
#include <QStyle>
#include <QDebug>
#include <QPoint>
#include <QSize>
#include <QMenu>
#include <QList>
#include <QtXml>

#include "qlcfile.h"

#include "virtualconsole.h"
#include "vccuelist.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "vcframe.h"
#include "vclabel.h"
#include "vcsoloframe.h"
#include "vcxypad.h"
#include "app.h"
#include "doc.h"

extern App* _app;

VCFrame::VCFrame(QWidget* parent) : VCWidget(parent)
{
    /* Set the class name "VCFrame" as the object name as well */
    setObjectName(VCFrame::staticMetaObject.className());

    m_frameStyle = KVCFrameStyleSunken;
}

VCFrame::~VCFrame()
{
}

bool VCFrame::isBottomFrame()
{
    return (parentWidget() != NULL && qobject_cast<VCFrame*>(parentWidget()) == NULL);
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCFrame::createCopy(VCWidget* parent)
{
    Q_ASSERT(parent != NULL);

    VCFrame* frame = new VCFrame(parent);
    if (frame->copyFrom(this) == false)
    {
        delete frame;
        frame = NULL;
    }

    return frame;
}

bool VCFrame::copyFrom(VCWidget* widget)
{
    VCFrame* frame = qobject_cast<VCFrame*> (widget);
    if (frame == NULL)
        return false;

    /* TODO: Copy children? */

    /* Copy common stuff */
    return VCWidget::copyFrom(widget);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCFrame::loader(const QDomElement* root, QWidget* parent)
{
    VCFrame* frame = NULL;

    Q_ASSERT(root != NULL);
    Q_ASSERT(parent != NULL);

    if (root->tagName() != KXMLQLCVCFrame)
    {
        qWarning() << Q_FUNC_INFO << "Frame node not found";
        return false;
    }

    /* Create a new frame into its parent */
    frame = new VCFrame(parent);
    frame->show();

    /* Continue loading */
    return frame->loadXML(root);
}

bool VCFrame::loadXML(const QDomElement* root)
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

    if (root->tagName() != xmlTagName())
    {
        qWarning() << Q_FUNC_INFO << "Frame node not found";
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
        else if (tag.tagName() == KXMLQLCVCFrame)
        {
            VCFrame::loader(&tag, this);
        }
        else if (tag.tagName() == KXMLQLCVCLabel)
        {
            VCLabel::loader(&tag, this);
        }
        else if (tag.tagName() == KXMLQLCVCButton)
        {
            VCButton::loader(&tag, this);
        }
        else if (tag.tagName() == KXMLQLCVCXYPad)
        {
            VCXYPad::loader(&tag, this);
        }
        else if (tag.tagName() == KXMLQLCVCSlider)
        {
            VCSlider::loader(&tag, this);
        }
        else if (tag.tagName() == KXMLQLCVCSoloFrame)
        {
            VCSoloFrame::loader(&tag, this);
        }
        else if (tag.tagName() == KXMLQLCVCCueList)
        {
            VCCueList::loader(&tag, this);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown frame tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

bool VCFrame::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(vc_root != NULL);

    /* VC Frame entry */
    root = doc->createElement(xmlTagName());
    vc_root->appendChild(root);

    /* Caption */
    root.setAttribute(KXMLQLCVCCaption, caption());

    /* Save appearance */
    saveXMLAppearance(doc, &root);

    /* Save widget proportions only for child frames */
    if (isBottomFrame() == false)
        saveXMLWindowState(doc, &root);

    /* Save children */
    QListIterator <VCWidget*> it(findChildren<VCWidget*>());
    while (it.hasNext() == true)
    {
        VCWidget* widget = it.next();

        /* findChildren() is recursive, so the list contains all
           possible child widgets below this frame. Each frame must
           save only its direct children to preserve hierarchy, so
           save only such widgets that have this widget as their
           direct parent. */
        if (widget->parentWidget() == this)
            widget->saveXML(doc, &root);
    }

    return true;
}

/*****************************************************************************
 * Custom menu
 *****************************************************************************/

QMenu* VCFrame::customMenu(QMenu* parentMenu)
{
    /* No point coming here if there is no VC */
    VirtualConsole* vc = VirtualConsole::instance();
    if (vc == NULL)
        return NULL;

    /* Basically, just returning VC::addMenu() would suffice here, but
       since the returned menu will be deleted when the current widget
       changes, we have to copy the menu's contents into a new menu. */
    QMenu* menu = new QMenu(parentMenu);
    menu->setTitle(tr("Add"));
    QListIterator <QAction*> it(vc->addMenu()->actions());
    while (it.hasNext() == true)
        menu->addAction(it.next());
    return menu;
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void VCFrame::handleWidgetSelection(QMouseEvent* e)
{
    /* No point coming here if there is no VC */
    VirtualConsole* vc = VirtualConsole::instance();
    if (vc == NULL)
        return;

    /* Don't allow selection of the bottom frame. Selecting it will always
       actually clear the current selection. */
    if (isBottomFrame() == false)
        VCWidget::handleWidgetSelection(e);
    else
        vc->clearWidgetSelection();
}

void VCFrame::mouseMoveEvent(QMouseEvent* e)
{
    if (isBottomFrame() == false)
        VCWidget::mouseMoveEvent(e);
    else
        QWidget::mouseMoveEvent(e);
}
