/*
  Q Light Controller
  assignhotkey.cpp

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

#include <QKeySequence>
#include <QTextBrowser>
#include <QLineEdit>
#include <QKeyEvent>
#include <QDebug>

#include "assignhotkey.h"

/*****************************************************************************
 * Initialization
 *****************************************************************************/
AssignHotKey::AssignHotKey(QWidget* parent, const QKeySequence& keySequence)
        : QDialog(parent)
{
    setupUi(this);

    QString str("<HTML><HEAD><TITLE></TITLE></HEAD><BODY><CENTER>");
    str += QString("<H1>") + tr("Assign Key") + QString("</H1>");
    str += tr("Hit the key combination that you wish to assign. "
              "You may hit either a single key or a combination "
              "using CTRL, ALT, and SHIFT.");
    str += QString("</CENTER></BODY></HTML>");

    /* TODO: For OSX, put the apple key to the above text */

    m_infoText->setText(str);
    m_infoText->setFocusPolicy(Qt::NoFocus);
    m_buttonBox->setFocusPolicy(Qt::NoFocus);

    m_previewEdit->setReadOnly(true);
    m_previewEdit->setAlignment(Qt::AlignCenter);

    m_keySequence = QKeySequence(keySequence);
    m_previewEdit->setText(m_keySequence.toString());
}

AssignHotKey::~AssignHotKey()
{
}

void AssignHotKey::keyPressEvent(QKeyEvent* event)
{
    m_keySequence = QKeySequence(event->key() | event->modifiers());
    m_previewEdit->setText(m_keySequence.toString());
}
