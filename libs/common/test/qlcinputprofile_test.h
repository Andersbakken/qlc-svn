#ifndef QLCINPUTPROFILE_TEST_H
#define QLCINPUTPROFILE_TEST_H

#include <QObject>

class QLCInputProfile_Test : public QObject
{
	Q_OBJECT

private slots:
	void manufacturer();
	void model();
	void name();
	void addChannel();
	void removeChannel();
	void remapChannel();
	void channel();
	void channels();
	void copy();
	void load();
};

#endif
