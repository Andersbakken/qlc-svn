/*
  Q Light Controller
  chaser.cpp

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

#include <QDebug>
#include <QFile>
#include <QtXml>

#include "qlcfixturedef.h"
#include "qlcfile.h"

#include "universearray.h"
#include "chaserrunner.h"
#include "mastertimer.h"
#include "function.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"
#include "bus.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Chaser::Chaser(Doc* doc) : Function(doc)
{
    m_runTimeDirection = Forward;
    m_runTimePosition = 0;
    m_runner = NULL;

    setName(tr("New Chaser"));
    setBus(Bus::defaultHold());

    connect(Bus::instance(), SIGNAL(tapped(quint32)),
            this, SLOT(slotBusTapped(quint32)));

    // Listen to member Function removals
    connect(doc, SIGNAL(functionRemoved(t_function_id)),
            this, SLOT(slotFunctionRemoved(t_function_id)));
}

Chaser::~Chaser()
{
    m_steps.clear();
}

/*****************************************************************************
 * Function type
 *****************************************************************************/

Function::Type Chaser::type() const
{
    return Function::Chaser;
}

/*****************************************************************************
 * Copying
 *****************************************************************************/

Function* Chaser::createCopy(Doc* doc)
{
    Q_ASSERT(doc != NULL);

    Function* copy = new Chaser(doc);
    Q_ASSERT(copy != NULL);
    if (copy->copyFrom(this) == false)
    {
        delete copy;
        copy = NULL;
    }
    else if (doc->addFunction(copy) == false)
    {
        delete copy;
        copy = NULL;
    }
    else
    {
        copy->setName(tr("Copy of %1").arg(name()));
    }

    return copy;
}

bool Chaser::copyFrom(const Function* function)
{
    const Chaser* chaser = qobject_cast<const Chaser*> (function);
    if (chaser == NULL)
        return false;

    m_steps.clear();
    m_steps = chaser->m_steps;

    bool result = Function::copyFrom(function);

    emit changed(m_id);

    return result;
}

/*****************************************************************************
 * Contents
 *****************************************************************************/

bool Chaser::addStep(t_function_id id)
{
    if (id != m_id)
    {
        m_steps.append(id);
        emit changed(m_id);
        return true;
    }
    else
    {
        return false;
    }
}

bool Chaser::removeStep(int index)
{
    if (index >= 0 && index < m_steps.size())
    {
        m_steps.removeAt(index);
        emit changed(m_id);
        return true;
    }
    else
    {
        return false;
    }
}

bool Chaser::raiseStep(int index)
{
    if (index > 0 && index < m_steps.count())
    {
        t_function_id fid = m_steps.takeAt(index);
        m_steps.insert(index - 1, fid);

        emit changed(m_id);
        return true;
    }
    else
    {
        return false;
    }
}

bool Chaser::lowerStep(int index)
{
    if (index >= 0 && index < (m_steps.count() - 1))
    {
        t_function_id fid = m_steps.takeAt(index);
        m_steps.insert(index + 1, fid);

        emit changed(m_id);
        return true;
    }
    else
    {
        return false;
    }
}

QList <t_function_id> Chaser::steps() const
{
    return m_steps;
}

QList <Function*> Chaser::stepFunctions() const
{
    Doc* doc = qobject_cast<Doc*> (parent());
    Q_ASSERT(doc != NULL);

    QList <Function*> list;
    QListIterator <t_function_id> it(m_steps);
    while (it.hasNext() == true)
    {
        Function* function = doc->function(it.next());
        if (function != NULL)
            list << function;
    }

    return list;
}

void Chaser::slotFunctionRemoved(t_function_id fid)
{
    m_steps.removeAll(fid);
}

/*****************************************************************************
 * Save & Load
 *****************************************************************************/

bool Chaser::saveXML(QDomDocument* doc, QDomElement* wksp_root)
{
    QDomElement root;
    QDomElement tag;
    QDomText text;
    QString str;
    int i = 0;

    Q_ASSERT(doc != NULL);
    Q_ASSERT(wksp_root != NULL);

    /* Function tag */
    root = doc->createElement(KXMLQLCFunction);
    wksp_root->appendChild(root);

    root.setAttribute(KXMLQLCFunctionID, id());
    root.setAttribute(KXMLQLCFunctionType, Function::typeToString(type()));
    root.setAttribute(KXMLQLCFunctionName, name());

    /* Speed bus */
    tag = doc->createElement(KXMLQLCBus);
    root.appendChild(tag);
    tag.setAttribute(KXMLQLCBusRole, KXMLQLCBusHold);
    str.setNum(busID());
    text = doc->createTextNode(str);
    tag.appendChild(text);

    /* Direction */
    tag = doc->createElement(KXMLQLCFunctionDirection);
    root.appendChild(tag);
    text = doc->createTextNode(Function::directionToString(m_direction));
    tag.appendChild(text);

    /* Run order */
    tag = doc->createElement(KXMLQLCFunctionRunOrder);
    root.appendChild(tag);
    text = doc->createTextNode(Function::runOrderToString(m_runOrder));
    tag.appendChild(text);

    /* Steps */
    QListIterator <t_function_id> it(m_steps);
    while (it.hasNext() == true)
    {
        /* Step tag */
        tag = doc->createElement(KXMLQLCFunctionStep);
        root.appendChild(tag);

        /* Step number */
        tag.setAttribute(KXMLQLCFunctionNumber, i++);

        /* Step Function ID */
        str.setNum(it.next());
        text = doc->createTextNode(str);
        tag.appendChild(text);
    }

    return true;
}

bool Chaser::loadXML(const QDomElement* root)
{
    QDomNode node;
    QDomElement tag;

    Q_ASSERT(root != NULL);

    if (root->tagName() != KXMLQLCFunction)
    {
        qWarning() << Q_FUNC_INFO << "Function node not found";
        return false;
    }

    if (root->attribute(KXMLQLCFunctionType) != typeToString(Function::Chaser))
    {
        qWarning() << Q_FUNC_INFO << root->attribute(KXMLQLCFunctionType)
                   << "is not a chaser";
        return false;
    }

    /* Load chaser contents */
    node = root->firstChild();
    while (node.isNull() == false)
    {
        tag = node.toElement();

        if (tag.tagName() == KXMLQLCBus)
        {
            /* Bus */
            setBus(tag.text().toUInt());
        }
        else if (tag.tagName() == KXMLQLCFunctionDirection)
        {
            /* Direction */
            setDirection(Function::stringToDirection(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCFunctionRunOrder)
        {
            /* Run Order */
            setRunOrder(Function::stringToRunOrder(tag.text()));
        }
        else if (tag.tagName() == KXMLQLCFunctionStep)
        {
            t_function_id fid = -1;
            int num = 0;

            num = tag.attribute(KXMLQLCFunctionNumber).toInt();
            fid = tag.text().toInt();

            /* Don't check for the member function's existence,
               because it might not have been loaded yet. */
            if (num >= m_steps.size())
                m_steps.append(fid);
            else
                m_steps.insert(num, fid);
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown chaser tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    return true;
}

/*****************************************************************************
 * Running
 *****************************************************************************/

void Chaser::slotBusTapped(quint32 id)
{
    if (id == m_busID && m_runner != NULL)
        m_runner->next();
}

void Chaser::arm()
{
    Doc* doc = qobject_cast <Doc*> (parent());
    Q_ASSERT(doc != NULL);
    m_runner = new ChaserRunner(doc, stepFunctions(), busID(), direction(), runOrder());
    resetElapsed();
}

void Chaser::disarm()
{
    delete m_runner;
    m_runner = NULL;
}

void Chaser::preRun(MasterTimer* timer)
{
    Q_UNUSED(timer);
    Q_ASSERT(m_runner != NULL);
    m_runner->reset();

    Function::preRun(timer);
}

void Chaser::write(MasterTimer* timer, UniverseArray* universes)
{
    Q_UNUSED(timer);
    Q_ASSERT(m_runner != NULL);

    if (m_runner->write(universes) == false)
        stop();

    incrementElapsed();
}
