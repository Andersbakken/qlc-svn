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

#include "mastertimer.h"
#include "function.h"
#include "fixture.h"
#include "chaser.h"
#include "doc.h"
#include "bus.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/

Chaser::Chaser(Doc* doc) : Function(doc)
{
    m_runTimeDirection = Forward;
    m_runTimePosition = 0;

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
        qDebug() << "Function node not found!";
        return false;
    }

    if (root->attribute(KXMLQLCFunctionType) !=
            typeToString(Function::Chaser))
    {
        qWarning("Function is not a chaser!");
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
            qDebug() << "Unknown chaser tag:" << tag.tagName();
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
    if (id == m_busID)
        m_tapped = true;
}

void Chaser::arm()
{
    Doc* doc = qobject_cast <Doc*> (parent());
    Q_ASSERT(doc != NULL);

    /* Check that all member functions exist (nonexistent functions can
       be present only when a corrupted file has been loaded) */
    QMutableListIterator<t_function_id> it(m_steps);
    while (it.hasNext() == true)
    {
        /* Remove any nonexistent member functions */
        if (doc->function(it.next()) == NULL)
            it.remove();
    }

    resetElapsed();
}

void Chaser::disarm()
{
}

void Chaser::preRun(MasterTimer* timer)
{
    /* Current position starts from invalid index because nextStep() is
       called first in write(). */
    m_runTimeDirection = m_direction;
    if (m_runTimeDirection == Forward)
        m_runTimePosition = -1;
    else
        m_runTimePosition = m_steps.count();

    /* Reset bus tapping flag */
    m_tapped = false;

    Function::preRun(timer);
}

void Chaser::postRun(MasterTimer* timer, QByteArray* universes)
{
    /* Only stop a member if the current position is within range. It is
       not in range, when nextStep() moves it deliberately off at each
       end in all running modes. */
    if (m_runTimePosition >= 0 && m_runTimePosition < m_steps.size())
        stopCurrent();

    Function::postRun(timer, universes);
}

void Chaser::write(MasterTimer* timer, QByteArray* universes)
{
    Q_UNUSED(universes);

    /* With some changes to scene, chaser could basically act as a
       proxy for its members by calling the scene's write() functions
       by itself instead of starting/stopping them. Whether this would do
       any good is not clear. */

    /* TODO: One extra step is required in single shot mode to return
       false and thus get a chaser removed from MasterTimer. Not a big
       problem, but removing already after starting the last step would
       improve performance (very) slightly. */

    if (elapsed() == 0)
    {
        /* This is the first step since the last start() call */

        /* Get the next step index */
        nextStep();

        /* Start the current function */
        startCurrent(timer);

        /* Mark one cycle being elapsed */
        incrementElapsed();
    }
    else if (elapsed() >= Bus::instance()->value(m_busID) ||
             m_tapped == true)
    {
        /* Reset tapped flag even if it wasn't true */
        m_tapped = false;

        /* Stop current function */
        stopCurrent();

        /* Get the next step index */
        nextStep();

        /* Check, whether we have gone a full cycle (thru all steps) */
        if (roundCheck() == false)
            stop();
        else
            startCurrent(timer);

        /* Mark one cycle being elapsed since it tracks only the
           duration of the current step. */
        resetElapsed();
        incrementElapsed();
    }
    else
    {
        /* Just waiting for hold time to pass */
        incrementElapsed();
    }
}

bool Chaser::roundCheck()
{
    /* Check if one complete chase round has been completed. */
    if ((m_runTimeDirection == Backward && m_runTimePosition == -1) ||
            (m_runTimeDirection == Forward && m_runTimePosition == m_steps.size()))
    {
        if (m_runOrder == SingleShot)
        {
            /* One single-shot round complete. Terminate. */
            return false;
        }
        else if (m_runOrder == Loop)
        {
            /* Loop around at either end */
            if (m_runTimeDirection == Forward)
                m_runTimePosition = 0;
            else
                m_runTimePosition = m_steps.count() - 1;
        }
        else /* Ping Pong */
        {
            /* Change running direction, don't run the step at
               each end twice, so -1/+1 */
            if (m_runTimeDirection == Forward)
            {
                m_runTimePosition = m_steps.count() - 2;
                m_runTimeDirection = Backward;
            }
            else
            {
                m_runTimePosition = 1;
                m_runTimeDirection = Forward;
            }
        }
    }

    /* Let's continue */
    return true;
}

void Chaser::nextStep()
{
    if (m_runTimeDirection == Forward)
        m_runTimePosition++;
    else
        m_runTimePosition--;
}

void Chaser::startCurrent(MasterTimer* timer)
{
    Doc* doc = qobject_cast <Doc*> (parent());
    Q_ASSERT(doc != NULL);

    t_function_id fid = m_steps.at(m_runTimePosition);
    Function* function = doc->function(fid);
    if (function != NULL)
        timer->startFunction(function, true);
    else
        qWarning() << name() << "step" << m_runTimePosition
        << "doesn't exist";
}

void Chaser::stopCurrent()
{
    Doc* doc = qobject_cast <Doc*> (parent());
    Q_ASSERT(doc != NULL);

    t_function_id fid = m_steps.at(m_runTimePosition);
    Function* function = doc->function(fid);
    if (function != NULL)
        function->stop();
    else
        qWarning() << name() << "step" << m_runTimePosition
        << "doesn't exist";
}
