#ifndef QLCFIXTUREDEF_TEST_H
#define QLCFIXTUREDEF_TEST_H

#include <QObject>

class QLCFixtureDef_Test : public QObject
{
	Q_OBJECT

private slots:
	void initial();
	void manufacturer();
	void model();
	void name();
	void type();
	void addChannel();
	void removeChannel();
	void channel();
	void channels();
	void addMode();
	void removeMode();
	void mode();
	void modes();
	void copy();
};

#endif
