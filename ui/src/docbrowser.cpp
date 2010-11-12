/*
  Q Light Controller
  docbrowser.cpp

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

#include <QGestureEvent>
#include <QSwipeGesture>
#include <QApplication>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QSettings>
#include <QToolBar>
#include <QAction>
#include <QDebug>
#include <QIcon>
#include <QUrl>

#include "docbrowser.h"
#include "qlcconfig.h"
#include "apputil.h"

#define SETTINGS_GEOMETRY "documentbrowser/geometry"

QLCTextBrowser::QLCTextBrowser(QWidget* parent)
    : QTextBrowser(parent)
{
    grabGesture(Qt::SwipeGesture);
}

QLCTextBrowser::~QLCTextBrowser()
{
}

bool QLCTextBrowser::event(QEvent* ev)
{
    if (ev->type() == QEvent::Gesture)
    {
        QGestureEvent* gesture = static_cast<QGestureEvent*> (ev);
        QSwipeGesture* swipe = qobject_cast<QSwipeGesture*> (
            gesture->gesture(Qt::SwipeGesture));
        if (swipe == NULL)
        {
            /* NOP */
        }
        else if (swipe->horizontalDirection() == QSwipeGesture::Left)
        {
            backward();
            ev->accept();
        }
        else if (swipe->horizontalDirection() == QSwipeGesture::Right)
        {
            forward();
            ev->accept();
        }
    }

    return QTextBrowser::event(ev);
}

DocBrowser::DocBrowser(QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f)
{
    new QVBoxLayout(this);

    setWindowTitle(tr("%1 - Document Browser").arg(APPNAME));
    setWindowIcon(QIcon(":/help.png"));

    /* Recall window size */
    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());
    AppUtil::ensureWidgetIsVisible(this);

    /* Actions */
    m_backwardAction = new QAction(QIcon(":/back.png"), tr("Backward"), this);
    m_forwardAction = new QAction(QIcon(":/forward.png"), tr("Forward"), this);
    m_homeAction = new QAction(QIcon(":/qlc.png"), tr("Index"), this);

    m_backwardAction->setEnabled(false);
    m_forwardAction->setEnabled(false);

    /* Toolbar */
    m_toolbar = new QToolBar("Document Browser", this);
    layout()->addWidget(m_toolbar);
    m_toolbar->addAction(m_backwardAction);
    m_toolbar->addAction(m_forwardAction);
    m_toolbar->addAction(m_homeAction);

    /* Browser */
    m_browser = new QLCTextBrowser(this);
    m_browser->setOpenExternalLinks(true);
    layout()->addWidget(m_browser);
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

    /* Set document search paths */
    QStringList searchPaths;
#ifdef __APPLE__
    searchPaths << QString("%1/../%2/html/")
    .arg(QApplication::applicationDirPath())
    .arg(DOCSDIR);
#else
    searchPaths << QString("%1/html/").arg(DOCSDIR);
#endif

    m_browser->setSearchPaths(searchPaths);
    m_browser->setSource(QUrl(QString("index.html")));
}

DocBrowser::~DocBrowser()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

void DocBrowser::slotBackwardAvailable(bool available)
{
    m_backwardAction->setEnabled(available);
}

void DocBrowser::slotForwardAvailable(bool available)
{
    m_forwardAction->setEnabled(available);
}
