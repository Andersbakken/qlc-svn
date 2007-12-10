/****************************************************************************
** Form interface generated from reading ui file 'configurellaout.ui'
**
** Created: Mon Dec 10 13:23:31 2007
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
class QListView;
class QListViewItem;
class QPushButton;

class UI_ConfigureLlaOut : public QDialog
{
    Q_OBJECT

public:
    UI_ConfigureLlaOut( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~UI_ConfigureLlaOut();

    QListView* m_listView;
    QPushButton* m_testButton;
    QPushButton* m_refreshButton;
    QPushButton* m_ok;
    QPushButton* m_cancel;

public slots:
    virtual void slotTestToggled(bool);
    virtual void slotRefreshClicked();

protected:
    QVBoxLayout* UI_ConfigureLlaOutLayout;
    QHBoxLayout* layout2;
    QSpacerItem* spacer2;

protected slots:
    virtual void languageChange();

};

#endif // UI_CONFIGURELLAOUT_H
