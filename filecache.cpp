#include "filecache.h"
#include <QDir>
#include <QDebug>

//------------------------------------------------------------------------------

namespace {
QString cachePathFromPathAndKey(const QString& path, const QString& key) {
    return QFileInfo(QDir(path), key).absoluteFilePath();
}
} // namespace {}

//------------------------------------------------------------------------------

AbstractFileCacheItem::AbstractFileCacheItem(const QString& path, const QString &key, int cost, const QDateTime &date_time, QObject* parent)
    : QObject(parent)
    , m_path(cachePathFromPathAndKey(path, key))
    , m_key(key)
    , m_cost(cost)
    , m_dateTime(date_time)
{
    qDebug() << qPrintable(QString("new file cache item ->   key: %1   cost: %2   path: %3").arg(key, -10).arg(cost, -5).arg(m_path));
}

AbstractFileCacheItem::~AbstractFileCacheItem()
{
}

//------------------------------------------------------------------------------

FileCacheItem::FileCacheItem(const QString& path, const QString &key, int cost, const QDateTime &date_time, QObject *parent)
    : AbstractFileCacheItem(path, key, cost, date_time, parent)
    , m_removed(false)
{
}

FileCacheItem::~FileCacheItem()
{
}

void FileCacheItem::removeFileFromDisk(const QString& some_path) const
{
    if (m_removed || // try to remove twice
            some_path != path() // trying to remove something else
            )
        throw FileCacheError();

    QDir().remove(path());
    qDebug() << qPrintable(QString("removed cached file ->   key: %1   cost: %2   path: %3").arg(key(), -10).arg(cost(), -5).arg(path()));
    m_removed = true;
}

//------------------------------------------------------------------------------

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
        if (old_item == item || // adding the same item twice is an error
                old_item->path() != item->path() // adding with same key but different path is not supported
                ) {
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

void FileCache::addItem(const QByteArray &data, const QString &key, FileCache::ItemGenerator item_generator)
{
    QFile file(cachePathFromPathAndKey(m_path, key));
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }
    file.write(data);
    file.close();

    QFileInfo info(QDir(m_path), key);
    int cost = info.size();
    QDateTime date_time = info.lastModified();
    addItem(item_generator(m_path, key, cost, date_time, this));
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
    qDebug() << "clear from disk:" << m_path;

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
        QString key = info.fileName();
        int cost = info.size();
        QDateTime date_time = info.lastRead();
        addItem(item_generator(path, key, cost, date_time, this));
    }
    return true;
}

//------------------------------------------------------------------------------
