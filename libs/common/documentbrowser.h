/*
  Q Light Controller
  documentbrowser.h

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

#ifndef DOCUMENTBROWSER_H
#define DOCUMENTBROWSER_H

#include <qwidget.h>

class QVBoxLayout;
class QToolBar;
class QToolButton;
class QTextBrowser;
class QCloseEvent;
class QMainWindow;

class DocumentBrowser : public QWidget
{
  Q_OBJECT

 public:
  DocumentBrowser(QMainWindow* appMainWindow,
		  QWidget* parent = NULL,
		  const char* name = NULL);
  ~DocumentBrowser();

  void init();
  void updateButtons();

 signals:
  void closed();

 public slots:
  void slotPrev();
  void slotBackwardAvailable(bool);
  void slotNext();
  void slotForwardAvailable(bool);

 protected:
  void closeEvent(QCloseEvent*);

 protected:
  QToolBar* m_toolbar;
  QVBoxLayout* m_layout;
  QTextBrowser* m_browser;

  QToolButton* m_prevTB;
  QToolButton* m_nextTB;

  QMainWindow* m_appMainWindow;
};

#endif
