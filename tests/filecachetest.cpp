#include <QObject>
#include <QString>
#include <QDateTime>
#include <QMap>
#include <QSet>
#include <gmock/gmock.h>

class AbstractFileCacheItem : public QObject
{
public:
    explicit AbstractFileCacheItem(const QString& key, int cost, const QDateTime& date_time, QObject* parent = 0);
    virtual ~AbstractFileCacheItem() {}

    const QString& key() const { return m_key; }
    int cost() const { return m_cost; }
    const QDateTime& dateTime() const { return m_dateTime; }

    virtual void removeFileFromDisk() const = 0;

private:
    QString m_key;
    int m_cost;
    QDateTime m_dateTime;
};

AbstractFileCacheItem::AbstractFileCacheItem(const QString &key, int cost, const QDateTime &date_time, QObject* parent)
    : QObject(parent)
    , m_key(key)
    , m_cost(cost)
    , m_dateTime(date_time)
{
}

//------------------------------------------------------------------------------

class FileCache : public QObject
{
public:
    FileCache(int maxCost = 0, QObject* parent = 0);
    ~FileCache();

    int maxCost() const { return m_maxCost; }
    void setMaxCost(int max_cost);

    bool hasItem(const QString& key) const;
    void addItem(AbstractFileCacheItem* item);

    int totalCost() const { return m_totalCost; }

    int size() const { return m_items.size(); }
    QList<QString> keys() const { return m_items.keys(); }
    const AbstractFileCacheItem* item(const QString& key) const { return m_items.value(key); }

private:
    int m_maxCost;
    int m_totalCost;
    QMap<QString, AbstractFileCacheItem*> m_items;
    QList<QString> m_indexByDate;
};

FileCache::FileCache(int size, QObject *parent)
    : QObject(parent)
    , m_maxCost(size)
    , m_totalCost(0)
{
}

FileCache::~FileCache()
{
    foreach (AbstractFileCacheItem* item, m_items) {
        delete item;
    }
}

void FileCache::setMaxCost(int max_cost)
{
    m_maxCost = max_cost;
}

bool FileCache::hasItem(const QString &key) const
{
    return m_items.contains(key);
}

void FileCache::addItem(AbstractFileCacheItem *item)
{
    AbstractFileCacheItem* old_item = m_items.value(item->key());
    if (old_item) {
        m_totalCost += item->cost() - old_item->cost();
        m_indexByDate.removeOne(item->key());
        m_items.remove(item->key());
        delete old_item;
    } else {
        m_totalCost += item->cost();
    }

    m_items[item->key()] = item;

    bool index_by_date_updated = false;
    for(int index = 0; index < m_indexByDate.size(); index++) {
        if (m_items[m_indexByDate[index]]->dateTime() > item->dateTime()) {
            m_indexByDate.insert(index, item->key());
            index_by_date_updated = true;
            break;
        }
    }
    if (!index_by_date_updated) {
        m_indexByDate.append(item->key());
    }

    item->setParent(this);

    while (m_totalCost > m_maxCost && m_indexByDate.size() > 1) {
        QString tmp_key = m_indexByDate.value(0);
        const AbstractFileCacheItem* tmp_item = m_items.value(tmp_key);
        Q_ASSERT(tmp_item);
        m_totalCost -= tmp_item->cost();
        tmp_item->removeFileFromDisk();
        m_items.remove(tmp_key);
        m_indexByDate.removeAt(0);
        delete tmp_item;
    }
}

//------------------------------------------------------------------------------
class MockFileCacheItem : public AbstractFileCacheItem
{
public:
    explicit MockFileCacheItem(const QString& key, int cost, const QDateTime& date_time = QDateTime(), QObject* parent = 0)
        : AbstractFileCacheItem(key, cost, date_time, parent) {}
    MOCK_CONST_METHOD0(removeFileFromDisk, void());
};

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
    cache.addItem(new MockFileCacheItem("foo", 10));
    EXPECT_TRUE(cache.hasItem("foo"));
}

TEST(FileCache, testItemIsParentedWhenAdded) {
    FileCache cache(100);
    AbstractFileCacheItem* item = new MockFileCacheItem("foo", 10);
    cache.addItem(item);
    EXPECT_EQ(&cache, item->parent());
}

TEST(FileCache, testTotalCostIncreasesAfterItemIsAdded) {
    FileCache cache(100);
    EXPECT_EQ(0, cache.totalCost());
    cache.addItem(new MockFileCacheItem("foo", 10));
    EXPECT_EQ(10, cache.totalCost());
}

TEST(FileCache, testSizeIncreasesAfterItemIsAdded) {
    FileCache cache(100);
    EXPECT_EQ(0, cache.size());
    cache.addItem(new MockFileCacheItem("foo", 10));
    EXPECT_EQ(1, cache.size());
}

TEST(FileCache, testRetrieveItemAfterItemIsAdded) {
    const char* KEY = "foo";
    const int COST = 10;
    const QDateTime DATE_TIME(QDate(2012, 8, 1), QTime(0, 0));
    FileCache cache(100);
    cache.addItem(new MockFileCacheItem(KEY, COST, DATE_TIME));
    const AbstractFileCacheItem* actual = cache.item(KEY);
    EXPECT_EQ(KEY, actual->key());
    EXPECT_EQ(COST, actual->cost());
    EXPECT_EQ(DATE_TIME, actual->dateTime());
}

TEST(FileCache, testOlderItemsAreRemovedToMakeRoomForNewerOnes) {
    FileCache cache(100);

    MockFileCacheItem* item1 = new MockFileCacheItem("item1", 10, QDateTime(QDate(2010, 1, 1), QTime(0, 0)));
    EXPECT_CALL(*item1, removeFileFromDisk()).Times(1);

    cache.addItem(item1);
    cache.addItem(new MockFileCacheItem("item2", 40, QDateTime(QDate(2010, 1, 2), QTime(0, 0))));
    cache.addItem(new MockFileCacheItem("item3", 55, QDateTime(QDate(2010, 1, 3), QTime(0, 0)))); // forces "item1" out

    EXPECT_EQ(2, cache.size());
    EXPECT_EQ(95, cache.totalCost());
    EXPECT_EQ(QSet<QString>::fromList(QList<QString>() << "item2" << "item3"),
              QSet<QString>::fromList(cache.keys()));
}

TEST(FileCache, testFileIsNotRemoveOnlyBecauseTheCacheIsDestroyed) {
    FileCache cache(100);
    MockFileCacheItem* item = new MockFileCacheItem("item", 10, QDateTime(QDate(2010, 1, 1), QTime(0, 0)));
    EXPECT_CALL(*item, removeFileFromDisk()).Times(0);
    cache.addItem(item);
}

TEST(FileCache, testAddingAgainAnItemOnlyUpdatesCostAndDate) {
    const char* KEY1 = "item1";
    const char* KEY2 = "item2";

    const int COST1 = 10;
    const int COST2 = 35;
    const int COST3 = 17;

    const int MAX_COST = COST2 + qMax(COST1, COST3) + 5;

    const QDateTime DATE_TIME1(QDate(2010, 1, 1), QTime(0, 0));
    const QDateTime DATE_TIME2(QDate(2010, 1, 2), QTime(0, 0));
    const QDateTime DATE_TIME3(QDate(2010, 1, 3), QTime(0, 0));

    FileCache cache(MAX_COST);
    MockFileCacheItem* item1 = new MockFileCacheItem(KEY1, COST1, DATE_TIME1);
    EXPECT_CALL(*item1, removeFileFromDisk()).Times(0);

    cache.addItem(item1);
    cache.addItem(new MockFileCacheItem(KEY2, COST2, DATE_TIME2));
    cache.addItem(new MockFileCacheItem(KEY1, COST3, DATE_TIME3));

    EXPECT_EQ(COST2 + COST3, cache.totalCost());
    EXPECT_EQ(QSet<QString>::fromList(QList<QString>() << "item1" << "item2"),
              QSet<QString>::fromList(cache.keys()));
    EXPECT_EQ(COST3, cache.item(KEY1)->cost());
    EXPECT_EQ(DATE_TIME3, cache.item(KEY1)->dateTime());
}
