#ifndef RECENTDOCUMENTS_H
#define RECENTDOCUMENTS_H

#include <QObject>
#include <QList>
#include <QStringList>

class QAction;
class QSettings;

class RecentDocuments : public QObject
{
    Q_OBJECT
public:
    explicit RecentDocuments(int max_documents, QObject *parent = 0);
    QList<QAction*> actions() const { return m_actions; }

    void clear();
    void accessing(const QString& name);
    void readFromSettings(QSettings& settings, const QString& section);
    void writeToSettings(QSettings& settings, const QString& section);

signals:
    void recentDocument(const QString& name);

private slots:
    void onCleatActionTriggered();
    void onRecentDocumentsActionTriggered(int index);

private:
    const int m_maxDocuments;
    QList<QAction*> m_actions;
    QList<QAction*> m_recentDocumentsActions;
    QAction* m_separatorAction;
    QAction* m_clearAction;
    QStringList m_documents;
};

#endif // RECENTDOCUMENTS_H
