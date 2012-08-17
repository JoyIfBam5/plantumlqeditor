#ifndef ASSISTANTXMLREADER_H
#define ASSISTANTXMLREADER_H

#include <QObject>
#include <QXmlStreamReader>

class Assistant;
class AssistantXmlReader;

class AssistantItem : public QObject
{
    Q_OBJECT
public:
    explicit AssistantItem(const QString& name, const QString& data, const QString& notes, const QString& icon_prefix, QObject *parent = 0);

    const QString& name() const { return m_name; }
    const QString& data() const { return m_data; }
    const QString& icon() const { return m_icon; }
    const QString& notes() const { return m_notes; }

private:
    const QString m_name;
    const QString m_data;
    const QString m_notes;
    const QString m_icon;
};

class Assistant : public QObject
{
    Q_OBJECT
public:
    explicit Assistant(const QString& name, QObject *parent = 0);
    ~Assistant();

    const QString& name() const { return m_name; }
    int size() const { return m_items.size(); }

    const AssistantItem* item(int index) const { return m_items.at(index); }
    void append(AssistantItem* item);

private:
    QString m_name;
    QList<AssistantItem*> m_items;
};

class AssistantXmlReader: public QObject
{
    Q_OBJECT
public:
    explicit AssistantXmlReader(QObject *parent = 0);

    bool readFile(const QString& path);

    int size() const { return m_items.size(); }
    const Assistant* assistant(int index) { return m_items.at(index); }
    const QString& iconDir() const { return m_iconDir; }

    static QString removeWhiteSpace(const QString& data);
    static int trimLeft(QString& data); //< returns the number of chars trimmed
    static void trimRight(QString& data);

private:
    void readRootElement();
    void skipUnknownElement();
    void readAssistantElement();
    void readAssistantItemElement(Assistant *assistant);
    QString readAssistantItemNotes();

    QString m_iconDir;
    QList<Assistant*> m_items;
    QXmlStreamReader m_reader;
};

#endif // ASSISTANTXMLREADER_H
