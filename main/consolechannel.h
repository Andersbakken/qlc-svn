/*
  Q Light Controller
  consolechannel.h

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

#ifndef CONSOLECHANNEL_H
#define CONSOLECHANNEL_H

#include <QGroupBox>
#include <QMutex>
#include <QIcon>

#include "qlctypes.h"
#include "dmxsource.h"

class QContextMenuEvent;
class QIntValidator;
class QToolButton;
class QVBoxLayout;
class QLineEdit;
class QSlider;
class QLabel;
class QMenu;

class QLCChannel;
class Fixture;

class ConsoleChannel : public QGroupBox, public DMXSource
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	ConsoleChannel(QWidget *parent, t_fixture_id fixtureID,
		       t_channel channel);
	~ConsoleChannel();

private:
	Q_DISABLE_COPY(ConsoleChannel)

protected:
	/** Initialize the UI */
	void init();

	/*********************************************************************
	 * Menu
	 *********************************************************************/
protected slots:
	void slotContextMenuTriggered(QAction* action);

protected:
	/** Open a context menu */
	void contextMenuEvent(QContextMenuEvent*);

	/** Initialize the context menu */
	void initMenu();

	/** Initialize the context menu for fixtures with capabilities */
	void initCapabilityMenu(const QLCChannel* ch);

protected:
	const QIcon colorIcon(const QString& name);

protected:
	QMenu* m_menu;

	/*********************************************************************
	 * Value
	 *********************************************************************/
public:
	/** Get the channel's value */
	int sliderValue() const;

	/** Update the UI to match the channel's real status & value */
	void updateValue();

	/** Enable/disable DMX value output */
	void setOutputDMX(bool state);

	/** Emulate the user dragging the value slider */
	void setValue(t_value value);

public slots:
	/** Value edit box was edited */
	void slotValueEdited(const QString& text);

	/** Slider value was changed */
	void slotValueChange(int value);

signals:
	/** Slider value has changed */
	void valueChanged(t_channel channel, t_value value, bool enabled);

protected:
	t_value m_value;
	bool m_valueChanged;
	QMutex m_valueChangedMutex;
	bool m_outputDMX;

	/*********************************************************************
	 * DMXSource
	 *********************************************************************/
public:
	/** \reimp */
	void writeDMX(QByteArray* universes);

	/*********************************************************************
	 * Enable/disable
	 *********************************************************************/
public:
	/** Enable/disable the channel */
	void enable(bool state);

	/** Check, whether this channel has been enabled */
	bool isEnabled() const { return isChecked(); }

protected slots:
	/** This channel was checked/unchecked */
	void slotToggled(bool state);

	/*********************************************************************
	 * Checkable
	 *********************************************************************/
public:
	void setCheckable(bool checkable);

	 /*********************************************************************
	 * Fixture channel
	 *********************************************************************/
public:
	t_channel channel() const { return m_channel; }

protected:
	t_channel m_channel;
	t_fixture_id m_fixtureID;
	Fixture* m_fixture;

	/*********************************************************************
	 * Widgets
	 *********************************************************************/
protected:
	QToolButton* m_presetButton;
	QIntValidator* m_validator;
	QLineEdit* m_valueEdit;
	QSlider* m_valueSlider;
	QLabel* m_numberLabel;
};

#endif
