/*
  Q Light Controller
  qlcdocbrowser.h

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

#ifndef QLCDOCBROWSER_H
#define QLCDOCBROWSER_H

#include <QMainWindow>
#include "qlctypes.h"

class QTextBrowser;
class QAction;

class QLC_DECLSPEC QLCDocBrowser : public QMainWindow
{
	Q_OBJECT

public:
	QLCDocBrowser(QWidget* parent);
	~QLCDocBrowser();

private:
	Q_DISABLE_COPY(QLCDocBrowser)

protected slots:
	void slotBackwardAvailable(bool);
	void slotForwardAvailable(bool);

protected:
	QAction* m_backwardAction;
	QAction* m_forwardAction;
	QAction* m_homeAction;

	QTextBrowser* m_browser;
};

#endif
