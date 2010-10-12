#ifndef TESTEPROGRAMWING_H
#define TESTEPROGRAMWING_H

#include <QByteArray>
#include <QObject>

class EProgramWing;

class TestEProgramWing : public QObject
{
    Q_OBJECT

protected:
    QByteArray data();

private slots:
    void initTestCase();

    void firmware();
    void address();
    void isOutputData();
    void name();
    void infoText();
    void tooShortData();

    void buttons_data();
    void buttons();

    void encoders_data();
    void encoders();

    void cleanupTestCase();

private:
    EProgramWing* m_ewing;
};

#endif
