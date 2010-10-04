/*
  Q Light Controller
  efxeditor.h

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

#ifndef EFXEDITOR_H
#define EFXEDITOR_H

#include <QPolygon>
#include <QFrame>
#include <QTimer>
#include "qlctypes.h"

#include "ui_efxeditor.h"
#include "efx.h"

class QPaintEvent;
class EFXPreviewArea;

class EFXEditor : public QDialog, public Ui_EFXEditor
{
    Q_OBJECT

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    EFXEditor(QWidget* parent, EFX* efx);
    ~EFXEditor();

private:
    Q_DISABLE_COPY(EFXEditor)

protected:
    void initGeneralPage();
    void fillBusCombo();
    void initMovementPage();
    void initInitializationPage();

protected slots:
    void accept();

protected:
    EFXPreviewArea* m_previewArea;
    QPolygon* m_points;

    EFX* m_efx;
    EFX* m_original;

    /*********************************************************************
     * General page
     *********************************************************************/
protected:
    QTreeWidgetItem* fixtureItem(EFXFixture* ef);
    const QList <EFXFixture*> selectedFixtures() const;
    void updateIndices(int from, int to);
    void addFixtureItem(EFXFixture* ef);
    void removeFixtureItem(EFXFixture* ef);

protected slots:
    void slotNameEdited(const QString &text);
    void slotFixtureItemChanged(QTreeWidgetItem* item, int column);
    void slotAddFixtureClicked();
    void slotRemoveFixtureClicked();
    void slotRaiseFixtureClicked();
    void slotLowerFixtureClicked();

    void slotParallelRadioToggled(bool state);
    void slotBusComboActivated(int index);

    /*********************************************************************
     * Movement page
     *********************************************************************/
protected slots:
    void slotAlgorithmSelected(const QString &text);
    void slotWidthSpinChanged(int value);
    void slotHeightSpinChanged(int value);
    void slotXOffsetSpinChanged(int value);
    void slotYOffsetSpinChanged(int value);
    void slotRotationSpinChanged(int value);

    void slotXFrequencySpinChanged(int value);
    void slotYFrequencySpinChanged(int value);
    void slotXPhaseSpinChanged(int value);
    void slotYPhaseSpinChanged(int value);

    void slotLoopClicked();
    void slotSingleShotClicked();
    void slotPingPongClicked();

    void slotForwardClicked();
    void slotBackwardClicked();

protected:
    void redrawPreview();

    /*********************************************************************
     * Initialization page
     *********************************************************************/
protected:
    void fillSceneLists();

protected slots:
    void slotStartSceneGroupToggled(bool);
    void slotStopSceneGroupToggled(bool);

    void slotStartSceneListSelectionChanged();
    void slotStopSceneListSelectionChanged();
};

/**
 * The area that is used to draw a preview of
 * the EFX function currently being edited.
 */
class EFXPreviewArea : public QFrame
{
    Q_OBJECT

public:
    EFXPreviewArea(QWidget* parent);
    ~EFXPreviewArea();

    /**
     * Set an an array of X-Y points that can be used for drawing a preview
     *
     * @param points The point array
     */
    void setPoints(const QVector<QPoint>& points);

    /**
     * Tell the preview area to draw the points
     */
    void draw();

    /**
     * Set the preview area to draw the pattern in reverse or normal
     * order.
     *
     * @param reverse if true, the pattern is reversed
     */
    void setReverse(bool reverse) {
        m_reverse = reverse;
    }

    /**
     * Check, whether the preview are is drawing a reversed pattern
     */
    bool isReverse() const {
        return m_reverse;
    }

protected:
    /**
     * QT Framework calls this when the widget needs
     * to be repainted.
     *
     * @param e QPaintEvent
     */
    void paintEvent(QPaintEvent* e);

protected slots:
    /** Animation timeout */
    void slotTimeout();

protected:
    /** Points that are drawn in the preview area */
    QPolygon m_points;

    /** Animation timer */
    QTimer m_timer;

    /** Animation position */
    int m_iter;

    /** Reverse or normal order */
    bool m_reverse;
};


#endif
