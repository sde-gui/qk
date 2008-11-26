#include "@test@-t.h"
#include <moo/unit-tests.h>
#include <QtCore>

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
    QApplication app(argc, argv);
    moo::test::Tester t;
    return t.exec(argc, argv);
}

#include "@test@-test.moc"
