/*
  Q Light Controller
  pluginmanager.h

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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <qwidget.h>

class QToolBar;
class QToolButton;
class QVBoxLayout;
class QCloseEvent;
class QDockArea;
class QSplitter;
class QListView;
class QListViewItem;
class QTextView;

class Plugin;

class PluginManager : public QWidget
{
	Q_OBJECT

public:
	PluginManager(QWidget* parent);
	~PluginManager();

	void initView();

protected:
	void initTitle();
	void initToolBar();
	void initMenuBar();
	void initDataView();
	
	void fillPlugins();
	
protected slots:
	void slotConfigureClicked();
	void slotDMXMapperClicked();

	void slotSelectionChanged(QListViewItem* item);
	void slotRightButtonClicked(QListViewItem* item,
				    const QPoint& point,
				    int col);

protected:
	void closeEvent(QCloseEvent* e);

signals:
	void closed();
	
protected:
	QVBoxLayout* m_layout;
	QToolBar* m_toolbar;
	QDockArea* m_dockArea;
	QSplitter* m_splitter;
	QListView* m_listView;
	QTextView* m_textView;
	
	QToolButton* m_configureButton;
	QToolButton* m_mapButton;
};

#endif
