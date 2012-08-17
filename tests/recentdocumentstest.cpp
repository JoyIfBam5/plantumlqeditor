#include "recentdocuments.h"
#include <QAction>
#include <gmock/gmock.h>

TEST(RecentDocuments, testDefaultActions) {
    int MAX_DOCUMENTS = 2;
    RecentDocuments documents(MAX_DOCUMENTS);
    QList<QAction*> actions = documents.actions();
    EXPECT_EQ(MAX_DOCUMENTS + 2, actions.size());
    EXPECT_TRUE(actions[MAX_DOCUMENTS]->isSeparator());
}

TEST(RecentDocuments, testAccessOneFile) {
    int MAX_DOCUMENTS = 2;
    RecentDocuments documents(MAX_DOCUMENTS);
    documents.accessing("foo");
    EXPECT_EQ("1. foo", documents.actions()[0]->text());
    EXPECT_EQ("", documents.actions()[1]->text());
}

TEST(RecentDocuments, testAccessTwoFiles) {
    int MAX_DOCUMENTS = 2;
    RecentDocuments documents(MAX_DOCUMENTS);
    documents.accessing("foo");
    documents.accessing("bar");
    EXPECT_EQ("1. bar", documents.actions()[0]->text());
    EXPECT_EQ("2. foo", documents.actions()[1]->text());
}

TEST(RecentDocuments, testAccessThreeFiles) {
    int MAX_DOCUMENTS = 2;
    RecentDocuments documents(MAX_DOCUMENTS);
    documents.accessing("foo");
    documents.accessing("bar");
    documents.accessing("baz");
    EXPECT_EQ("1. baz", documents.actions()[0]->text());
    EXPECT_EQ("2. bar", documents.actions()[1]->text());
}

TEST(RecentDocuments, testAccessSameFileMoreThanOnce) {
    int MAX_DOCUMENTS = 2;
    RecentDocuments documents(MAX_DOCUMENTS);
    documents.accessing("foo");
    documents.accessing("bar");
    documents.accessing("foo");
    EXPECT_EQ("1. foo", documents.actions()[0]->text());
    EXPECT_EQ("2. bar", documents.actions()[1]->text());
}

TEST(RecentDocuments, testClear) {
    int MAX_DOCUMENTS = 2;
    RecentDocuments documents(MAX_DOCUMENTS);
    documents.accessing("foo");
    documents.accessing("bar");
    documents.clear();
    EXPECT_EQ("", documents.actions()[0]->text());
    EXPECT_EQ("", documents.actions()[1]->text());
}
