/*
  Q Light Controller
  documentbrowser.cpp

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

#include <qevent.h>
#include <qlayout.h>
#include <qtoolbar.h>
#include <qdockarea.h>
#include <qtoolbutton.h>
#include <qtextbrowser.h>
#include <qmainwindow.h>

#include "documentbrowser.h"

DocumentBrowser::DocumentBrowser(QMainWindow* appMainWindow, QWidget* parent, const char* name)
{
	m_appMainWindow = appMainWindow;
}

DocumentBrowser::~DocumentBrowser()
{
}

void DocumentBrowser::init()
{
  setCaption("Q Light Controller - Document Browser");
  resize(600, 600);

  m_layout = new QVBoxLayout(this);

  QDockArea* m_dockArea = new QDockArea(Horizontal, QDockArea::Normal, this);
  m_layout->addWidget(m_dockArea);

  //
  // Toolbar
  //
  m_toolbar = new QToolBar("Document Browser", m_appMainWindow, m_dockArea);
  m_toolbar->setMovingEnabled(false);

  m_prevTB = new QToolButton(QPixmap(QString(PIXMAPS) + QString("/back.png")),
			     "Previous", 0, this, SLOT(slotPrev()), m_toolbar);
  m_prevTB->setUsesTextLabel(true);
  m_prevTB->setEnabled(false);

  m_nextTB = new QToolButton(QPixmap(QString(PIXMAPS) + QString("/forward.png")),
			     "Next", 0, this, SLOT(slotNext()), m_toolbar);
  m_nextTB->setUsesTextLabel(true);
  m_nextTB->setEnabled(false);

  //
  // Browser
  //
  m_browser = new QTextBrowser(this);
  m_layout->addWidget(m_browser);

  m_browser->setSource(QString(DOCUMENTS) + QString("/index.html"));
  connect(m_browser, SIGNAL(backwardAvailable(bool)),
	  this, SLOT(slotBackwardAvailable(bool)));
  connect(m_browser, SIGNAL(forwardAvailable(bool)),
	  this, SLOT(slotForwardAvailable(bool)));
}

void DocumentBrowser::slotPrev()
{
  m_browser->backward();
}

void DocumentBrowser::slotBackwardAvailable(bool available)
{
  m_prevTB->setEnabled(available);
}

void DocumentBrowser::slotNext()
{
  m_browser->forward();
}

void DocumentBrowser::slotForwardAvailable(bool available)
{
  m_nextTB->setEnabled(available);
}

void DocumentBrowser::closeEvent(QCloseEvent*)
{
  emit closed();
}
