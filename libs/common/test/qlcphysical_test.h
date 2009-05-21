#ifndef QLCPHYSICAL_TEST_H
#define QLCPHYSICAL_TEST_H

#include <QObject>
#include "../qlcphysical.h"

class QLCPhysical_Test : public QObject
{
	Q_OBJECT

private slots:
	void bulbType();
	void bulbLumens();
	void bulbColourTemp();

	void weight();
	void width();
	void height();
	void depth();

	void lensName();
	void lensDegreesMin();
	void lensDegreesMax();

	void focusType();
	void focusPanMax();
	void focusTiltMax();

	void copy();

private:
	QLCPhysical p;
};

#endif
