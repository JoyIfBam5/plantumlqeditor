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
class QSignalMapper;
class QToolBox;
class QListWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void openDocument(const QString& path);

public slots:
    void newDocument();

private slots:
    void about();
    void refresh();
    void refreshFinished();
    void changeImageFormat();
    void undo();
    void redo();

    void onAutoRefreshActionToggled(bool state);
    void onEditorChanged();
    void onRefreshActionTriggered();
    void onPreferencesActionTriggered();
    void onOpenDocumentActionTriggered();
    void onSaveActionTriggered();
    void onSaveAsActionTriggered();
    void onExportImageActionTriggered();
    void onExportAsImageActionTriggered();
    void onClearRecentDocumentsActionTriggered();
    void onRecentDocumentsActionTriggered(const QString& path);

private:
    enum ImageFormat { SvgFormat, PngFormat };

    void closeEvent(QCloseEvent *event);

    bool maybeSave();
    void readSettings();
    void writeSettings();
    bool saveDocument(const QString& name);
    void exportImage(const QString& name);

    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createDockWindows();
    void enableUndoRedoActions();

    void checkPaths();
    void updateRecentDocumentsList(const QString& path);
    void updateRecentDocumentsMenu();
    void reloadAssistantXml(const QString& path);

    QLabel *m_currentImageFormatLabel;
    QLabel *m_autorefreshLabel;
    QLabel *m_exportPathLabel;

    QString m_documentPath;
    QString m_exportPath;
    QByteArray m_cachedImage;

    QString m_assistantXmlPath;
    QList<QListWidget*> m_assistantWidgets;

    QStringList m_recentDocumentsList;
    QSignalMapper *m_recentDocumentsSignalMapper;

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

    QToolBox *m_assistantToolBox;

    QToolBar *m_mainToolBar;

    QMenu *m_fileMenu;
    QAction *m_newDocumentAction;
    QAction *m_openDocumentAction;
    QAction *m_saveDocumentAction;
    QAction *m_saveAsDocumentAction;
    QAction *m_exportImageAction;
    QAction *m_exportAsImageAction;
    QAction *m_quitAction;

    QMenu *m_recentDocumentsMenu;
    QAction *m_clearRecentDocumentsAction;

    QMenu *m_editMenu;
    QAction *m_undoAction;
    QAction *m_redoAction;
    QAction *m_refreshAction;

    QMenu *m_settingsMenu;
    QAction *m_showAssistantDockAction;
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
