/*
  Q Light Controller
  vcxypad.cpp

  Copyright (c) Heikki Junnila, Stefan Krumm

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
#include <QMouseEvent>
#include <QMessageBox>
#include <QByteArray>
#include <QPainter>
#include <QPixmap>
#include <QCursor>
#include <QDebug>
#include <QPoint>
#include <QMenu>
#include <QList>
#include <QtXml>

#include "qlcfile.h"

#include "vcxypadproperties.h"
#include "virtualconsole.h"
#include "mastertimer.h"
#include "vcxypad.h"
#include "fixture.h"
#include "app.h"
#include "doc.h"

extern App* _app;

/*****************************************************************************
 * VCXYPad Initialization
 *****************************************************************************/

VCXYPad::VCXYPad(QWidget* parent) : VCWidget(parent)
{
    /* Set the class name "VCXYPad" as the object name as well */
    setObjectName(VCXYPad::staticMetaObject.className());

    setFrameStyle(KVCFrameStyleSunken);
    setCaption("XY Pad");
    setMinimumSize(20, 20);

    resize(QSize(120, 120));

    m_xyPosPixmap = QPixmap(":/xypad-point.png");

    /* Set initial position to center */
    m_currentXYPosition.setX(width() / 2);
    m_currentXYPosition.setY(height() / 2);
    m_currentXYPositionChanged = false;
}

VCXYPad::~VCXYPad()
{
}

/*****************************************************************************
 * Clipboard
 *****************************************************************************/

VCWidget* VCXYPad::createCopy(VCWidget* parent)
{
    Q_ASSERT(parent != NULL);

    VCXYPad* xypad = new VCXYPad(parent);
    if (xypad->copyFrom(this) == false)
    {
        delete xypad;
        xypad = NULL;
    }

    return xypad;
}

bool VCXYPad::copyFrom(VCWidget* widget)
{
    VCXYPad* xypad = qobject_cast <VCXYPad*> (widget);
    if (xypad == NULL)
        return false;

    /* Get rid of existing channels */
    m_fixtures.clear();

    /* Copy the other widget's fixtures */
    m_fixtures = xypad->fixtures();

    /* Copy the current position */
    setCurrentXYPosition(xypad->currentXYPosition());

    /* Copy common stuff */
    return VCWidget::copyFrom(widget);
}

/*****************************************************************************
 * Properties
 *****************************************************************************/

void VCXYPad::editProperties()
{
    VCXYPadProperties prop(_app, this);
    if (prop.exec() == QDialog::Accepted)
        _app->doc()->setModified();
}

/*****************************************************************************
 * Fixtures
 *****************************************************************************/

void VCXYPad::appendFixture(const VCXYPadFixture& fxi)
{
    if (m_fixtures.indexOf(fxi) == -1)
        m_fixtures.append(fxi);
}

void VCXYPad::removeFixture(t_fixture_id fxi_id)
{
    VCXYPadFixture fixture;
    fixture.setFixture(fxi_id);

    m_fixtures.removeAll(fixture);
}

void VCXYPad::clearFixtures()
{
    m_fixtures.clear();
}

/*****************************************************************************
 * Current XY position
 *****************************************************************************/

void VCXYPad::setCurrentXYPosition(const QPoint& point)
{
    m_currentXYPositionMutex.lock();
    m_currentXYPosition = point;
    m_currentXYPositionChanged = true;
    m_currentXYPositionMutex.unlock();

    update();
}

void VCXYPad::writeDMX(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer);

    m_currentXYPositionMutex.lock();
    if (m_currentXYPositionChanged == true)
    {
        m_currentXYPositionChanged = false;

        /* Scale XY coordinate values to 0.0 - 1.0 */
        float x = SCALE(float(m_currentXYPosition.x()),
                        float(0), float(width()), float(0), float(1));
        float y = SCALE(float(m_currentXYPosition.y()),
                        float(0), float(height()), float(0), float(1));

        /* Write values outside mutex lock to keep UI snappy */
        m_currentXYPositionMutex.unlock();
        foreach (VCXYPadFixture fixture, m_fixtures)
            fixture.writeDMX(x, y, universes);
    }
    else
    {
        /* No changes in values, unlock and get out */
        m_currentXYPositionMutex.unlock();
    }
}

/*****************************************************************************
 * QLC mode
 *****************************************************************************/

void VCXYPad::slotModeChanged(Doc::Mode mode)
{
    QMutableListIterator <VCXYPadFixture> it(m_fixtures);
    while (it.hasNext() == true)
    {
        VCXYPadFixture fxi(it.next());
        if (mode == Doc::Operate)
            fxi.arm();
        else
            fxi.disarm();
        it.setValue(fxi);
    }

    if (mode == Doc::Operate)
        _app->masterTimer()->registerDMXSource(this);
    else
        _app->masterTimer()->unregisterDMXSource(this);

    /* Reset this flag so that the pad won't immediately set a value
       when mode is changed */
    m_currentXYPositionChanged = false;

    VCWidget::slotModeChanged(mode);
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool VCXYPad::loader(const QDomElement* root, QWidget* parent)
{
    VCXYPad* xypad = NULL;

    Q_ASSERT(root != NULL);
    Q_ASSERT(parent != NULL);

    if (root->tagName() != KXMLQLCVCXYPad)
    {
        qWarning() << Q_FUNC_INFO << "XY Pad node not found";
        return false;
    }

    /* Create a new xy pad into its parent */
    xypad = new VCXYPad(parent);
    xypad->show();

    /* Continue loading */
    return xypad->loadXML(root);
}

bool VCXYPad::loadXML(const QDomElement* root)
{
    bool visible = false;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    int xpos = 0;
    int ypos = 0;

    QDomNode node;
    QDomElement tag;
    QString str;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCVCXYPad)
    {
        qWarning() << Q_FUNC_INFO << "XY Pad node not found";
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
        }
        else if (tag.tagName() == KXMLQLCVCAppearance)
        {
            loadXMLAppearance(&tag);
        }
        else if (tag.tagName() == KXMLQLCVCXYPadPosition)
        {
            str = tag.attribute(KXMLQLCVCXYPadPositionX);
            xpos = str.toInt();

            str = tag.attribute(KXMLQLCVCXYPadPositionY);
            ypos = str.toInt();
        }
        else if (tag.tagName() == KXMLQLCVCXYPadFixture)
        {
            VCXYPadFixture fxi;
            if (fxi.loadXML(&tag) == true)
                appendFixture(fxi);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown XY Pad tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    /* First set window dimensions and AFTER that set the
       pointer's XY position */
    setGeometry(x, y, w, h);
    setCurrentXYPosition(QPoint(xpos, ypos));

    return true;
}

bool VCXYPad::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;
    QString str;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(vc_root != NULL);

    /* VC XY Pad entry */
    root = doc->createElement(KXMLQLCVCXYPad);
    vc_root->appendChild(root);

    /* Caption */
    root.setAttribute(KXMLQLCVCCaption, caption());

    /* Fixtures */
    VCXYPadFixture fixture;
    foreach (fixture, m_fixtures)
    fixture.saveXML(doc, &root);

    /* Current XY Position */
    tag = doc->createElement(KXMLQLCVCXYPadPosition);
    tag.setAttribute(KXMLQLCVCXYPadPositionX,
                     QString("%1").arg(currentXYPosition().x()));
    tag.setAttribute(KXMLQLCVCXYPadPositionY,
                     QString("%1").arg(currentXYPosition().y()));
    root.appendChild(tag);

    /* Window state */
    saveXMLWindowState(doc, &root);

    /* Appearance */
    saveXMLAppearance(doc, &root);

    return true;
}

/*****************************************************************************
 * Event handlers
 *****************************************************************************/

void VCXYPad::paintEvent(QPaintEvent* e)
{
    /* Let the parent class draw its stuff first */
    VCWidget::paintEvent(e);

    QPainter p(this);
    QPen pen;

    /* Draw name (offset just a bit to avoid frame) */
    p.drawText(1, 1, width() - 2, height() - 2,
               Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, caption());

    /* Draw crosshairs to indicate the center position */
    pen.setStyle(Qt::DotLine);
    pen.setColor(palette().color(QPalette::WindowText));
    pen.setWidth(0);
    p.setPen(pen);
    p.drawLine(width() / 2, 0, width() / 2, height());
    p.drawLine(0, height() / 2, width(), height() / 2);

    /* Draw the current point pixmap */
    p.drawPixmap(m_currentXYPosition.x() - (m_xyPosPixmap.width() / 2),
                 m_currentXYPosition.y() - (m_xyPosPixmap.height() / 2),
                 m_xyPosPixmap);
}

void VCXYPad::mousePressEvent(QMouseEvent* e)
{
    if (mode() == Doc::Operate)
    {
        /* Mouse moves the XY point in operate mode */
        int x = CLAMP(e->x(), 0, width());
        int y = CLAMP(e->y(), 0, height());
        QPoint point(x, y);

        setCurrentXYPosition(point);
        setMouseTracking(true);
        setCursor(Qt::CrossCursor);
    }

    VCWidget::mousePressEvent(e);
}

void VCXYPad::mouseReleaseEvent(QMouseEvent* e)
{
    if (mode() == Doc::Operate)
    {
        /* Mouse moves the XY point in operate mode */
        setMouseTracking(false);
        unsetCursor();

        VCWidget::mouseReleaseEvent(e);
    }

    VCWidget::mouseReleaseEvent(e);
}

void VCXYPad::mouseMoveEvent(QMouseEvent* e)
{
    if (mode() == Doc::Operate)
    {
        /* Mouse moves the XY point in operate mode */
        int x = CLAMP(e->x(), 0, width());
        int y = CLAMP(e->y(), 0, height());
        QPoint point(x, y);

        setCurrentXYPosition(point);
        update();
    }

    VCWidget::mouseMoveEvent(e);
}
