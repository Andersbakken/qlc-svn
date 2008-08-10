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

#include <QWidget>

class QTreeWidgetItem;
class QTextBrowser;
class QTreeWidget;
class QVBoxLayout;
class QSplitter;
class QAction;

class QLCPlugin;

class PluginManager : public QWidget
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	PluginManager(QWidget* parent);
	~PluginManager();

protected:
	void initActions();
	void initToolBar();
	void initMenuBar();
	void initDataView();
	
	void fillPlugins();
	
	/*********************************************************************
	 * Plugin tree
	 *********************************************************************/
protected slots:
	void slotConfigure();
	void slotOutputMap();
	void slotInputMap();

	void slotSelectionChanged();
	void slotContextMenuRequested(const QPoint& point);

protected:
	QAction* m_configureAction;
	QAction* m_outputMapAction;
	QAction* m_inputMapAction;

	QSplitter* m_splitter;
	QTreeWidget* m_tree;
	QTextBrowser* m_info;

	/*********************************************************************
	 * Default settings
	 *********************************************************************/
public:
	void saveDefaults();
	void loadDefaults();
};

#endif
