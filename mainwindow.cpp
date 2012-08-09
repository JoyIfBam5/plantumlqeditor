#include "mainwindow.h"
#include <QtGui>
#include <QSvgWidget>

namespace {
int STATUS_BAR_TIMEOUT = 5000; // in miliseconds
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
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

    m_refreshAction = new QAction(QIcon::fromTheme("view-refresh"), tr("Refresh"), this);
    m_refreshAction->setShortcuts(QKeySequence::Refresh);
    m_refreshAction->setStatusTip(tr("Call PlantUML to regenerate the UML preview"));

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
    m_viewMenu->addAction(m_umlPreviewViewAction);
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(m_refreshAction);

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
    m_mainToolBar->addAction(m_umlPreviewViewAction);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(m_refreshAction);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"), STATUS_BAR_TIMEOUT);
}

void MainWindow::createDockWindows()
{
    QDockWidget *dock = new QDockWidget(tr("UML diagram"), this);
    m_umlPreview = new QSvgWidget(dock);
    dock->setWidget(m_umlPreview);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    m_umlPreviewViewAction = dock->toggleViewAction();
    m_umlPreviewViewAction->setIconVisibleInMenu(false);
    m_umlPreviewViewAction->setStatusTip("Show or hide the UML diagram");
    m_umlPreviewViewAction->setIcon(QIcon::fromTheme("image-x-generic"));
}
