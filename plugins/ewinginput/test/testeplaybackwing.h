#ifndef TESTEPLAYBACKWING_H
#define TESTEPLAYBACKWING_H

#include <QByteArray>
#include <QObject>

class EPlaybackWing;

class TestEPlaybackWing : public QObject
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

    void faders_data();
    void faders();

    void cleanupTestCase();

private:
    EPlaybackWing* m_ewing;
};

#endif
