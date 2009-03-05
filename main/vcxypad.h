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

#include <QWidget>
#include <QPixmap>
#include <QString>
#include <QList>

#include "common/qlctypes.h"
#include "vcxypadfixture.h"
#include "vcwidget.h"

class QDomDocument;
class QDomElement;
class QPaintEvent;
class QMouseEvent;

#define KXMLQLCVCXYPad "XYPad"
#define KXMLQLCVCXYPadPosition "Position"
#define KXMLQLCVCXYPadPositionX "X"
#define KXMLQLCVCXYPadPositionY "Y"

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
	 * Clipboard
	 *********************************************************************/
public:
	/** Create a copy of this widget into the given parent */
	VCWidget* createCopy(VCWidget* parent);

	/** Copy the contents for this widget from another widget */
	bool copyFrom(VCWidget* widget);

	/*********************************************************************
	 * Properties
	 *********************************************************************/
public:
	/** Display a properties dialog */
	void editProperties();

	/*********************************************************************
	 * Fixtures
	 *********************************************************************/
public:
	void appendFixture(const VCXYPadFixture& fxi);
	void removeFixture(t_fixture_id fxi);
	void clearFixtures();

	QList <VCXYPadFixture> fixtures() const { return m_fixtures; }
	
protected:
	QList <VCXYPadFixture> m_fixtures;

	/*********************************************************************
	 * Current position
	 *********************************************************************/
public:
	/** Get the pad's current position (i.e. where the point is) */
	QPoint currentXYPosition() const { return m_currentXYPosition; }

	/** Set the pad's current position (i.e. move the point) */
	void setCurrentXYPosition(const QPoint& point);

protected:
	/** Write DMX data according to the given XY position */
	void outputDMX(const QPoint& point);

protected:
	QPoint m_currentXYPosition;
	QPixmap m_xyPosPixmap;

	/*********************************************************************
	 * Application Mode
	 *********************************************************************/
protected slots:
	void slotAppModeChanged(App::Mode mode);

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
	 * Event handlers
	 *********************************************************************/
protected:
	void paintEvent(QPaintEvent* e);

	void mousePressEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
};

#endif
