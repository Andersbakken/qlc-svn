/*
  Q Light Controller
  keybind.cpp

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

#include <QKeyEvent>
#include <QDebug>
#include <QtXml>

#include "virtualconsole.h"
#include "keybind.h"
#include "app.h"

extern App* _app;

/*****************************************************************************
 * Initialization
 *****************************************************************************/

KeyBind::KeyBind() : QObject()
{
	m_action = Toggle;
	m_key = Qt::Key_unknown;
	m_mod = Qt::NoModifier;

	Q_ASSERT(_app->virtualConsole() != NULL);
	connect(_app->virtualConsole(), SIGNAL(keyPressed(QKeyEvent*)),
		this, SLOT(slotKeyPressed(QKeyEvent*)));

	connect(_app->virtualConsole(), SIGNAL(keyReleased(QKeyEvent*)),
		this, SLOT(slotKeyReleased(QKeyEvent*)));
}

KeyBind::KeyBind(const int key, const Qt::KeyboardModifiers mod) : QObject()
{
	m_action = Toggle;
	m_key = key;
	m_mod = mod;

	Q_ASSERT(_app->virtualConsole() != NULL);
	connect(_app->virtualConsole(), SIGNAL(keyPressed(QKeyEvent*)),
		this, SLOT(slotKeyPressed(QKeyEvent*)));

	connect(_app->virtualConsole(), SIGNAL(keyReleased(QKeyEvent*)),
		this, SLOT(slotKeyReleased(QKeyEvent*)));
}

KeyBind::KeyBind(const KeyBind* kb) : QObject()
{
	Q_ASSERT(kb != NULL);
	m_action = kb->action();
	m_key = kb->key();
	m_mod = kb->mod();

	Q_ASSERT(_app->virtualConsole() != NULL);
	connect(_app->virtualConsole(), SIGNAL(keyPressed(QKeyEvent*)),
		this, SLOT(slotKeyPressed(QKeyEvent*)));

	connect(_app->virtualConsole(), SIGNAL(keyReleased(QKeyEvent*)),
		this, SLOT(slotKeyReleased(QKeyEvent*)));
}

KeyBind::~KeyBind()
{
}

bool KeyBind::operator==(KeyBind* kb)
{
	if (m_key != kb->key() || m_mod != kb->mod())
		return false;
	else if (action() != kb->action())
		return false;
	else
		return true;
}

bool KeyBind::isValid() const
{
	if (m_key >= 0 && m_key <= Qt::Key_unknown)
		return true;
	else
		return false;
}

/*****************************************************************************
 * Key combo to string
 *****************************************************************************/

QString KeyBind::keyString(int key, int mod)
{
	QString modString = QString::null;
	QString keyString = QString::null;
	QString string;

	if (key >= Qt::Key_F1 && key <= Qt::Key_F35)
		keyString.sprintf("F%d", key - Qt::Key_F1 + 1);
	else if (key >= Qt::Key_0 && key <= Qt::Key_9)
		keyString.sprintf("%d", key - Qt::Key_0);
	else if (key >= Qt::Key_A && key <= Qt::Key_Z)
		keyString.sprintf("%c", 'A' + key - Qt::Key_A);
	else
	{
		switch(key)
		{
		case Qt::Key_Exclam:
			keyString.sprintf("!");
			break;
		case Qt::Key_QuoteDbl:
			keyString.sprintf("\"");
			break;
		case Qt::Key_NumberSign:
			keyString.sprintf("Unknown");
			break;
		case Qt::Key_Dollar:
			keyString.sprintf("$");
			break;
		case Qt::Key_Percent:
			keyString.sprintf("%%");
			break;
		case Qt::Key_Ampersand:
			keyString.sprintf("&");
			break;
		case Qt::Key_Apostrophe:
			keyString.sprintf("'");
			break;
		case Qt::Key_ParenLeft:
			keyString.sprintf("(");
			break;
		case Qt::Key_ParenRight:
			keyString.sprintf(")");
			break;
		case Qt::Key_Asterisk:
			keyString.sprintf("*");
			break;
		case Qt::Key_Plus:
			keyString.sprintf("+");
			break;
		case Qt::Key_Comma:
			keyString.sprintf(",");
			break;
		case Qt::Key_Minus:
			keyString.sprintf("-");
			break;
		case Qt::Key_Period:
			keyString.sprintf(".");
			break;
		case Qt::Key_Slash:
			keyString.sprintf("/");
			break;
		case Qt::Key_Colon:
			keyString.sprintf(":");
			break;
		case Qt::Key_Semicolon:
			keyString.sprintf(";");
			break;
		case Qt::Key_Less:
			keyString.sprintf("<");
			break;
		case Qt::Key_Equal:
			keyString.sprintf("/");
			break;
		case Qt::Key_Greater:
			keyString.sprintf(">");
			break;
		case Qt::Key_Question:
			keyString.sprintf("?");
			break;
		case Qt::Key_BracketLeft:
			keyString.sprintf("?");
			break;
		case Qt::Key_Backslash:
			keyString.sprintf("?");
			break;
		case Qt::Key_BracketRight:
			keyString.sprintf("?");
			break;
		case Qt::Key_AsciiCircum:
			keyString.sprintf("?");
			break;
		case Qt::Key_Underscore:
			keyString.sprintf("_");
			break;
		case Qt::Key_QuoteLeft:
			keyString.sprintf("`");
			break;
		case Qt::Key_BraceLeft:
			keyString.sprintf("{");
			break;
		case Qt::Key_Bar:
			keyString.sprintf("|");
			break;
		case Qt::Key_BraceRight:
			keyString.sprintf("}");
			break;
		case Qt::Key_AsciiTilde:
			keyString.sprintf("~");
			break;
		case Qt::Key_At:
			keyString.sprintf("@");
			break;
		case Qt::Key_Space:
			keyString.sprintf("Space");
			break;
		case Qt::Key_Escape:
			keyString.sprintf("Escape");
			break;
		case Qt::Key_Return:
			keyString.sprintf("Return");
			break;
		case Qt::Key_Enter:
			keyString.sprintf("Enter");
			break;
		case Qt::Key_Insert:
			keyString.sprintf("Insert");
			break;
		case Qt::Key_Delete:
			keyString.sprintf("Delete");
			break;
		case Qt::Key_Pause:
			keyString.sprintf("Pause");
			break;
		case Qt::Key_Home:
			keyString.sprintf("Home");
			break;
		case Qt::Key_End:
			keyString.sprintf("End");
			break;
		case Qt::Key_PageUp:
			keyString.sprintf("PageUp");
			break;
		case Qt::Key_PageDown:
			keyString.sprintf("PageDown");
			break;
		case Qt::Key_Left:
			keyString.sprintf("Left");
			break;
		case Qt::Key_Right:
			keyString.sprintf("Right");
			break;
		case Qt::Key_Up:
			keyString.sprintf("Up");
			break;
		case Qt::Key_Down:
			keyString.sprintf("Down");
			break;
		case Qt::Key_Shift:
			keyString.sprintf("Shift +");
			break;
		case Qt::Key_Alt:
			keyString.sprintf("Alt +");
			break;
		case Qt::Key_Control:
			keyString.sprintf("Control +");
			break;
		case 0:
			keyString.sprintf("None");
			break;
		case Qt::Key_unknown:
			keyString.sprintf("Unknown");
			break;
		default:
			keyString.sprintf("Code %d", key);
			break;
		}
	}

	if (mod & Qt::Key_Shift)
		modString += QString("Shift + ");

	if (mod & Qt::Key_Alt)
		modString += QString("Alt + ");

	if (mod & Qt::Key_Control)
		modString += QString("Control + ");

	if (key <= 0 || key > Qt::Key_unknown)
		string = QString("None");
	else
		string = QString(modString + keyString);

	return string;
}

/*****************************************************************************
 * Key and modifier
 *****************************************************************************/

void KeyBind::setKey(int key)
{
	if (key >= 0 && key <= Qt::Key_unknown)
		m_key = key;
	else
		key = Qt::Key_unknown;
}

void KeyBind::setMod(Qt::KeyboardModifiers mod)
{
	m_mod = mod;
}

/*****************************************************************************
 * Actions
 *****************************************************************************/

void KeyBind::slotKeyPressed(QKeyEvent* e)
{
	if (e->key() == m_key && (e->modifiers() == m_mod))
		emit pressed();
}

void KeyBind::slotKeyReleased(QKeyEvent* e)
{
	if (e->key() == m_key && (e->modifiers() == m_mod))
		emit released();
}

/*****************************************************************************
 * Action to string
 *****************************************************************************/

QString KeyBind::actionToString(KeyBind::Action action)
{
	switch (action)
	{
		default:
		case Toggle:
			return QString("Toggle");
		case Flash:
			return QString("Flash");
		case StepForward:
			return QString("StepForward");
		case StepBackward:
			return QString("StepBackward");
	}
}

KeyBind::Action KeyBind::stringToAction(QString action)
{
	if (action == "Flash")
		return KeyBind::Flash;
	else if (action == "StepForward")
		return KeyBind::StepForward;
	else if (action == "StepBackward")
		return KeyBind::StepBackward;
	else
		return KeyBind::Toggle;
}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

bool KeyBind::loadXML(QDomDocument*/* doc*/, QDomElement* root)
{
	QString action;
	QString mod;
	QString key;

	QDomNode node;
	QDomElement tag;

	Q_ASSERT(root != NULL);

	if (root->tagName() != KXMLQLCKeyBind)
	{
		qDebug() << "Key binding node not found!";
		return false;
	}

	/* Children */
	node = root->firstChild();
	while (node.isNull() == false)
	{
		tag = node.toElement();
		if (tag.tagName() == KXMLQLCKeyBindKey)
		{
			mod = tag.attribute(KXMLQLCKeyBindModifier);
			key = tag.text();

			setKey(key.toInt());
			setMod((Qt::KeyboardModifiers)mod.toInt());
		}
		else if (tag.tagName() == KXMLQLCKeyBindAction)
		{
			setAction(stringToAction(tag.text()));
		}
		else
		{
			qDebug() << "Unknown key binding tag:"
				 << tag.tagName();
		}

		node = node.nextSibling();
	}

	return true;
}

bool KeyBind::saveXML(QDomDocument* doc, QDomElement* vc_root)
{
	QDomElement root;
	QDomElement tag;
	QDomText text;
	QString str;

	Q_ASSERT(doc != NULL);
	Q_ASSERT(vc_root != NULL);

	/* KeyBind entry */
	root = doc->createElement(KXMLQLCKeyBind);
	vc_root->appendChild(root);

	/* Key node */
	tag = doc->createElement(KXMLQLCKeyBindKey);
	root.appendChild(tag);

	/* Modifier */
	str.setNum(mod());
	tag.setAttribute(KXMLQLCKeyBindModifier, str);

	/* Key */
	str.setNum(key());
	text = doc->createTextNode(str);
	tag.appendChild(text);

	/* Action */
	tag = doc->createElement(KXMLQLCKeyBindAction);
	root.appendChild(tag);
	text = doc->createTextNode(actionToString(m_action));
	tag.appendChild(text);
	
	return true;
}
