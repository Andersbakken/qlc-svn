#ifndef QLCINPUTCHANNEL_TEST_H
#define QLCINPUTCHANNEL_TEST_H

#include <QObject>

class QLCInputChannel_Test : public QObject
{
	Q_OBJECT

private slots:
	void channel();
	void type();
	void name();
	void copy();
	void load();
	void loadWrongType();
};

#endif
