#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = 0);
    ~PreferencesDialog();

    void setJavaPath(const QString& path);
    QString javaPath() const;

    void setPlantUmlPath(const QString& path);
    QString plantUmlPath() const;

    void setAutoRefreshTimeout(int timeout);
    int autoRefreshTimeout() const;

private slots:
    void on_javaPathButton_clicked();

    void on_plantUmlButton_clicked();

private:
    Ui::PreferencesDialog *m_ui;
};

#endif // PREFERENCESDIALOG_H
