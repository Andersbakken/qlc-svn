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

#include <QTextBrowser>
#include <QLineEdit>
#include <QKeyEvent>

#include "assignhotkey.h"
#include "keybind.h"

AssignHotKey::AssignHotKey(QWidget* parent) : QDialog(parent)
{
	setupUi(this);

	QString str = QString::null;
	str += QString("<HTML><HEAD><TITLE>Assign Key</TITLE></HEAD><BODY>");
	str += QString("<CENTER><H1>Assign Key</H1>");
	str += QString("Hit the key-combination that you want to assign. ");
	str += QString("You may hit either a single key or a key-combination");
	str += QString("using CTRL, ALT, and/or SHIFT.</CENTER>");
	str += QString("</BODY></HTML>");
	m_infoText->setText(str);
	m_infoText->setFocusPolicy(Qt::NoFocus);
	m_buttonBox->setFocusPolicy(Qt::NoFocus);

	m_previewEdit->setReadOnly(true);
	m_previewEdit->setAlignment(Qt::AlignCenter);
}

AssignHotKey::~AssignHotKey()
{
}

void AssignHotKey::keyPressEvent(QKeyEvent* e)
{
	m_keyBind = KeyBind(e->key(), e->modifiers());
	m_previewEdit->setText(m_keyBind.keyString());
}
