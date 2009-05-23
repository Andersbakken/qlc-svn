#ifndef QLCCAPABILITY_TEST_H
#define QLCCAPABILITY_TEST_H

#include <QObject>
#include "../qlccapability.h"

class QLCCapability_Test : public QObject
{
	Q_OBJECT

private slots:
	void initial();

	void min_data();
	void min();

	void max_data();
	void max();

	void name();
	void overlaps();
	void copy();
};

#endif
