/*
  Q Light Controller
  vccuelistproperties.cpp

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

#include "vccuelistproperties.h"
#include "selectinputchannel.h"
#include "functionselection.h"
#include "assignhotkey.h"
#include "vccuelist.h"
#include "inputmap.h"
#include "app.h"
#include "doc.h"

extern App* _app;

VCCueListProperties::VCCueListProperties(QWidget* parent, VCCueList* cueList)
        : QDialog(parent)
{
    Q_ASSERT(cueList != NULL);
    m_cueList = cueList;

    setupUi(this);

    /************************************************************************
     * Cues page
     ************************************************************************/

    /* Name */
    m_nameEdit->setText(cueList->caption());
    m_nameEdit->setSelection(0, cueList->caption().length());

    /* Connections */
    connect(m_attachButton, SIGNAL(clicked()), this, SLOT(slotAttachClicked()));
    connect(m_detachButton, SIGNAL(clicked()), this, SLOT(slotDetachClicked()));

    /* Chaser */
    m_chaser = cueList->chaser();
    updateChaserName();

    /************************************************************************
     * Next Cue page
     ************************************************************************/

    /* Connections */
    connect(m_nextAttachButton, SIGNAL(clicked()),
            this, SLOT(slotNextAttachClicked()));
    connect(m_nextDetachButton, SIGNAL(clicked()),
            this, SLOT(slotNextDetachClicked()));
    connect(m_nextAutoDetectInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotNextAutoDetectInputToggled(bool)));
    connect(m_nextChooseInputButton, SIGNAL(clicked()),
            this, SLOT(slotNextChooseInputClicked()));

    /* Key binding */
    m_nextKeySequence = QKeySequence(cueList->nextKeySequence());
    m_nextKeyEdit->setText(m_nextKeySequence.toString());

    /* External input */
    m_nextInputUniverse = cueList->nextInputUniverse();
    m_nextInputChannel = cueList->nextInputChannel();
    updateNextInputSource();

    /************************************************************************
     * Previous Cue page
     ************************************************************************/

    /* Connections */
    connect(m_previousAttachButton, SIGNAL(clicked()),
            this, SLOT(slotPreviousAttachClicked()));
    connect(m_previousDetachButton, SIGNAL(clicked()),
            this, SLOT(slotPreviousDetachClicked()));
    connect(m_previousAutoDetectInputButton, SIGNAL(toggled(bool)),
            this, SLOT(slotPreviousAutoDetectInputToggled(bool)));
    connect(m_previousChooseInputButton, SIGNAL(clicked()),
            this, SLOT(slotPreviousChooseInputClicked()));

    /* Key binding */
    m_previousKeySequence = QKeySequence(cueList->previousKeySequence());
    m_previousKeyEdit->setText(m_previousKeySequence.toString());

    /* External input */
    m_previousInputUniverse = cueList->previousInputUniverse();
    m_previousInputChannel = cueList->previousInputChannel();
    updatePreviousInputSource();
}

VCCueListProperties::~VCCueListProperties()
{
}

void VCCueListProperties::accept()
{
    /* Replace existing chaser */
    m_cueList->setChaser(m_chaser);

    /* Key sequences */
    m_cueList->setNextKeySequence(m_nextKeySequence);
    m_cueList->setPreviousKeySequence(m_previousKeySequence);

    /* Input sources */
    m_cueList->setNextInputSource(m_nextInputUniverse, m_nextInputChannel);
    m_cueList->setPreviousInputSource(m_previousInputUniverse, m_previousInputChannel);

    QDialog::accept();
}

void VCCueListProperties::slotTabChanged()
{
    // Disengage both auto-detect buttons
    if (m_nextAutoDetectInputButton->isChecked() == true)
        m_nextAutoDetectInputButton->toggle();
    if (m_previousAutoDetectInputButton->isChecked() == true)
        m_previousAutoDetectInputButton->toggle();
}

/****************************************************************************
 * Cues
 ****************************************************************************/

void VCCueListProperties::updateChaserName()
{
    Function* function = _app->doc()->function(m_chaser);
    if (function != NULL)
        m_chaserEdit->setText(function->name());
    else
        m_chaserEdit->setText(tr("Nothing"));
}

void VCCueListProperties::slotAttachClicked()
{
    FunctionSelection fs(this, false, Function::invalidId(), Function::Chaser, true);
    if (fs.exec() == QDialog::Accepted)
    {
        if (fs.selection().isEmpty() == false)
            m_chaser = fs.selection().first();
        updateChaserName();
    }
}

void VCCueListProperties::slotDetachClicked()
{
    m_chaser = Function::invalidId();
    updateChaserName();
}

/****************************************************************************
 * Next Cue
 ****************************************************************************/

void VCCueListProperties::slotNextAttachClicked()
{
    AssignHotKey ahk(this, m_nextKeySequence);
    if (ahk.exec() == QDialog::Accepted)
    {
        m_nextKeySequence = QKeySequence(ahk.keySequence());
        m_nextKeyEdit->setText(m_nextKeySequence.toString());
    }
}

void VCCueListProperties::slotNextDetachClicked()
{
    m_nextKeySequence = QKeySequence();
    m_nextKeyEdit->setText(m_nextKeySequence.toString());
}

void VCCueListProperties::slotNextChooseInputClicked()
{
    SelectInputChannel sic(this);
    if (sic.exec() == QDialog::Accepted)
    {
        m_nextInputUniverse = sic.universe();
        m_nextInputChannel = sic.channel();
        updateNextInputSource();
    }
}

void VCCueListProperties::slotNextAutoDetectInputToggled(bool checked)
{
    if (checked == true)
    {
        connect(_app->inputMap(),
                SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotNextInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(_app->inputMap(),
                   SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotNextInputValueChanged(quint32,quint32)));
    }
}

void VCCueListProperties::slotNextInputValueChanged(quint32 uni, quint32 ch)
{
    m_nextInputUniverse = uni;
    m_nextInputChannel = ch;
    updateNextInputSource();
}

void VCCueListProperties::updateNextInputSource()
{
    QString uniName;
    QString chName;

    if (_app->inputMap()->inputSourceNames(m_nextInputUniverse,
                                           m_nextInputChannel,
                                           uniName, chName) == true)
    {
        /* Display the gathered information */
        m_nextInputUniverseEdit->setText(uniName);
        m_nextInputChannelEdit->setText(chName);
    }
    else
    {
        m_nextInputUniverseEdit->setText(KInputNone);
        m_nextInputChannelEdit->setText(KInputNone);
    }
}

/****************************************************************************
 * Previous Cue
 ****************************************************************************/

void VCCueListProperties::slotPreviousAttachClicked()
{
    AssignHotKey ahk(this, m_previousKeySequence);
    if (ahk.exec() == QDialog::Accepted)
    {
        m_previousKeySequence = QKeySequence(ahk.keySequence());
        m_previousKeyEdit->setText(m_previousKeySequence.toString());
    }
}

void VCCueListProperties::slotPreviousDetachClicked()
{
    m_previousKeySequence = QKeySequence();
    m_previousKeyEdit->setText(m_previousKeySequence.toString());
}

void VCCueListProperties::slotPreviousChooseInputClicked()
{
    SelectInputChannel sic(this);
    if (sic.exec() == QDialog::Accepted)
    {
        m_previousInputUniverse = sic.universe();
        m_previousInputChannel = sic.channel();
        updatePreviousInputSource();
    }
}

void VCCueListProperties::slotPreviousAutoDetectInputToggled(bool checked)
{
    if (checked == true)
    {
        if (m_nextAutoDetectInputButton->isChecked() == true)
            m_nextAutoDetectInputButton->toggle();

        connect(_app->inputMap(),
                SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                this, SLOT(slotPreviousInputValueChanged(quint32,quint32)));
    }
    else
    {
        disconnect(_app->inputMap(),
                   SIGNAL(inputValueChanged(quint32,quint32,uchar)),
                   this, SLOT(slotPreviousInputValueChanged(quint32,quint32)));
    }
}

void VCCueListProperties::slotPreviousInputValueChanged(quint32 uni, quint32 ch)
{
    m_previousInputUniverse = uni;
    m_previousInputChannel = ch;
    updatePreviousInputSource();
}

void VCCueListProperties::updatePreviousInputSource()
{
    QString uniName;
    QString chName;

    if (_app->inputMap()->inputSourceNames(m_previousInputUniverse,
                                           m_previousInputChannel,
                                           uniName, chName) == true)
    {
        /* Display the gathered information */
        m_previousInputUniverseEdit->setText(uniName);
        m_previousInputChannelEdit->setText(chName);
    }
    else
    {
        m_previousInputUniverseEdit->setText(KInputNone);
        m_previousInputChannelEdit->setText(KInputNone);
    }
}
