#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QAction;
class QMenu;
class QTextEdit;
class QSvgWidget;
class QProcess;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void about();
    void refresh();
    void refreshFinished();

private:
    void newDocument();

    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createDockWindows();

    QProcess *m_process;

    QTextEdit *m_textEdit;
    QSvgWidget *m_preview;

    QMenu *m_fileMenu;
    QMenu *m_viewMenu;
    QMenu *m_helpMenu;

    QToolBar *m_mainToolBar;

    QAction *m_newDocumentAction;
    QAction *m_openDocumentAction;
    QAction *m_saveDocumentAction;
    QAction *m_saveAsDocumentAction;

    QAction *m_quitAction;

    QAction *m_previewViewAction;
    QAction *m_previewRefreshAction;

    QAction *m_aboutAction;
    QAction *m_aboutQtAction;
};

#endif // MAINWINDOW_H
