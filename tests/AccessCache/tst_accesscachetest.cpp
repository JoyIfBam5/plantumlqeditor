#include <QString>
#include <QtTest>

class AccessCacheTest : public QObject
{
    Q_OBJECT

public:
    AccessCacheTest();

private Q_SLOTS:
    void testCase1();
};

AccessCacheTest::AccessCacheTest()
{
}

void AccessCacheTest::testCase1()
{
    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(AccessCacheTest)

#include "tst_accesscachetest.moc"
