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

#include "uic_deviceclasseditor.h"

class QCloseEvent;
class QLCFixture;
class QLCChannel;
class QLCFixtureMode;
class QString;

class QLCFixtureEditor : public UI_QLCFixtureEditor
{
	Q_OBJECT

 public:
	QLCFixtureEditor(QWidget* parent, QLCFixture* fixture);
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

	void slotAddModeClicked();
	void slotRemoveModeClicked();
	void slotEditModeClicked();

 protected:
	void closeEvent(QCloseEvent* e);

 signals:
	void closed(QLCFixtureEditor*);

 private:
	void setModified(bool modified = true);
	void refreshChannelList();
	void refreshModeList();
 
	QLCChannel* currentChannel();
	QLCFixtureMode* currentMode();

 private:
	QLCFixture* m_fixture;
	QString m_fileName;
	bool m_modified;
};

#endif
