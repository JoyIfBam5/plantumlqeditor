#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

namespace Ui {
class PreferencesDialog;
}

class FileCache;

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(FileCache* file_cache, QWidget *parent = 0);
    ~PreferencesDialog();

    void readSettings();
    void writeSettings();

private slots:
    void onRejected();
    void on_customJavaPathButton_clicked();
    void on_customPlantUmlButton_clicked();
    void on_assistantXmlButton_clicked();

private:
    Ui::PreferencesDialog *m_ui;
    FileCache* m_fileCache;
};

#endif // PREFERENCESDIALOG_H
