#include "assistantxmlreader.h"
#include <gtest/gtest.h>

struct AssistantXmlReaderTestData {
    QString input;
    QString expectedOutput;
};

class AssistantXmlReaderTest : public ::testing::TestWithParam<AssistantXmlReaderTestData> {};

TEST_P(AssistantXmlReaderTest, testRemoveWhitespace) {
    AssistantXmlReaderTestData data = GetParam();
    EXPECT_EQ(data.expectedOutput, AssistantXmlReader::removeWhiteSpace(data.input));
}

INSTANTIATE_TEST_CASE_P(Test,
                        AssistantXmlReaderTest,
                        ::testing::Values(
  (AssistantXmlReaderTestData) { "", "" } // null string
, (AssistantXmlReaderTestData) { "foo", "foo" } // 1 line
, (AssistantXmlReaderTestData) { "foo\nbar", "foo\nbar"} // 2 lines
, (AssistantXmlReaderTestData) { "\n\nfoo\nbar", "foo\nbar"} // empty lines at start of the block are removed
, (AssistantXmlReaderTestData) { "foo\nbar\n\n", "foo\nbar"} // empty lines at end of the block are removed
, (AssistantXmlReaderTestData) { "\n\nfoo\nbar\n\n", "foo\nbar"} // empty lines at start and at end of the block are removed
, (AssistantXmlReaderTestData) { "foo\n\n\nbar", "foo\n\n\nbar"} // empty lines in the middle of the block are kept
, (AssistantXmlReaderTestData) { "  foo\n   bar", "foo\nbar"} // whitespace at the start of the line is removed
, (AssistantXmlReaderTestData) { "foo    \nbar ", "foo\nbar"} // whitespace at the end of the line is removed
));
