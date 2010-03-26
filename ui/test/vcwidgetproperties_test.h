#ifndef VCWIDGETPROPERTIES_TEST_H
#define VCWIDGETPROPERTIES_TEST_H

#include <QObject>

class VCWidgetProperties_Test : public QObject
{
	Q_OBJECT

private slots:
	void stateAndVisibility();
	void xy();
	void wh();
	void load();
};

#endif
