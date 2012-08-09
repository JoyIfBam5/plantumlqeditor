#include "mainwindow.h"
#include "previewwidget.h"

#include <QtGui>

namespace {
const int STATUS_BAR_TIMEOUT = 5000; // in miliseconds
const QString PLANTUML_JAR = "/home/borco/local/bin/plantuml.jar";
const QString JAVA_PATH = "/usr/bin/java";
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_process(0)
{
    m_textEdit = new QTextEdit;
    setCentralWidget(m_textEdit);

    createDockWindows();
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    setWindowTitle(tr("PlantUML Editor"));

    newDocument();
    setUnifiedTitleAndToolBarOnMac(true);
}

MainWindow::~MainWindow()
{
}

void MainWindow::newDocument()
{
    QString text = "@startuml\n\nclass Foo\n\n@enduml";
    m_textEdit->setPlainText(text);
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

    QString program = JAVA_PATH;
    QStringList arguments;
    QString output;

    if (m_svgPreviewAction->isChecked()) {
        output = "-tsvg";
        m_preview->setMode(PreviewWidget::SvgMode);
    } else {
        output = "-tpng";
        m_preview->setMode(PreviewWidget::PngMode);
    }

    arguments << "-jar" << PLANTUML_JAR << output << "-pipe";

    m_process = new QProcess(this);
    m_process->start(program, arguments);
    if (!m_process->waitForStarted()) {
        qDebug() << "refresh subprocess failed to start";
        return;
    }

    connect(m_process, SIGNAL(finished(int)), this, SLOT(refreshFinished()));

    m_process->write(m_textEdit->toPlainText().toAscii());
    m_process->closeWriteChannel();
}

void MainWindow::refreshFinished()
{
    QByteArray output = m_process->readAll();
    m_preview->load(output);
    m_process->deleteLater();
    m_process = 0;
}

void MainWindow::createActions()
{
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

    m_pngPreviewAction = new QAction(tr("PNG"), this);
    m_pngPreviewAction->setCheckable(true);
    m_pngPreviewAction->setStatusTip(tr("Set PlantUML to produce PNG output"));

    m_svgPreviewAction = new QAction(tr("SVG"), this);
    m_svgPreviewAction->setCheckable(true);
    m_svgPreviewAction->setStatusTip(tr("Set PlantUML to produce SVG output"));

    QActionGroup* output_action_group = new QActionGroup(this);
    output_action_group->setExclusive(true);
    output_action_group->addAction(m_pngPreviewAction);
    output_action_group->addAction(m_svgPreviewAction);
    m_svgPreviewAction->setChecked(true);

    m_previewRefreshAction = new QAction(QIcon::fromTheme("view-refresh"), tr("Refresh"), this);
    m_previewRefreshAction->setShortcuts(QKeySequence::Refresh);
    m_previewRefreshAction->setStatusTip(tr("Call PlantUML to regenerate the UML preview"));
    connect(m_previewRefreshAction, SIGNAL(triggered()), this, SLOT(refresh()));

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

    m_viewMenu = menuBar()->addMenu(tr("&View"));
    m_viewMenu->addAction(m_previewViewAction);
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(m_previewRefreshAction);

    m_viewMenu->addSeparator();
    m_viewMenu->addAction(m_pngPreviewAction);
    m_viewMenu->addAction(m_svgPreviewAction);

    menuBar()->addSeparator();

    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_aboutAction);
    m_helpMenu->addAction(m_aboutQtAction);
}

void MainWindow::createToolBars()
{
    m_mainToolBar = addToolBar(tr("File"));
    m_mainToolBar->addAction(m_newDocumentAction);
    m_mainToolBar->addAction(m_openDocumentAction);
    m_mainToolBar->addAction(m_saveDocumentAction);
    m_mainToolBar->addAction(m_saveAsDocumentAction);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(m_previewViewAction);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(m_previewRefreshAction);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"), STATUS_BAR_TIMEOUT);
}

void MainWindow::createDockWindows()
{
    QDockWidget *dock = new QDockWidget(tr("UML diagram"), this);
    m_preview = new PreviewWidget(dock);
    dock->setWidget(m_preview);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    m_previewViewAction = dock->toggleViewAction();
    m_previewViewAction->setIconVisibleInMenu(false);
    m_previewViewAction->setStatusTip("Show or hide the UML diagram");
    m_previewViewAction->setIcon(QIcon::fromTheme("image-x-generic"));
}
