#ifndef TESTESHORTCUTWING_H
#define TESTESHORTCUTWING_H

#include <QByteArray>
#include <QObject>

class EShortcutWing;

class TestEShortcutWing : public QObject
{
	Q_OBJECT

protected:
	QByteArray data();

private slots:
	void initTestCase();

	void firmware();
	void address();
	void isOutputData();

	void buttons_data();
	void buttons();

	void cleanupTestCase();

private:
	EShortcutWing* m_ewing;
};

#endif
