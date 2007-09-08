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

#ifndef SLIDERKEYBIND_H
#define SLIDERKEYBIND_H

#include <qobject.h>

class QKeyEvent;

class QDomDocument;
class QDomElement;

#define KXMLQLCSliderKeyBind "KeyBind"
#define KXMLQLCSliderKeyBindKey "Key"
#define KXMLQLCSliderKeyBindAction "Action"
#define KXMLQLCSliderKeyBindUp "Up"
#define KXMLQLCSliderKeyBindDown "Down"
#define KXMLQLCSliderKeyBindModifier "Modifier"

class SliderKeyBind : public QObject
{
	Q_OBJECT

public:
	SliderKeyBind();
	SliderKeyBind(const int keyUp, const int modUp,
		      const int keyDown, const int modDown);
	SliderKeyBind(const SliderKeyBind* skb);
	~SliderKeyBind();

	static void keyStringUp(int keyUp, int modUp, QString &string);
	void keyStringUp(QString &string) { return keyStringUp(m_keyUp, m_modUp, string); }

	static void keyStringDown(int keyDown, int modDown, QString &string);
	void keyStringDown(QString &string) { return keyStringDown(m_keyDown, m_modDown, string); }

	int keyUp() const { return m_keyUp; }
	void setKeyUp(int keyUp);

	int keyDown() const { return m_keyDown; }
	void setKeyDown(int keyDown);

	int modUp() const { return m_modUp; }
	void setModUp(int modUp);

	int modDown() const { return m_modDown; }
	void setModDown(int modDown);

	bool validUp() const { return m_validUp; }
	bool validDown() const { return m_validDown; }

	bool operator==(SliderKeyBind*);

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	static SliderKeyBind* loader(QDomDocument* doc, QDomElement* kb_root);
	bool loadXML(QDomDocument* doc, QDomElement* kb_root);
	bool saveXML(QDomDocument* doc, QDomElement* vc_root);

signals:
	void pressedUp();
	void pressedDown();

	public slots:
	void slotSliderKeyPressed(QKeyEvent* e);

private:
	int m_keyUp; // Key
	int m_keyDown;
	int m_modUp; // Modifier [shift|alt|control]
	int m_modDown;

	bool m_validUp; // Does this object contain a valid key or not
	bool m_validDown;
};

#endif
