/*
  Q Light Controller
  deviceconsole.cpp

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

#include <qobjectlist.h>
#include <qlayout.h>
#include <assert.h>
#include <qpixmap.h>

#include "app.h"
#include "doc.h"
#include "device.h"
#include "sceneeditor.h"
#include "deviceconsole.h"
#include "consolechannel.h"
#include "configkeys.h"
#include "common/settings.h"

extern App* _app;

DeviceConsole::DeviceConsole(QWidget *parent)
	: QWidget(parent, "DeviceConsole"),
	
	m_layout      ( NULL ),
	m_sceneEditor ( NULL )
{
}

DeviceConsole::~DeviceConsole()
{
	while (!m_unitList.isEmpty())
	{
		delete m_unitList.take(0);
	}
	
	if (m_sceneEditor)
	{
		delete m_sceneEditor;
		m_sceneEditor = NULL;
	}
}

void DeviceConsole::closeEvent(QCloseEvent* e)
{
	emit closed();
}

void DeviceConsole::setDevice(t_device_id id)
{
	unsigned int i = 0;
	Device* device = NULL;
	ConsoleChannel* unit = NULL;
	
	m_deviceID = id;

	device = _app->doc()->device(m_deviceID);
	assert(device);

	// Set an icon
	setIcon(QPixmap(PIXMAPS + QString("/console.png")));

	// Set the main horizontal layout
	m_layout = new QHBoxLayout(this);
	m_layout->setAutoAdd(true);

	// Create scene editor widget
	if (m_sceneEditor) delete m_sceneEditor;
	m_sceneEditor = new SceneEditor(this);
	m_sceneEditor->setDevice(m_deviceID);
	m_sceneEditor->show();

	// Catch function add signals
	connect(_app->doc(), SIGNAL(functionAdded(t_function_id)),
		m_sceneEditor, SLOT(slotFunctionAdded(t_function_id)));

	// Catch function remove signals
	connect(_app->doc(), SIGNAL(functionRemoved(t_function_id)),
		m_sceneEditor, SLOT(slotFunctionRemoved(t_function_id)));

	// Catch function change signals
	connect(_app->doc(), SIGNAL(functionChanged(t_function_id)),
		m_sceneEditor, SLOT(slotFunctionChanged(t_function_id)));

	// Create channel units
	for (i = 0; i < device->deviceClass()->channels()->count(); i++)
	{
		unit = new ConsoleChannel(this, m_deviceID, i);
		unit->init();
		unit->update();

		// Channel updates to scene editor
		connect(unit, 
			SIGNAL(changed(t_channel, t_value, Scene::ValueType)),
			m_sceneEditor,
			SLOT(slotChannelChanged(t_channel, t_value, Scene::ValueType)));

		// Scene editor updates to channels
		connect(m_sceneEditor,
			SIGNAL(sceneActivated(SceneValue*,t_channel)),
			unit, 
			SLOT(slotSceneActivated(SceneValue*, t_channel)));

		m_unitList.append(unit);
	}

	// Update scene editor (also causes an update to channelunits)
	m_sceneEditor->update();
}
