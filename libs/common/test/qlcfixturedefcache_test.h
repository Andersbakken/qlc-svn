#ifndef QLCFIXTUREDEFCACHE_TEST_H
#define QLCFIXTUREDEFCACHE_TEST_H

#include <QObject>

class QLCFixtureDefCache_Test : public QObject
{
	Q_OBJECT

private slots:
	void add();
	void fixtureDef();
	void load();
	void clear();
};

#endif
