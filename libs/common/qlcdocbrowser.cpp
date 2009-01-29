/*
  Q Light Controller
  qlcdocbrowser.cpp

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

#include <QApplication>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QSettings>
#include <QToolBar>
#include <QAction>
#include <QIcon>
#include <QUrl>

#include "qlcdocbrowser.h"
#include "qlctypes.h"

QLCDocBrowser::QLCDocBrowser(QWidget* parent) : QMainWindow(parent)
{
	QStringList searchPaths;
	QSettings settings;
	QVariant w, h;

	setWindowTitle("Q Light Controller - Document Browser");

	/* Recall window size */
	w = settings.value("documentbrowser/width");
	h = settings.value("documentbrowser/height");
	if (w.isValid() == false || h.isValid() == false)
		resize(600, 600);
	else
		resize(w.toInt(), h.toInt());

	/* Actions */
	m_backwardAction = new QAction(QIcon(":/back.png"),
				       tr("Backward"), this);
	m_backwardAction->setEnabled(false);

	m_forwardAction = new QAction(QIcon(":/forward.png"),
				      tr("Forward"), this);
	m_forwardAction->setEnabled(false);

	m_homeAction = new QAction(QIcon(":/help.png"), tr("Index"), this);

	/* Toolbar */
	QToolBar* toolbar = new QToolBar("Document Browser", this);
	addToolBar(toolbar);
	toolbar->addAction(m_backwardAction);
	toolbar->addAction(m_forwardAction);
	toolbar->addAction(m_homeAction);

	/* Browser */
	m_browser = new QTextBrowser(this);
	setCentralWidget(m_browser);

	connect(m_browser, SIGNAL(backwardAvailable(bool)),
		this, SLOT(slotBackwardAvailable(bool)));
	connect(m_browser, SIGNAL(forwardAvailable(bool)),
		this, SLOT(slotForwardAvailable(bool)));

	connect(m_backwardAction, SIGNAL(triggered(bool)),
		m_browser, SLOT(backward()));
	connect(m_forwardAction, SIGNAL(triggered(bool)),
		m_browser, SLOT(forward()));
	connect(m_homeAction, SIGNAL(triggered(bool)),
		m_browser, SLOT(home()));

	searchPaths << QString("%1/html/").arg(DOCSDIR);
	m_browser->setSearchPaths(searchPaths);
	m_browser->setSource(QUrl(QString("index.html")));
}

QLCDocBrowser::~QLCDocBrowser()
{
	QSettings settings;
	settings.setValue("documentbrowser/width", width());
	settings.setValue("documentbrowser/height", height());
}

void QLCDocBrowser::slotBackwardAvailable(bool available)
{
	m_backwardAction->setEnabled(available);
}

void QLCDocBrowser::slotForwardAvailable(bool available)
{
	m_forwardAction->setEnabled(available);
}
