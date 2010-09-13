
# Code coverage measurement with gcov
QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
LIBS += -lgcov
QMAKE_CLEAN += *.gcno *.gcov
QMAKE_DISTCLEAN += *.gcno *.gcov *.gcda

gcov.target = gcov
gcov.commands += gcov -b $$GCOV_FILES
QMAKE_EXTRA_TARGETS += gcov

lcovbase.target = lcovbase
lcovbase.commands += lcov -c -i -d ./ -o base.info
QMAKE_EXTRA_TARGETS += lcovbase
QMAKE_CLEAN += base.info
QMAKE_DISTCLEAN += base.info

lcovtest.target = lcovtest
lcovtest.commands += lcov -c -d ./ -o test.info
QMAKE_EXTRA_TARGETS += lcovtest
QMAKE_CLEAN += test.info
QMAKE_DISTCLEAN += test.info

lcovmerge.target = lcovmerge
lcovmerge.commands += lcov -a base.info -a test.info -o total.info
QMAKE_EXTRA_TARGETS += lcovmerge
QMAKE_CLEAN += total.info
QMAKE_DISTCLEAN += total.info
