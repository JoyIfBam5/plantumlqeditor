#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>

class QAction;
class QMenu;
class QTextEdit;
class QProcess;
class PreviewWidget;
class QTimer;
class QLabel;

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
    void changeImageFormat();
    void onAutoRefreshActionToggled(bool state);
    void onEditorChanged();
    void onRefreshActionTriggered();
    void onPreferencesActionTriggered();
    void onSaveActionTriggered();
    void onSaveAsActionTriggered();
    void onExportImageActionTriggered();
    void onExportAsImageActionTriggered();
    void newDocument();
    void openDocument();

private:
    enum ImageFormat { SvgFormat, PngFormat };
    void closeEvent(QCloseEvent *);

    void readSettings();
    void writeSettings();
    void saveDocument(const QString& name);
    void exportImage(const QString& name);

    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createDockWindows();

    void checkPaths();

    QLabel *m_currentImageFormatLabel;
    QLabel *m_autorefreshLabel;

    QString m_documentPath;
    QString m_exportPath;
    QByteArray m_cachedImage;

    QString m_javaPath;
    QString m_platUmlPath;
    bool m_hasValidPaths;

    QProcess *m_process;
    QMap<ImageFormat, QString> m_imageFormatNames;
    ImageFormat m_currentImageFormat;
    QTimer *m_autoRefreshTimer;
    bool m_needsRefresh;

    QTextEdit *m_editor;
    PreviewWidget *m_imageWidget;

    QToolBar *m_mainToolBar;

    QMenu *m_fileMenu;
    QAction *m_newDocumentAction;
    QAction *m_openDocumentAction;
    QAction *m_saveDocumentAction;
    QAction *m_saveAsDocumentAction;
    QAction *m_exportImageAction;
    QAction *m_exportAsImageAction;
    QAction *m_quitAction;

    QMenu *m_editMenu;
    QAction *m_undoAction;
    QAction *m_redoAction;
    QAction *m_refreshAction;

    QMenu *m_settingsMenu;
    QAction *m_showEditorDockAction;
    QAction *m_showMainToolbarAction;
    QAction *m_showStatusBarAction;
    QAction *m_pngPreviewAction;
    QAction *m_svgPreviewAction;
    QAction *m_autoRefreshAction;
    QAction *m_preferencesAction;

    QMenu *m_helpMenu;
    QAction *m_aboutAction;
    QAction *m_aboutQtAction;
};

#endif // MAINWINDOW_H
