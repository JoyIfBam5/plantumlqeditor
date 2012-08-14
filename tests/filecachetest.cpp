#include <functional>
#include <QObject>
#include <QString>
#include <QDateTime>
#include <QMap>
#include <QSet>
#include <QDir>
#include "config.h"

#include <QDebug>
#include <gmock/gmock.h>

//------------------------------------------------------------------------------

struct FileCacheError {};

//------------------------------------------------------------------------------

class AbstractFileCacheItem : public QObject
{
public:
    explicit AbstractFileCacheItem(const QString& path, const QString& key, int cost, const QDateTime& date_time, QObject* parent = 0);
    virtual ~AbstractFileCacheItem() {}

    const QString& path() const { return m_path; }
    const QString& key() const { return m_key; }
    int cost() const { return m_cost; }
    const QDateTime& dateTime() const { return m_dateTime; }

    virtual void removeFileFromDisk() const = 0;

private:
    QString m_path;
    QString m_key;
    int m_cost;
    QDateTime m_dateTime;
};

AbstractFileCacheItem::AbstractFileCacheItem(const QString& path, const QString &key, int cost, const QDateTime &date_time, QObject* parent)
    : QObject(parent)
    , m_path(path)
    , m_key(key)
    , m_cost(cost)
    , m_dateTime(date_time)
{
}

//------------------------------------------------------------------------------

class FileCacheItem : public AbstractFileCacheItem
{
public:
    explicit FileCacheItem(const QString& path, const QString& key, int cost, const QDateTime& date_time, QObject* parent = 0);

    virtual void removeFileFromDisk() const;

private:
    mutable bool m_removed;
};

FileCacheItem::FileCacheItem(const QString& path, const QString &key, int cost, const QDateTime &date_time, QObject *parent)
    : AbstractFileCacheItem(path, key, cost, date_time, parent)
    , m_removed(false)
{
}

void FileCacheItem::removeFileFromDisk() const
{
    if (m_removed)
        throw FileCacheError(); // try to remove twice

    QDir().remove(path());
    m_removed = true;
}

//------------------------------------------------------------------------------

class FileCache : public QObject
{
public:
    typedef std::function<AbstractFileCacheItem* (const QString&, // path
                                                  const QString&, // key
                                                  int, // cost
                                                  const QDateTime&, // access time
                                                  QObject* // parent
                                                  )>  ItemGenerator;

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

    void clear();
    void clearFromDisk();

    bool setPath(const QString& path);
    bool setPath(const QString& path, ItemGenerator item_generator);
    const QString& path() const { return m_path; }

private:
    bool updateFromDisk(const QString &path, ItemGenerator item_generator);

    QString m_path;
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
        if (old_item == item) {
            // adding the same item twice is an error
            throw FileCacheError();
        }
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

void FileCache::clear()
{
    foreach (AbstractFileCacheItem* item, m_items) {
        delete item;
    }
    m_items.clear();
    m_indexByDate.clear();
    m_totalCost = 0;
}

void FileCache::clearFromDisk()
{
    foreach (AbstractFileCacheItem* item, m_items) {
        item->removeFileFromDisk();
        delete item;
    }
    m_items.clear();
    m_indexByDate.clear();
    m_totalCost = 0;
}

bool FileCache::setPath(const QString &path, ItemGenerator item_generator)
{
    if (m_path != path) {
        clear();
        bool success = updateFromDisk(path, item_generator);
        if (success) {
            m_path = path;
        }
        return success;
    }
    return true;
}

bool FileCache::updateFromDisk(const QString &path, ItemGenerator item_generator)
{
    QDir dir(path);
    if (!dir.mkpath(path)) {
        return false;
    }

    foreach (QFileInfo info, dir.entryInfoList(QDir::Files)) {
        QString file_path = info.canonicalFilePath();
        QString key = info.fileName();
        int cost = info.size();
        QDateTime date_time = info.lastRead();
        addItem(item_generator(file_path, key, cost, date_time, this));
    }
}

//------------------------------------------------------------------------------
class MockNoPathFileCacheItem : public AbstractFileCacheItem
{
public:
    explicit MockNoPathFileCacheItem(const QString& key, int cost, const QDateTime& date_time = QDateTime(), QObject* parent = 0)
        : AbstractFileCacheItem("", key, cost, date_time, parent) {}
    MOCK_CONST_METHOD0(removeFileFromDisk, void());
};

class MockFileCacheItem : public AbstractFileCacheItem
{
public:
    explicit MockFileCacheItem(const QString& path, const QString& key, int cost, const QDateTime& date_time = QDateTime(), QObject* parent = 0)
        : AbstractFileCacheItem(path, key, cost, date_time, parent) {}

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
    cache.addItem(new MockNoPathFileCacheItem("foo", 10));
    EXPECT_TRUE(cache.hasItem("foo"));
}

TEST(FileCache, testItemIsParentedWhenAdded) {
    FileCache cache(100);
    AbstractFileCacheItem* item = new MockNoPathFileCacheItem("foo", 10);
    cache.addItem(item);
    EXPECT_EQ(&cache, item->parent());
}

TEST(FileCache, testTotalCostIncreasesAfterItemIsAdded) {
    FileCache cache(100);
    EXPECT_EQ(0, cache.totalCost());
    cache.addItem(new MockNoPathFileCacheItem("foo", 10));
    EXPECT_EQ(10, cache.totalCost());
}

TEST(FileCache, testSizeIncreasesAfterItemIsAdded) {
    FileCache cache(100);
    EXPECT_EQ(0, cache.size());
    cache.addItem(new MockNoPathFileCacheItem("foo", 10));
    EXPECT_EQ(1, cache.size());
}

TEST(FileCache, testRetrieveItemAfterItemIsAdded) {
    const char* KEY = "foo";
    const int COST = 10;
    const QDateTime DATE_TIME(QDate(2012, 8, 1), QTime(0, 0));
    FileCache cache(100);
    cache.addItem(new MockNoPathFileCacheItem(KEY, COST, DATE_TIME));
    const AbstractFileCacheItem* actual = cache.item(KEY);
    EXPECT_EQ(KEY, actual->key());
    EXPECT_EQ(COST, actual->cost());
    EXPECT_EQ(DATE_TIME, actual->dateTime());
}

TEST(FileCache, testOlderItemsAreRemovedToMakeRoomForNewerOnes) {
    FileCache cache(100);

    MockNoPathFileCacheItem* item1 = new MockNoPathFileCacheItem("item1", 10, QDateTime(QDate(2010, 1, 1), QTime(0, 0)));
    EXPECT_CALL(*item1, removeFileFromDisk()).Times(1);

    cache.addItem(item1);
    cache.addItem(new MockNoPathFileCacheItem("item2", 40, QDateTime(QDate(2010, 1, 2), QTime(0, 0))));
    cache.addItem(new MockNoPathFileCacheItem("item3", 55, QDateTime(QDate(2010, 1, 3), QTime(0, 0)))); // forces "item1" out

    EXPECT_EQ(2, cache.size());
    EXPECT_EQ(95, cache.totalCost());
    EXPECT_EQ(QSet<QString>::fromList(QList<QString>() << "item2" << "item3"),
              QSet<QString>::fromList(cache.keys()));
}

TEST(FileCache, testFileIsNotRemoveOnlyBecauseTheCacheIsDestroyed) {
    FileCache cache(100);
    MockNoPathFileCacheItem* item = new MockNoPathFileCacheItem("item", 10, QDateTime(QDate(2010, 1, 1), QTime(0, 0)));
    EXPECT_CALL(*item, removeFileFromDisk()).Times(0);
    cache.addItem(item);
}

TEST(FileCache, testClearFromDiskRemovesFilesFromDisk) {
    FileCache cache(100);
    MockNoPathFileCacheItem* item = new MockNoPathFileCacheItem("foo", 10);
    EXPECT_CALL(*item, removeFileFromDisk()).Times(1);
    cache.addItem(item);
    cache.clearFromDisk();
    EXPECT_EQ(0, cache.size());
    EXPECT_EQ(0, cache.totalCost());
}

TEST(FileCache, testClearDoenstRemovesFilesFromDisk) {
    FileCache cache(100);
    MockNoPathFileCacheItem* item = new MockNoPathFileCacheItem("foo", 10);
    EXPECT_CALL(*item, removeFileFromDisk()).Times(0);
    cache.addItem(item);
    cache.clear();
    EXPECT_EQ(0, cache.size());
    EXPECT_EQ(0, cache.totalCost());
}

TEST(FileCache, testAddingAgainAnItemOnlyUpdatesCostAndDate) {
    const char* KEY1 = "item1";
    const char* KEY2 = "item2";

    const int COST1 = 10;
    const int COST2 = 35;
    const int COST3 = 15;

    const int MAX_COST = COST2 + COST3 + 5;

    const QDateTime DATE_TIME1(QDate(2010, 1, 1), QTime(0, 0));
    const QDateTime DATE_TIME2(QDate(2010, 1, 2), QTime(0, 0));
    const QDateTime DATE_TIME3(QDate(2010, 1, 3), QTime(0, 0));

    FileCache cache(MAX_COST);
    MockNoPathFileCacheItem* item1 = new MockNoPathFileCacheItem(KEY1, COST1, DATE_TIME1);
    EXPECT_CALL(*item1, removeFileFromDisk()).Times(0);

    cache.addItem(item1);
    cache.addItem(new MockNoPathFileCacheItem(KEY2, COST2, DATE_TIME2));
    cache.addItem(new MockNoPathFileCacheItem(KEY1, COST3, DATE_TIME3));

    EXPECT_EQ(COST2 + COST3, cache.totalCost());
    EXPECT_EQ(QSet<QString>::fromList(QList<QString>() << KEY1 << KEY2),
              QSet<QString>::fromList(cache.keys()));
    EXPECT_EQ(COST3, cache.item(KEY1)->cost());
    EXPECT_EQ(DATE_TIME3, cache.item(KEY1)->dateTime());
}

TEST(FileCache, testCorrectFileIsDeletedFromDiskAfterUpdating) {
    const char* KEY1 = "item1";
    const char* KEY2 = "item2";
    const char* KEY4 = "item4";

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
    MockNoPathFileCacheItem* item1 = new MockNoPathFileCacheItem(KEY1, COST1, DATE_TIME1);
    EXPECT_CALL(*item1, removeFileFromDisk()).Times(0);

    MockNoPathFileCacheItem* item2 = new MockNoPathFileCacheItem(KEY2, COST2, DATE_TIME2);
    EXPECT_CALL(*item2, removeFileFromDisk()).Times(1);

    MockNoPathFileCacheItem* item3 = new MockNoPathFileCacheItem(KEY1, COST3, DATE_TIME3);
    EXPECT_CALL(*item3, removeFileFromDisk()).Times(0);

    MockNoPathFileCacheItem* item4 = new MockNoPathFileCacheItem(KEY4, COST4, DATE_TIME4);
    EXPECT_CALL(*item4, removeFileFromDisk()).Times(0);

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
    AbstractFileCacheItem* item = new MockNoPathFileCacheItem("foo", 10);
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

    EXPECT_EQ(QFileInfo(QDir(TEST_DIR1), "item1.svg").canonicalFilePath(),
              cache.item("item1.svg")->path());
}
