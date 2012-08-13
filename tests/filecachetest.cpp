#include <QString>
#include <QDateTime>
#include <QMap>
#include <QSet>
#include <gtest/gtest.h>

class FileCacheItem
{
public:
    explicit FileCacheItem(const QString& key, int cost, const QDateTime& date_time);
    explicit FileCacheItem();

    const QString& key() const { return m_key; }
    int cost() const { return m_cost; }
    const QDateTime& dateTime() const { return m_dateTime; }

    bool isValid() const { return m_dateTime.isValid() && !m_key.isEmpty(); }

private:
    QString m_key;
    int m_cost;
    QDateTime m_dateTime;
};

FileCacheItem::FileCacheItem(const QString &key, int cost, const QDateTime &date_time)
    : m_key(key), m_cost(cost), m_dateTime(date_time)
{
}

FileCacheItem::FileCacheItem()
    : m_cost(0)
{
}

//------------------------------------------------------------------------------

class FileCache
{
public:
    FileCache(int maxCost = 0);

    int maxCost() const { return m_maxCost; }
    void setMaxCost(int max_cost);

    bool hasItem(const QString& key) const;
    void addItem(const QString& key, int cost, const QDateTime& date_time);

    int totalCost() const { return m_totalCost; }

    int size() const { return m_items.size(); }
    QList<QString> keys() const { return m_items.keys(); }
    FileCacheItem item(const QString& key) const { return m_items.value(key); }

private:
    int m_maxCost;
    int m_totalCost;
    QMap<QString, FileCacheItem> m_items;
    QList<QString> m_indexByDate;
};

FileCache::FileCache(int size)
    : m_maxCost(size)
    , m_totalCost(0)
{
}

void FileCache::setMaxCost(int max_cost)
{
    m_maxCost = max_cost;
}

bool FileCache::hasItem(const QString &key) const
{
    return m_items.contains(key);
}

void FileCache::addItem(const QString &key, int cost, const QDateTime &date_time)
{
    m_items[key] = FileCacheItem(key, cost, date_time);
    m_totalCost += cost;

    bool index_by_date_updated = false;
    for(int index = 0; index < m_indexByDate.size(); index++) {
        if (m_items[m_indexByDate[index]].dateTime() > date_time) {
            m_indexByDate.insert(index, key);
            index_by_date_updated = true;
            break;
        }
    }
    if (!index_by_date_updated) {
        m_indexByDate.append(key);
    }

    while (m_totalCost > m_maxCost && m_indexByDate.size() > 1) {
        QString tmp_key = m_indexByDate.value(0);
        m_totalCost -= m_items.value(tmp_key).cost();
        m_items.remove(tmp_key);
        m_indexByDate.removeAt(0);
    }
}

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
    cache.addItem("foo", 10, QDateTime());
    EXPECT_TRUE(cache.hasItem("foo"));
}

TEST(FileCache, testTotalCostIncreasesAfterItemIsAdded) {
    FileCache cache(100);
    EXPECT_EQ(0, cache.totalCost());
    cache.addItem("foo", 10, QDateTime());
    EXPECT_EQ(10, cache.totalCost());
}

TEST(FileCache, testSizeIncreasesAfterItemIsAdded) {
    FileCache cache(100);
    EXPECT_EQ(0, cache.size());
    cache.addItem("foo", 10, QDateTime());
    EXPECT_EQ(1, cache.size());
}

TEST(FileCache, testRetrieveItemAfterItemIsAdded) {
    const char* KEY = "foo";
    const int COST = 10;
    const QDateTime DATE_TIME(QDate(2012, 8, 1), QTime(0, 0));
    FileCache cache(100);
    cache.addItem(KEY, COST, DATE_TIME);
    FileCacheItem actual = cache.item(KEY);
    EXPECT_EQ(KEY, actual.key());
    EXPECT_EQ(COST, actual.cost());
    EXPECT_EQ(DATE_TIME, actual.dateTime());
}

TEST(FileCache, testOlderItemsAreRemovedToMakeRoomForNewerOnes) {
    FileCache cache(100);
    cache.addItem("item1", 10, QDateTime(QDate(2010, 1, 1), QTime(0, 0)));
    cache.addItem("item2", 40, QDateTime(QDate(2010, 1, 2), QTime(0, 0)));
    cache.addItem("item3", 55, QDateTime(QDate(2010, 1, 3), QTime(0, 0))); // forces "item1" out

    EXPECT_EQ(2, cache.size());
    EXPECT_EQ(95, cache.totalCost());
    EXPECT_EQ(QSet<QString>::fromList(QList<QString>() << "item2" << "item3"),
              QSet<QString>::fromList(cache.keys()));
}
