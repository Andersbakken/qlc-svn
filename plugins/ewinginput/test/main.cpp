#include <QtTest>

#include "testeplaybackwing.h"
#include "testeshortcutwing.h"
#include "testeprogramwing.h"

int main(int argc, char** argv)
{
    QApplication qapp(argc, argv);
    int r;

    TestEPlaybackWing test1;
    r = QTest::qExec(&test1, argc, argv);
    if (r != 0)
        return r;

    TestEShortcutWing test2;
    r = QTest::qExec(&test2, argc, argv);
    if (r != 0)
        return r;

    TestEProgramWing test3;
    r = QTest::qExec(&test3, argc, argv);
    if (r != 0)
        return r;

    return 0;
}
