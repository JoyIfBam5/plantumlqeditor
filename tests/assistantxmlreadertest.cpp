#include "assistantxmlreader.h"
#include <gtest/gtest.h>
#include <QFile>
#include <QDebug>

namespace {
bool writeFile(const QString& data, const QString& file_name) {
    QFile file(file_name);
    if (!file.open(QFile::WriteOnly)) {
        return false;
    }
    file.write(data.toAscii());
    file.close();
    return true;
}
}

struct TrimLeftTestData {
    QString input;
    QString expectedOutput;
    int expectedReturn;
};

class TrimLeftTest : public ::testing::TestWithParam<TrimLeftTestData> {};

TEST_P(TrimLeftTest, testTrimLeft) {
    TrimLeftTestData data = GetParam();
    EXPECT_EQ(data.expectedReturn, AssistantXmlReader::trimLeft(data.input));
    EXPECT_EQ(data.expectedOutput, data.input);
}

INSTANTIATE_TEST_CASE_P(Test,
                        TrimLeftTest,
                        ::testing::Values(
  (TrimLeftTestData) { "", "", 0 } // null string
, (TrimLeftTestData) { "  ", "", 2 } // trim to nothing
, (TrimLeftTestData) { "foo", "foo", 0 } // nothing to trim
, (TrimLeftTestData) { "foo  ", "foo  ", 0 } // right side unaffected
, (TrimLeftTestData) { "foo  bar", "foo  bar", 0 } // middle unaffected
, (TrimLeftTestData) { "   foo", "foo", 3 } // left space remove
, (TrimLeftTestData) { " \t foo", "foo", 3 } // tabs count as one
, (TrimLeftTestData) { "   foo  bar ", "foo  bar ", 3 } // complete test
));

struct TrimRightTestData {
    QString input;
    QString expectedOutput;
};

class TrimRightTest : public ::testing::TestWithParam<TrimRightTestData> {};

TEST_P(TrimRightTest, testTrimRight) {
    TrimRightTestData data = GetParam();
    AssistantXmlReader::trimRight(data.input);
    EXPECT_EQ(data.expectedOutput, data.input);
}

INSTANTIATE_TEST_CASE_P(Test,
                        TrimRightTest,
                        ::testing::Values(
  (TrimRightTestData) { "", "" } // null string
, (TrimRightTestData) { "  ", "" } // trim to nothing
, (TrimRightTestData) { "foo", "foo" } // nothing to trim
, (TrimRightTestData) { "foo  ", "foo" } // right space removed
, (TrimRightTestData) { "foo  bar", "foo  bar" } // middle unaffected
, (TrimRightTestData) { "   foo", "   foo" } // left space unaffected
, (TrimRightTestData) { "   foo  bar ", "   foo  bar" } // complete test
));

struct RemoveWhitespaceTestData {
    QString input;
    QString expectedOutput;
};

class RemoveWhitespaceTest : public ::testing::TestWithParam<RemoveWhitespaceTestData> {};

TEST_P(RemoveWhitespaceTest, testRemoveWhitespace) {
    RemoveWhitespaceTestData data = GetParam();
    EXPECT_EQ(data.expectedOutput, AssistantXmlReader::removeWhiteSpace(data.input));
}

INSTANTIATE_TEST_CASE_P(Test,
                        RemoveWhitespaceTest,
                        ::testing::Values(
  (RemoveWhitespaceTestData) { "", "" } // null string
, (RemoveWhitespaceTestData) { "\n\n\n", "" } // only empty lines
, (RemoveWhitespaceTestData) { "foo", "foo" } // 1 line
, (RemoveWhitespaceTestData) { "foo\nbar", "foo\nbar"} // 2 lines
, (RemoveWhitespaceTestData) { "\n\n\nfoo\nbar", "foo\nbar"} // empty lines at start of the block are removed
, (RemoveWhitespaceTestData) { "foo\nbar\n\n", "foo\nbar"} // empty lines at end of the block are removed
, (RemoveWhitespaceTestData) { "\n\nfoo\nbar\n\n", "foo\nbar"} // empty lines at start and at end of the block are removed
, (RemoveWhitespaceTestData) { "foo\n\n\nbar", "foo\n\n\nbar"} // empty lines in the middle of the block are kept
, (RemoveWhitespaceTestData) { "  foo\n    bar\n  baz", "foo\n  bar\nbaz"} // whitespace at the start of the line is removed, but identation is kept
, (RemoveWhitespaceTestData) { "foo    \nbar ", "foo\nbar"} // whitespace at the end of the line is removed
, (RemoveWhitespaceTestData) { "\n\n\n  \n\t\t\n\n  foo  \n    bar   \n  baz   \n\n   \n", "foo\n  bar\nbaz"} // more whitespace on the right
));

TEST(AssistantXmlReader, testParsingCDATANotes) {
    const QString XML =
            "<assistants>\n"
            "<assistant name=\"assistant1\">\n"
            "<item name=\"item1\">\n"
            "<notes>Simple notes</notes>\n"
            "<![CDATA[\n"
            "  content\n"
            "    content\n"
            "  content\n"
            "]]>\n"
            "<notes><![CDATA[\n"
            "  notes\n"
            "    notes\n"
            "  notes\n"
            "]]>\n"
            "</notes>\n"
            "more content\n"
            "</item>\n"
            "</assistant>\n"
            "</assistants>\n"
            ;
    const QString FILE_NAME = "assistant.xml";
    writeFile(XML, FILE_NAME);
    AssistantXmlReader reader;
    reader.readFile(FILE_NAME);

    ASSERT_EQ(1, reader.size());
    const Assistant *assistant = reader.assistant(0);
    EXPECT_EQ("assistant1", assistant->name());
    ASSERT_EQ(1, assistant->size());
    const AssistantItem *item = assistant->item(0);
    EXPECT_EQ("item1", item->name());
    EXPECT_EQ("content\n  content\ncontent\nmore content", item->data());
    EXPECT_EQ("Simple notes\nnotes\n  notes\nnotes", item->notes());
}
