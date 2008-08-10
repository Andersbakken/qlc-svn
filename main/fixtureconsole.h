/*
  Q Light Controller
  fixtureconsole.h
  
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

#ifndef FIXTURECONSOLE_H
#define FIXTURECONSOLE_H

#include <QWidget>

#include "common/qlctypes.h"
#include "consolechannel.h"

class QDomDocument;
class QDomElement;

#define KXMLQLCFixtureConsole "Console"

class FixtureConsole : public QWidget
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	FixtureConsole(QWidget* parent, t_fixture_id fxi_id);
	~FixtureConsole();

	/*********************************************************************
	 * Fixture
	 *********************************************************************/
protected:
	void setFixture(t_fixture_id id);

protected:
	t_fixture_id m_fixture;

	/*********************************************************************
	 * Channels
	 *********************************************************************/
public:
	/** Set all channels enabled/disabled */
	void enableAllChannels(bool enable);

	/** Enable/disable DMX output when sliders are dragged */
	void setOutputDMX(bool state);

	/** Set the value of one scene channel */
	void setSceneValue(const SceneValue& scv);

protected slots:
	void slotValueChanged(t_channel channel, t_value value, bool enabled);

signals:
	void valueChanged(t_fixture_id fxi, t_channel channel, t_value value,
			  bool enabled);

	/*********************************************************************
	 * Save / Load
	 *********************************************************************/
public:
	bool loadXML(QDomDocument* doc, QDomElement* root);
	bool saveXML(QDomDocument* doc, QDomElement* fxi_root);
};

#endif
