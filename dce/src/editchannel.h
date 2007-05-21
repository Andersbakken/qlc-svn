/*
  Q Light Controller - Device Class Editor
  editchannel.h

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

#ifndef QLC_EDITCHANNEL_H
#define QLC_EDITCHANNEL_H

#include "uic_editchannel.h"

class QLCChannel;

class EditChannel : public UI_EditChannel
{
	Q_OBJECT
 public:
	EditChannel(QWidget* parent, QLCChannel* channel = NULL);
	~EditChannel();

	void init();

	/** Get the channel that was edited. Copy its contents to the real
	    LogicalChannel instance, but don't save the pointer! */
	QLCChannel* channel() { return m_channel; }

 public slots:
	void slotNameChanged(const QString& name);
	void slotGroupActivated(const QString& group);
	void slotControlByteActivated(int button);

 private:
	QLCChannel* m_channel;
};

#endif
