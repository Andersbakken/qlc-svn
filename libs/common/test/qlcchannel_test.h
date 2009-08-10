#ifndef QLCCHANNEL_TEST_H
#define QLCCHANNEL_TEST_H

#include <QObject>

class QLCChannel_Test : public QObject
{
	Q_OBJECT

private slots:
	void groupList();
	void name();
	void group();
	void controlByte();
	void searchCapabilityByValue();
	void searchCapabilityByName();
	void addCapability();
	void removeCapability();
	void sortCapabilities();
	void copy();
	void load();
	void loadWrongRoot();
	void save();
};

#endif
