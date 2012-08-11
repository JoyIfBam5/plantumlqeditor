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
    explicit AssistantItem(const QString& name, const QString& data, Assistant *parent = 0);

    const QString& name() const { return m_name; }
    const QString& data() const { return m_data; }
    const QString& icon() const { return m_icon; }

private:
    QString m_name;
    QString m_data;
    QString m_icon;
};

class Assistant : public QObject
{
    Q_OBJECT
public:
    explicit Assistant(const QString& name, AssistantXmlReader *parent = 0);

    const QString& name() const { return m_name; }
    int size() const { return m_items.size(); }
    const AssistantItem* item(int index) const { return m_items.at(index); }

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

private:
    void readRootElement();
    void skipUnknownElement();
    void readAssistantElement();

    QString m_iconDir;
    QList<Assistant*> m_items;
    QXmlStreamReader m_reader;
};

#endif // ASSISTANTXMLREADER_H
