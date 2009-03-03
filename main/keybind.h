/*
  Q Light Controller
  keybind.h

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

#ifndef KEYBIND_H
#define KEYBIND_H

class QDomDocument;
class QDomElement;

#define KXMLQLCKeyBind "KeyBind"
#define KXMLQLCKeyBindKey "Key"
#define KXMLQLCKeyBindModifier "Modifier"
#define KXMLQLCKeyBindAction "Action"

class KeyBind
{
	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	/** Construct an empty object */
	KeyBind();

	/** Construct an object with the given key and modifier */
	KeyBind(const int key, const Qt::KeyboardModifiers mod);

	/** Construct a copy object from the given KeyBind object */
	KeyBind(const KeyBind& kb);

	/** Destructor */
	~KeyBind();

	/*********************************************************************
	 * Operators
	 *********************************************************************/
public:
	/** Copy the contents from another KeyBind object */
	KeyBind& operator=(const KeyBind& kb);

	/** Compare operator between two KeyBind objects */
	bool operator==(const KeyBind& kb) const;

	/** Check, whether the key binding is valid (i.e. there is a key) */
	bool isValid() const;

	/*********************************************************************
	 * Key combo to string
	 *********************************************************************/
public:
	/**
	 * Get a string representation matching the given key & modifier
	 *
	 * @param key Keyboard key
	 * @param mod Keyboard modifier key (alt, ctrl, shift...)
	 * @return Key & modifier (e.g. "Ctrl + A")
	 */
	static QString keyString(int key, Qt::KeyboardModifiers mod);

	/**
	 * Get a string representation matching the object's current key
	 * and modifier.
	 *
	 * @return Key & modifier (e.g. "Ctrl + A")
	 */
	QString keyString() { return keyString(m_key, m_mod); }

	/*********************************************************************
	 * Key and modifier
	 *********************************************************************/
public:
	/** Get the assigned key */
	int key() const { return m_key; }

	/** Set the assigned key */
	void setKey(int key);

	/** Get the assigned modifier key */
	Qt::KeyboardModifiers mod() const { return m_mod; }

	/** Set the assigned modifier key */
	void setMod(Qt::KeyboardModifiers mod);

protected:
	/** The assigned key */
	int m_key;

	/** The assigned modified key (alt, ctrl, shift...) */
	Qt::KeyboardModifiers m_mod;
	
	/*********************************************************************
	 * Actions
	 *********************************************************************/
public:
	enum Action { Toggle, Flash };

	/** Set the action to take when the assigned key combo is pressed */
	void setAction(Action action) { m_action = action; }

	/** Get the action to take when the assigned key combo is pressed */
	KeyBind::Action action() const { return m_action; }

	/** Convert the given action to string */
	static QString actionToString(KeyBind::Action action);

	/** Convert the given string to action*/
	static KeyBind::Action stringToAction(const QString& action);

protected:
	/** The action to take when the assigned key combo is pressed */
	Action m_action;

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	/** Load this object's properties from an XML document */
	bool loadXML(QDomDocument* doc, QDomElement* root);

	/** Save this object's properties to an XML document */
	bool saveXML(QDomDocument* doc, QDomElement* vc_root);
};

#endif
