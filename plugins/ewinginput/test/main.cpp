#include <QtTest>

#include "testeplaybackwing.h"
#include "testeshortcutwing.h"
#include "testeprogramwing.h"

int main(int argc, char** argv)
{
    QApplication qapp(argc, argv);

    TestEPlaybackWing test1;
    QTest::qExec(&test1, argc, argv);

    TestEShortcutWing test2;
    QTest::qExec(&test2, argc, argv);

    TestEProgramWing test3;
    QTest::qExec(&test3, argc, argv);
}
