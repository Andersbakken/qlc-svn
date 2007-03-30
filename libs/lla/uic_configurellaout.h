/****************************************************************************
** Form interface generated from reading ui file 'configurellaout.ui'
**
** Created: Mon Jan 8 19:21:03 2007
**      by: The User Interface Compiler ($Id$)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef UI_CONFIGURELLAOUT_H
#define UI_CONFIGURELLAOUT_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QGroupBox;
class QSpinBox;
class QLabel;
class QPushButton;

class UI_ConfigureLlaOut : public QDialog
{
    Q_OBJECT

public:
    UI_ConfigureLlaOut( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~UI_ConfigureLlaOut();

    QGroupBox* m_deviceGroup;
    QSpinBox* m_firstNumberSpinBox;
    QLabel* textLabel1_2;
    QGroupBox* m_statusGroup;
    QLabel* m_statusLabel;
    QPushButton* m_activate;
    QPushButton* m_ok;
    QPushButton* m_cancel;

public slots:
    virtual void slotActivateClicked();

protected:
    QVBoxLayout* UI_ConfigureLlaOutLayout;
    QGridLayout* m_deviceGroupLayout;
    QHBoxLayout* m_statusGroupLayout;
    QHBoxLayout* layout6;
    QSpacerItem* spacer2;

protected slots:
    virtual void languageChange();

};

#endif // UI_CONFIGURELLAOUT_H
