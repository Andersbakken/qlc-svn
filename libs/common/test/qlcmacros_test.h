#ifndef QLCMACROS_TEST_H
#define QLCMACROS_TEST_H

#include <QObject>

class QLCMacros_Test : public QObject
{
	Q_OBJECT

private slots:
	void min();
	void max();
	void clamp();
	void scale();
};

#endif
