#ifndef QLCFIXTUREMODE_TEST_H
#define QLCFIXTUREMODE_TEST_H

#include <QObject>

class QLCFixtureDef;
class QLCChannel;

class QLCFixtureMode_Test : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();
	void name();
	void physical();
	void insertChannel();
	void removeChannel();
	void channelByName();
	void channelByIndex();
	void channels();
	void channelNumber();
	void copy();
	void cleanupTestCase();
	void load();
	void loadWrongRoot();
	void loadNoName();

private:
	QLCFixtureDef* m_fixtureDef;
	QLCChannel* m_ch1;
	QLCChannel* m_ch2;
	QLCChannel* m_ch3;
	QLCChannel* m_ch4;
};

#endif
