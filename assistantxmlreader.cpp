#include "assistantxmlreader.h"
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDebug>

namespace {
const QString ROOT_TAG = "assistants";
const QString ASSISTANT_TAG = "assistant";

QString removeWhiteSpace(const QString& data)
{
    QStringList lines = data.split('\n');

    for (int i = 0; i < lines.size(); ++i) {
        lines[i] = lines[i].trimmed();
    }

    int index_begin = 0;
    int index_end = lines.size() - 1;

    while (lines[index_begin].isEmpty() && index_begin < lines.size()) {
        ++index_begin;
    }

    while (lines[index_end].isEmpty() && index_end >= index_begin) {
        --index_end;
    }

    if (index_begin != 0 || index_end != lines.size()) {
        lines = lines.mid(index_begin, index_end - index_begin);
    }
    return lines.join(QChar('\n'));
}

}

AssistantXmlReader::AssistantXmlReader(QObject *parent)
    : QObject(parent)
{
}

bool AssistantXmlReader::readFile(const QString &path)
{
    foreach(Assistant* assistant, m_items) {
        delete assistant;
    }
    m_items.clear();

    QDir dir = QFileInfo(path).absoluteDir();
    if (dir.cd("icons")) {
        m_iconDir = dir.absolutePath();
    } else {
        m_iconDir.clear();
    }
    qDebug() << "icon path:" << m_iconDir;

    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "can't read assistant file:" << path;
        qDebug() << "i/o error:" << file.errorString();
        return false;
    }
    m_reader.setDevice(&file);
    m_reader.readNext();
    while (!m_reader.atEnd()) {
        if (m_reader.isStartElement()) {
            if (m_reader.name() == ROOT_TAG) {
                readRootElement();
            } else {
                m_reader.raiseError(QObject::tr("Not an assistants XML file"));
            }
        } else {
            m_reader.readNext();
        }
    }

    file.close();
    if (m_reader.hasError()) {
        qDebug() << "failed to parse assistant file:" << path;
        qDebug() << "xml parsing error:" << m_reader.errorString();
        return false;
    } else if (file.error() != QFile::NoError) {
        qDebug() << "can't read assistant file:" << path;
        qDebug() << "i/o error:" << file.errorString();
        return false;
    }

    return true;
}

void AssistantXmlReader::readRootElement()
{
    m_reader.readNext();
    while (!m_reader.atEnd()) {
        if (m_reader.isEndElement()) {
            m_reader.readNext();
            break;
        }

        if (m_reader.isStartElement()) {
            if (m_reader.name() == ASSISTANT_TAG) {
                readAssistantElement();
            } else {
                skipUnknownElement();
            }
        } else {
            m_reader.readNext();
        }
    }
}

void AssistantXmlReader::skipUnknownElement()
{
    m_reader.readNext();
    while (!m_reader.atEnd()) {
        if (m_reader.isEndElement()) {
            m_reader.readNext();
            break;
        }
        if (m_reader.isStartElement()) {
            skipUnknownElement();
        } else {
            m_reader.readNext();
        }
    }
}

void AssistantXmlReader::readAssistantElement()
{
    QString name = m_reader.attributes().value("name").toString();
    Assistant* assistant = new Assistant(name, this);
    m_items.append(assistant);

    m_reader.readNext();
    while (!m_reader.atEnd()) {
        if (m_reader.isEndElement()) {
            m_reader.readNext();
            break;
        }

        if (m_reader.isStartElement()) {
            if (m_reader.name() == "item") {
                readAssistantItemElement(assistant);
            } else {
                skipUnknownElement();
            }
        } else {
            m_reader.readNext();
        }
    }
}

void AssistantXmlReader::readAssistantItemElement(Assistant *assistant)
{
    QString name = m_reader.attributes().value("name").toString();

    // we assume the data is stored in the next child CDATA
    m_reader.readNext();
    QString data = removeWhiteSpace(m_reader.text().toString());
    m_reader.readNext();

    AssistantItem* item = new AssistantItem(name, data, m_iconDir + "/" + assistant->name(), assistant);
    assistant->append(item);

    skipUnknownElement();
}

AssistantItem::AssistantItem(const QString &name, const QString &data, const QString& icon_prefix, QObject *parent)
    : QObject(parent)
    , m_name(name)
    , m_data(data)
{
    m_icon = icon_prefix + " - " + name + ".svg";
}


Assistant::Assistant(const QString &name, QObject *parent)
    : QObject(parent)
    , m_name(name)
{
}

Assistant::~Assistant()
{
    foreach(AssistantItem* item, m_items) {
        delete item;
    }
    m_items.clear();
}

void Assistant::append(AssistantItem *item)
{
    m_items.append(item);
}
