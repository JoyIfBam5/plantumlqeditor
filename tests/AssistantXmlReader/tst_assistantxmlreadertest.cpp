#include <QString>
#include <QtTest>
#include "assistantxmlreader.h"

class AssistantXmlReaderTest : public QObject
{
    Q_OBJECT

public:
    AssistantXmlReaderTest();

private Q_SLOTS:
    // refs #186 - Assistant.xml's parser removes the last line from the CDATA block
    void testRemoveWithespace();
    void testRemoveWithespace_data();
};

AssistantXmlReaderTest::AssistantXmlReaderTest()
{
}

void AssistantXmlReaderTest::testRemoveWithespace()
{
    QFETCH(QString, input);
    QFETCH(QString, expected_output);
    QCOMPARE(AssistantXmlReader::removeWhiteSpace(input), expected_output);
}

void AssistantXmlReaderTest::testRemoveWithespace_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected_output");

    QTest::newRow("a null string returns a null string")
            << QString() << QString();
    QTest::newRow("1 line is ok")
            << "foo" << "foo";
    QTest::newRow("2 lines are ok")
            << "foo\nbar\n" << "foo\nbar";
    QTest::newRow("empty lines at start of the block are removed")
            << "\n\nfoo\nbar" << "foo\nbar";
    QTest::newRow("empty lines at end of the block are removed")
            << "foo\nbar\n\n" << "foo\nbar";
    QTest::newRow("empty lines at start and at end of the block are removed")
            << "\n\nfoo\nbar\n\n" << "foo\nbar";
    QTest::newRow("empty lines in the middle of the block are kept")
            << "foo\n\n\nbar" << "foo\n\n\nbar";
    QTest::newRow("whitespace at the start of the line is removed")
            << "  foo\n   bar" << "foo\nbar";
    QTest::newRow("whitespace at the end of the line is removed")
            << "foo    \nbar " << "foo\nbar";
}

QTEST_APPLESS_MAIN(AssistantXmlReaderTest)

#include "tst_assistantxmlreadertest.moc"
