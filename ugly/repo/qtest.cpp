#include "@test@-t.h"
#include <moo/unit-tests.h>
#include <QtCore>

#ifndef UnitTestMainClass
#error "`UnitTestMainClass' not defined"
#endif

#ifndef UnitTestAppClass
#define UnitTestAppClass QApplication
#endif

namespace moo {
namespace test {

class Tester {
public:
    int exec(int argc, char *argv[])
    {
        UnitTestMainClass test;
        return QTest::qExec(&test, argc, argv);
    }
};

}
}

int main(int argc, char *argv[])
{
    UnitTestAppClass app(argc, argv);
    moo::test::Tester t;
    return t.exec(argc, argv);
}

#include "@test@-test.moc"
