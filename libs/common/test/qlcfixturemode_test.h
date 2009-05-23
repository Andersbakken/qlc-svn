#ifndef QLCFIXTUREMODE_TEST_H
#define QLCFIXTUREMODE_TEST_H

#include <QObject>

class QLCFixtureMode_Test : public QObject
{
	Q_OBJECT

private slots:
	void name();
	void physical();
	void insertChannel();
	void removeChannel();
	void channelByName();
	void channelByIndex();
	void channels();
	void channelNumber();
	void copy();
};

#endif
