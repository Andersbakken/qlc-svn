#ifndef QLCWIDGETPROPERTIES_TEST_H
#define QLCWIDGETPROPERTIES_TEST_H

#include <QObject>

class QLCWidgetProperties_Test : public QObject
{
	Q_OBJECT

private slots:
	void stateAndVisibility();
	void xy();
	void wh();
	void load();
};

#endif
