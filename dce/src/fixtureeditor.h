/*
  Q Light Controller - Fixture Editor
  qlcfixtureeditor.h
  
  Copyright (C) 2000, 2001, 2002 Heikki Junnila
  
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

#ifndef QLC_FIXTURE_EDITOR_H
#define QLC_FIXTURE_EDITOR_H

#include "uic_fixtureeditor.h"

class QCloseEvent;
class QLCFixtureDef;
class QLCChannel;
class QLCFixtureMode;
class QString;

class QLCFixtureEditor : public UI_FixtureEditor
{
	Q_OBJECT

 public:
	QLCFixtureEditor(QWidget* parent, QLCFixtureDef* fixtureDef);
	virtual ~QLCFixtureEditor();

	void init();
	bool save();
	bool saveAs();

	void setFileName(QString path) { m_fileName = path; }
	QString fileName() const { return m_fileName; }
	bool modified() const { return m_modified; }

 public slots:
	void slotManufacturerEditTextChanged(const QString &text);
	void slotModelEditTextChanged(const QString &text);
	void slotTypeSelected(const QString &text);

	void slotChannelListSelectionChanged(QListViewItem* item);
	void slotAddChannelClicked();
	void slotRemoveChannelClicked();
	void slotEditChannelClicked();

	void slotModeListSelectionChanged(QListViewItem* item);
	void slotAddModeClicked();
	void slotRemoveModeClicked();
	void slotEditModeClicked();

 protected:
	void closeEvent(QCloseEvent* e);

 signals:
	void closed(QLCFixtureEditor*);

 private:
	bool checkManufacturerModel();
	void setCaption();
	void setModified(bool modified = true);
	void ensureNewExtension();
	bool newExtensionReminder();
	void refreshChannelList();
	void refreshModeList();
 
	QLCChannel* currentChannel();
	QLCFixtureMode* currentMode();

 private:
	QLCFixtureDef* m_fixtureDef;
	QString m_fileName;
	bool m_modified;
};

#endif
