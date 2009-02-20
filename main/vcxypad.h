/*
  Q Light Controller
  vcxypad.h

  Copyright (c) Stefan Krumm, Heikki Junnila

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

#ifndef VCXYPAD_H
#define VCXYPAD_H

#include <QPixmap>
#include <QList>

#include "vcwidget.h"
#include "common/qlctypes.h"

class QDomDocument;
class QDomElement;
class QPaintEvent;
class QMouseEvent;
class QString;
class QFile;

class XYChannelUnit;

#define KXMLQLCVCXYPad "XYPad"
#define KXMLQLCVCXYPadPosition "Position"
#define KXMLQLCVCXYPadPositionX "X"
#define KXMLQLCVCXYPadPositionY "Y"

#define KXMLQLCVCXYPadChannel "Channel"
#define KXMLQLCVCXYPadChannelAxis "Axis"
#define KXMLQLCVCXYPadChannelAxisX "X"
#define KXMLQLCVCXYPadChannelAxisY "Y"

#define KXMLQLCVCXYPadChannelFixture "Fixture"
#define KXMLQLCVCXYPadChannelLowLimit "LowLimit"
#define KXMLQLCVCXYPadChannelHighLimit "HighLimit"
#define KXMLQLCVCXYPadChannelReverse "Reverse"

class VCXYPad : public VCWidget
{
	Q_OBJECT

	/*********************************************************************
	 * Initialization
	 *********************************************************************/
public:
	VCXYPad(QWidget* parent);
	virtual ~VCXYPad();

private:
	Q_DISABLE_COPY(VCXYPad)

	/*********************************************************************
	 * Properties
	 *********************************************************************/
public slots:
	/** Display a properties dialog */
	void slotProperties();

	/*********************************************************************
	 * Channels
	 *********************************************************************/
public:
	/**
	 * Append a new channel to this pad
	 *
	 * @param axis X or Y axis channel
	 * @param fixture The ID of the fixture, whose channel to append
	 * @param channel The fixture channel to append
	 * @lowLimit The lowest value that the channel can have with this pad
	 * @param highLimit The highest value the channel can get with this pad
	 * @param reverse Treat the axis reversed (up<->down,left<->right)
	 */
	void appendChannel(t_axis axis, t_fixture_id fixture, t_channel channel,
			   t_value lowLimit, t_value highLimit, bool reverse);

	/**
	 * Remove a channel from this pad
	 *
	 * @param axis X or Y axis channel
	 * @param fixture The ID of the fixture, whose channel to remove
	 * @param channel The fixture channel to remove
	 **/
	void removeChannel(t_axis axis, t_fixture_id fixture, t_channel channel);

	/**
	 * Get a certain channel unit from this pad
	 *
	 * @param axis X or Y axis channel
	 * @param fixture The ID of the fixture to look up
	 * @param channel The fixture channel to get
	 **/
	XYChannelUnit* channel(t_axis axis, t_fixture_id fixture,
			       t_channel channel);

	/** Clear all channel lists from this pad */
	void clearChannels();

	/** Get the pad's list of X axis channels */
	QList <XYChannelUnit*>* channelsX() { return &m_channelsX; }

	/** Get the pad's list of Y axis channels */
	QList <XYChannelUnit*>* channelsY() { return &m_channelsY; }

protected:
	QList <XYChannelUnit*> m_channelsX;
	QList <XYChannelUnit*> m_channelsY;

	/*********************************************************************
	 * Current position
	 *********************************************************************/
public:
	/** Get the pad's current position (i.e. where the point is) */
	const QPoint currentXYPosition() const { return m_currentXYPosition; }

	/** Set the pad's current position (i.e. move the point) */
	void setCurrentXYPosition(const QPoint& point);

	/** Same stuff as above, but with separate x & y integers */
	void setCurrentXYPosition(int x, int y);

protected:
	QPoint m_currentXYPosition;
	QPixmap m_xyPosPixmap;

	/*********************************************************************
	 * Load & Save
	 *********************************************************************/
public:
	/** Create & load a VCXYPad from the given XML node
	 *
	 * @param doc A QDomDocument containing a QLC workspace
	 * @param root A QDomElement containing a VCXYPad node tree
	 * @param parent The parent widget that the loaded pad belongs to 
	 */
	static bool loader(QDomDocument* doc, QDomElement* root,
			   QWidget* parent);

	/**
	 * Load a VCXYPad's contents from the given XML node
	 *
	 * @param doc A QDomDocument containing a QLC workspace
	 * @param root A QDomElement containing a VCXYPad node tree
	 */
	bool loadXML(QDomDocument* doc, QDomElement* root);

	/**
	 * Save a VCXYPad's contents to the given XML node
	 *
	 * @param doc A QDomDocument containing a QLC workspace
	 * @param root A QDomElement where to create this pad's VCXYPad node
	 */
	bool saveXML(QDomDocument* doc, QDomElement* root);

	/*********************************************************************
	 * DMX writer
	 *********************************************************************/
protected:
	/** Write DMX data according to the given XY position */
	void outputDMX(int x, int y);

	/*********************************************************************
	 * Event handlers
	 *********************************************************************/
protected:
	void paintEvent(QPaintEvent* e);

	void mousePressEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
};

#endif
