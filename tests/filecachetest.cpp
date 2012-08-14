#include "filecache.h"
#include "config.h"
#include <QDir>
#include <gmock/gmock.h>

using ::testing::_;

//------------------------------------------------------------------------------

class MockFileCacheItem : public AbstractFileCacheItem
{
public:
    explicit MockFileCacheItem(const QString& path, const QString& key, int cost, const QDateTime& date_time = QDateTime(), QObject* parent = 0)
        : AbstractFileCacheItem(path, key, cost, date_time, parent) {}

    MOCK_CONST_METHOD1(removeFileFromDisk, void(const QString&));
};

//------------------------------------------------------------------------------

TEST(FileCache, testMaxCost) {
    FileCache cache;
    EXPECT_EQ(0, cache.maxCost());
    cache.setMaxCost(100);
    EXPECT_EQ(100, cache.maxCost());
    EXPECT_EQ(200, FileCache(200).maxCost());
}

TEST(FileCache, testItemFoundAfterItIsAdded) {
    FileCache cache(100);
    EXPECT_FALSE(cache.hasItem("foo"));
    cache.addItem(new MockFileCacheItem("", "foo", 10));
    EXPECT_TRUE(cache.hasItem("foo"));
}

TEST(FileCache, testItemIsParentedWhenAdded) {
    FileCache cache(100);
    AbstractFileCacheItem* item = new MockFileCacheItem("", "foo", 10);
    cache.addItem(item);
    EXPECT_EQ(&cache, item->parent());
}

TEST(FileCache, testTotalCostIncreasesAfterItemIsAdded) {
    FileCache cache(100);
    EXPECT_EQ(0, cache.totalCost());
    cache.addItem(new MockFileCacheItem("", "foo", 10));
    EXPECT_EQ(10, cache.totalCost());
}

TEST(FileCache, testSizeIncreasesAfterItemIsAdded) {
    FileCache cache(100);
    EXPECT_EQ(0, cache.size());
    cache.addItem(new MockFileCacheItem("", "foo", 10));
    EXPECT_EQ(1, cache.size());
}

TEST(FileCache, testRetrieveItemAfterItemIsAdded) {
    const char* KEY = "foo";
    const int COST = 10;
    const QDateTime DATE_TIME(QDate(2012, 8, 1), QTime(0, 0));
    FileCache cache(100);
    cache.addItem(new MockFileCacheItem("", KEY, COST, DATE_TIME));
    const AbstractFileCacheItem* actual = cache.item(KEY);
    EXPECT_EQ(KEY, actual->key());
    EXPECT_EQ(COST, actual->cost());
    EXPECT_EQ(DATE_TIME, actual->dateTime());
}

TEST(FileCache, testOlderItemsAreRemovedToMakeRoomForNewerOnes) {
    FileCache cache(100);

    MockFileCacheItem* item1 = new MockFileCacheItem("/foo", "item1", 10, QDateTime(QDate(2010, 1, 1), QTime(0, 0)));
    EXPECT_CALL(*item1, removeFileFromDisk(QString("/foo/item1"))).Times(1);

    cache.addItem(item1);
    cache.addItem(new MockFileCacheItem("", "item2", 40, QDateTime(QDate(2010, 1, 2), QTime(0, 0))));
    cache.addItem(new MockFileCacheItem("", "item3", 55, QDateTime(QDate(2010, 1, 3), QTime(0, 0)))); // forces "item1" out

    EXPECT_EQ(2, cache.size());
    EXPECT_EQ(95, cache.totalCost());
    EXPECT_EQ(QSet<QString>::fromList(QList<QString>() << "item2" << "item3"),
              QSet<QString>::fromList(cache.keys()));
}

TEST(FileCache, testFileIsNotRemoveOnlyBecauseTheCacheIsDestroyed) {
    FileCache cache(100);
    MockFileCacheItem* item = new MockFileCacheItem("", "item", 10, QDateTime(QDate(2010, 1, 1), QTime(0, 0)));
    EXPECT_CALL(*item, removeFileFromDisk(_)).Times(0);
    cache.addItem(item);
}

TEST(FileCache, testClearFromDiskRemovesFilesFromDisk) {
    FileCache cache(100);
    MockFileCacheItem* item = new MockFileCacheItem("/bar", "foo", 10);
    EXPECT_CALL(*item, removeFileFromDisk(QString("/bar/foo"))).Times(1);
    cache.addItem(item);
    cache.clearFromDisk();
    EXPECT_EQ(0, cache.size());
    EXPECT_EQ(0, cache.totalCost());
}

TEST(FileCache, testClearDoenstRemovesFilesFromDisk) {
    FileCache cache(100);
    MockFileCacheItem* item = new MockFileCacheItem("", "foo", 10);
    EXPECT_CALL(*item, removeFileFromDisk(_)).Times(0);
    cache.addItem(item);
    cache.clear();
    EXPECT_EQ(0, cache.size());
    EXPECT_EQ(0, cache.totalCost());
}

TEST(FileCache, testAddingAgainAnItemOnlyUpdatesCostAndDate) {
    const QString PATH1 = "/foo/item1";
    const QString PATH2 = "/foo/item2";
    const QString KEY1 = "item1";
    const QString KEY2 = "item2";

    const int COST1 = 10;
    const int COST2 = 35;
    const int COST3 = 15;

    const int MAX_COST = COST2 + COST3 + 5;

    const QDateTime DATE_TIME1(QDate(2010, 1, 1), QTime(0, 0));
    const QDateTime DATE_TIME2(QDate(2010, 1, 2), QTime(0, 0));
    const QDateTime DATE_TIME3(QDate(2010, 1, 3), QTime(0, 0));

    FileCache cache(MAX_COST);
    MockFileCacheItem* item1 = new MockFileCacheItem(PATH1, KEY1, COST1, DATE_TIME1);
    EXPECT_CALL(*item1, removeFileFromDisk(_)).Times(0);

    cache.addItem(item1);
    cache.addItem(new MockFileCacheItem(PATH2, KEY2, COST2, DATE_TIME2));
    cache.addItem(new MockFileCacheItem(PATH1, KEY1, COST3, DATE_TIME3));

    EXPECT_EQ(COST2 + COST3, cache.totalCost());
    EXPECT_EQ(QSet<QString>::fromList(QList<QString>() << KEY1 << KEY2),
              QSet<QString>::fromList(cache.keys()));
    EXPECT_EQ(COST3, cache.item(KEY1)->cost());
    EXPECT_EQ(DATE_TIME3, cache.item(KEY1)->dateTime());
}

TEST(FileCache, testCorrectFileIsDeletedFromDiskAfterUpdating) {
    const QString PATH = "/foo";
    const QString PATH2 = "/foo/item2";
    const QString KEY1 = "item1";
    const QString KEY2 = "item2";
    const QString KEY4 = "item4";

    const int COST1 = 10;
    const int COST2 = 35;
    const int COST3 = 15;
    const int COST4 = 40;

    const int MAX_COST = COST4 + COST3 + 5;

    const QDateTime DATE_TIME1(QDate(2010, 1, 1), QTime(0, 0));
    const QDateTime DATE_TIME2(QDate(2010, 1, 2), QTime(0, 0));
    const QDateTime DATE_TIME3(QDate(2010, 1, 3), QTime(0, 0));
    const QDateTime DATE_TIME4(QDate(2010, 1, 4), QTime(0, 0));

    FileCache cache(MAX_COST);
    MockFileCacheItem* item1 = new MockFileCacheItem(PATH, KEY1, COST1, DATE_TIME1);
    EXPECT_CALL(*item1, removeFileFromDisk(_)).Times(0);

    MockFileCacheItem* item2 = new MockFileCacheItem(PATH, KEY2, COST2, DATE_TIME2);
    EXPECT_CALL(*item2, removeFileFromDisk(PATH2)).Times(1);

    MockFileCacheItem* item3 = new MockFileCacheItem(PATH, KEY1, COST3, DATE_TIME3);
    EXPECT_CALL(*item3, removeFileFromDisk(_)).Times(0);

    MockFileCacheItem* item4 = new MockFileCacheItem(PATH, KEY4, COST4, DATE_TIME4);
    EXPECT_CALL(*item4, removeFileFromDisk(_)).Times(0);

    cache.addItem(item1);
    cache.addItem(item2);
    cache.addItem(item3);
    cache.addItem(item4);

    EXPECT_EQ(COST3 + COST4, cache.totalCost());
    EXPECT_EQ(QSet<QString>::fromList(QList<QString>() << KEY1 << KEY4),
              QSet<QString>::fromList(cache.keys()));
}

TEST(FileCache, testAddingSameItemTwiceThrowsException) {
    FileCache cache(100);
    AbstractFileCacheItem* item = new MockFileCacheItem("", "foo", 10);
    EXPECT_NO_THROW(cache.addItem(item));
    EXPECT_THROW(cache.addItem(item), FileCacheError);
}

TEST(FileCache, testSetPath) {
    FileCache cache(100);
    cache.setPath(TEST_DIR1, [](const QString& path,
                                const QString& key,
                                int cost,
                                const QDateTime& date_time,
                                QObject* parent
                                ) { return new MockFileCacheItem(path, key, cost, date_time, parent); });
    EXPECT_EQ(38, cache.totalCost());
    EXPECT_EQ(QSet<QString>::fromList(QList<QString>()
                                      << "item1.png"
                                      << "item1.svg"
                                      << "item2.svg"
                                      << "item3.png"
                                      << "item3.svg"
                                      << "item4.svg"
                                      << "item5.svg"
                                      ),
              QSet<QString>::fromList(cache.keys()));

    EXPECT_EQ(QFileInfo(QDir(TEST_DIR1), "item1.svg").absoluteFilePath(),
              cache.item("item1.svg")->path());
}
