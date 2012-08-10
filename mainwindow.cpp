#include "mainwindow.h"
#include "previewwidget.h"

#include <QtGui>

namespace {
const int STATUSBAR_TIMEOUT = 3000; // in miliseconds
const QString PLANTUML_JAR = "/home/borco/local/bin/plantuml.jar";
const QString JAVA_PATH = "/usr/bin/java";

const QString SETTINGS_SECTION = "MainWindow";
const QString SETTINGS_GEOMETRY = "geometry";
const QString SETTINGS_WINDOW_STATE = "window_state";
const QString SETTINGS_SHOW_STATUSBAR = "show_statusbar";
const QString SETTINGS_AUTOREFRESH_ENABLED = "autorefresh_enabled";
const QString SETTINGS_AUTOREFRESH_TIMEOUT = "autorefresh_timeout";
const int SETTINGS_AUTOREFRESH_TIMEOUT_DEFAULT = 5000; // in miliseconds
const QString SETTINGS_IMAGE_FORMAT = "image_format";
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_process(0)
    , m_currentImageFormat(SvgFormat)
    , m_needsRefresh(false)
{
    m_autoRefreshTimer = new QTimer(this);
    connect(m_autoRefreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));

    m_imageFormatNames[SvgFormat] = "svg";
    m_imageFormatNames[PngFormat] = "png";

    m_imageWidget = new PreviewWidget(this);
    setCentralWidget(m_imageWidget);

    createDockWindows();
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    setWindowTitle(tr("PlantUML Editor"));

    setUnifiedTitleAndToolBarOnMac(true);

    readSettings();

    newDocument();
}

MainWindow::~MainWindow()
{
}

void MainWindow::newDocument()
{
    QString text = "@startuml\n\nclass Foo\n\n@enduml";
    m_textEdit->setPlainText(text);
    refresh();
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About %1").arg(windowTitle()),
                       tr(
                           "The <b>%1</b> allows simple edit and preview of UML "
                           "diagrams generated with <u>%2</u>.<br>"
                           "<br>"
                           "<u>%2</u> and <u>%3</u> must be installed before "
                           "using %1."
                           )
                       .arg(windowTitle()).arg("PlantUML").arg("graphiz")
                       );
}

void MainWindow::refresh()
{
    if (m_process) {
        qDebug() << "still processing previous refresh. skipping...";
        return;
    }

    if (!m_needsRefresh) {
        return;
    }

    QByteArray current_document = m_textEdit->toPlainText().toAscii();
    m_needsRefresh = false;

    if (current_document.isEmpty()) {
        qDebug() << "empty document. skipping...";
        return;
    }

    statusBar()->showMessage(tr("Refreshing..."));

    QString program = JAVA_PATH;
    QStringList arguments;

    switch(m_currentImageFormat) {
    case SvgFormat:
        m_imageWidget->setMode(PreviewWidget::SvgMode);
        break;
    case PngFormat:
        m_imageWidget->setMode(PreviewWidget::PngMode);
        break;
    }

    arguments
            << "-jar" << PLANTUML_JAR
            << QString("-t%1").arg(m_imageFormatNames[m_currentImageFormat])
            << "-pipe";

    m_process = new QProcess(this);
    m_process->start(program, arguments);
    if (!m_process->waitForStarted()) {
        qDebug() << "refresh subprocess failed to start";
        return;
    }

    connect(m_process, SIGNAL(finished(int)), this, SLOT(refreshFinished()));

    m_process->write(current_document);
    m_process->closeWriteChannel();
}

void MainWindow::refreshFinished()
{
    QByteArray output = m_process->readAll();
    m_imageWidget->load(output);
    m_process->deleteLater();
    m_process = 0;
    statusBar()->showMessage(tr("Refreshed"), STATUSBAR_TIMEOUT);
}

void MainWindow::changeImageFormat()
{
    ImageFormat new_format;
    if (m_pngPreviewAction->isChecked()) {
        new_format = PngFormat;
    } else {
        new_format = SvgFormat;
    }

    if (new_format != m_currentImageFormat) {
        m_currentImageFormat = new_format;
        m_needsRefresh = true;
        refresh();
    }
}

void MainWindow::onAutoRefreshActionToggled(bool state)
{
    if (state) {
        refresh();
        m_autoRefreshTimer->start();
    } else {
        m_autoRefreshTimer->stop();
    }
}

void MainWindow::onDocumentChanged()
{
    m_needsRefresh = true;
}

void MainWindow::closeEvent(QCloseEvent *)
{
    writeSettings();
}

void MainWindow::readSettings()
{
    QSettings settings;

    settings.beginGroup(SETTINGS_SECTION);
    restoreGeometry(settings.value(SETTINGS_GEOMETRY).toByteArray());
    restoreState(settings.value(SETTINGS_WINDOW_STATE).toByteArray());

    m_showMainToolbarAction->setChecked(m_mainToolBar->isVisibleTo(this)); // NOTE: works even if the current window is not yet displayed
    connect(m_showMainToolbarAction, SIGNAL(toggled(bool)), m_mainToolBar, SLOT(setVisible(bool)));

    const bool show_statusbar = settings.value(SETTINGS_SHOW_STATUSBAR, true).toBool();
    m_showStatusBarAction->setChecked(show_statusbar);
    statusBar()->setVisible(show_statusbar);
    connect(m_showStatusBarAction, SIGNAL(toggled(bool)), statusBar(), SLOT(setVisible(bool)));

    const bool autorefresh_enabled = settings.value(SETTINGS_AUTOREFRESH_ENABLED, false).toBool();
    m_autoRefreshAction->setChecked(autorefresh_enabled);
    m_autoRefreshTimer->setInterval(settings.value(SETTINGS_AUTOREFRESH_TIMEOUT, SETTINGS_AUTOREFRESH_TIMEOUT_DEFAULT).toInt());
    if (autorefresh_enabled) {
        m_autoRefreshTimer->start();
    }

    m_currentImageFormat = m_imageFormatNames.key(settings.value(SETTINGS_IMAGE_FORMAT, m_imageFormatNames[SvgFormat]).toString());
    if (m_currentImageFormat == SvgFormat) {
        m_svgPreviewAction->setChecked(true);
    } else if (m_currentImageFormat == PngFormat) {
        m_pngPreviewAction->setChecked(true);
    }

    settings.endGroup();
}

void MainWindow::writeSettings()
{
    QSettings settings;

    settings.beginGroup(SETTINGS_SECTION);
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
    settings.setValue(SETTINGS_WINDOW_STATE, saveState());
    settings.setValue(SETTINGS_SHOW_STATUSBAR, m_showStatusBarAction->isChecked());
    settings.setValue(SETTINGS_AUTOREFRESH_ENABLED, m_autoRefreshAction->isChecked());
    settings.setValue(SETTINGS_IMAGE_FORMAT, m_imageFormatNames[m_currentImageFormat]);
    settings.setValue(SETTINGS_AUTOREFRESH_TIMEOUT, m_autoRefreshTimer->interval());
    settings.endGroup();
}

void MainWindow::createActions()
{
    // File menu
    m_newDocumentAction = new QAction(QIcon::fromTheme("document-new"), tr("&New"), this);
    m_newDocumentAction->setShortcut(QKeySequence::New);

    m_openDocumentAction = new QAction(QIcon::fromTheme("document-open"), tr("&Open"), this);
    m_openDocumentAction->setShortcuts(QKeySequence::Open);

    m_saveDocumentAction = new QAction(QIcon::fromTheme("document-save"), tr("&Save"), this);
    m_saveDocumentAction->setShortcuts(QKeySequence::Save);

    m_saveAsDocumentAction = new QAction(QIcon::fromTheme("document-save-as"), tr("Save As..."), this);
    m_saveAsDocumentAction->setShortcuts(QKeySequence::SaveAs);

    m_quitAction = new QAction(QIcon::fromTheme("application-exit"), tr("&Quit"), this);
    m_quitAction->setShortcuts(QKeySequence::Quit);
    m_quitAction->setStatusTip(tr("Quit the application"));
    connect(m_quitAction, SIGNAL(triggered()), this, SLOT(close()));

    // Edit menu
    m_undoAction = new QAction(QIcon::fromTheme("edit-undo"), tr("&Undo"), this);
    m_undoAction->setShortcuts(QKeySequence::Undo);

    m_redoAction = new QAction(QIcon::fromTheme("edit-redo"), tr("&Redo"), this);
    m_redoAction->setShortcuts(QKeySequence::Redo);

    // Tools menu
    m_pngPreviewAction = new QAction(tr("PNG"), this);
    m_pngPreviewAction->setCheckable(true);
    m_pngPreviewAction->setStatusTip(tr("Set PlantUML to produce PNG output"));
    connect(m_pngPreviewAction, SIGNAL(toggled(bool)), this, SLOT(changeImageFormat()));

    m_svgPreviewAction = new QAction(tr("SVG"), this);
    m_svgPreviewAction->setCheckable(true);
    m_svgPreviewAction->setStatusTip(tr("Set PlantUML to produce SVG output"));
    connect(m_svgPreviewAction, SIGNAL(toggled(bool)), this, SLOT(changeImageFormat()));

    QActionGroup* output_action_group = new QActionGroup(this);
    output_action_group->setExclusive(true);
    output_action_group->addAction(m_pngPreviewAction);
    output_action_group->addAction(m_svgPreviewAction);
    m_svgPreviewAction->setChecked(true);

    m_refreshAction = new QAction(QIcon::fromTheme("view-refresh"), tr("Refresh"), this);
    m_refreshAction->setShortcuts(QKeySequence::Refresh);
    m_refreshAction->setStatusTip(tr("Call PlantUML to regenerate the UML image"));
    connect(m_refreshAction, SIGNAL(triggered()), this, SLOT(refresh()));

    m_autoRefreshAction = new QAction(tr("Auto-Refresh"), this);
    m_autoRefreshAction->setCheckable(true);
    connect(m_autoRefreshAction, SIGNAL(toggled(bool)), this, SLOT(onAutoRefreshActionToggled(bool)));

    // Settings menu
    m_showMainToolbarAction = new QAction(tr("Show toolbar"), this);
    m_showMainToolbarAction->setCheckable(true);

    m_showStatusBarAction = new QAction(tr("Show statusbar"), this);
    m_showStatusBarAction->setCheckable(true);

    // Help menu
    m_aboutAction = new QAction(QIcon::fromTheme("help-about"), tr("&About"), this);
    m_aboutAction->setStatusTip(tr("Show the application's About box"));
    connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(about()));

    m_aboutQtAction = new QAction(tr("About &Qt"), this);
    m_aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
    connect(m_aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_newDocumentAction);
    m_fileMenu->addAction(m_openDocumentAction);
    m_fileMenu->addAction(m_saveDocumentAction);
    m_fileMenu->addAction(m_saveAsDocumentAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_quitAction);

    m_editMenu = menuBar()->addMenu(tr("&Edit"));
    m_editMenu->addAction(m_undoAction);
    m_editMenu->addAction(m_redoAction);
    m_editMenu->addSeparator();
    m_editMenu->addAction(m_refreshAction);

    m_settingsMenu = menuBar()->addMenu(tr("&Settings"));
    m_settingsMenu->addAction(m_showMainToolbarAction);
    m_settingsMenu->addAction(m_showStatusBarAction);
    m_settingsMenu->addAction(m_showPreviewAction);
    m_settingsMenu->addSeparator();
    m_settingsMenu->addAction(m_pngPreviewAction);
    m_settingsMenu->addAction(m_svgPreviewAction);
    m_settingsMenu->addSeparator();
    m_settingsMenu->addAction(m_autoRefreshAction);

    menuBar()->addSeparator();

    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_aboutAction);
    m_helpMenu->addAction(m_aboutQtAction);
}

void MainWindow::createToolBars()
{
    m_mainToolBar = addToolBar(tr("MainToolbar"));
    m_mainToolBar->setObjectName("main_toolbar");
    m_mainToolBar->addAction(m_newDocumentAction);
    m_mainToolBar->addAction(m_openDocumentAction);
    m_mainToolBar->addAction(m_saveDocumentAction);
    m_mainToolBar->addAction(m_saveAsDocumentAction);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(m_showPreviewAction);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(m_refreshAction);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"), STATUSBAR_TIMEOUT);
}

void MainWindow::createDockWindows()
{
    QDockWidget *dock = new QDockWidget(tr("Text Editor"), this);
    m_textEdit = new QTextEdit(dock);
    connect(m_textEdit, SIGNAL(textChanged()), this, SLOT(onDocumentChanged()));
    dock->setWidget(m_textEdit);
    dock->setObjectName("text_editor");
    addDockWidget(Qt::RightDockWidgetArea, dock);

    m_showPreviewAction = dock->toggleViewAction();
    m_showPreviewAction->setIconVisibleInMenu(false);
    m_showPreviewAction->setStatusTip("Show or hide the document editor");
    m_showPreviewAction->setIcon(QIcon::fromTheme("accessories-text-editor"));
}
