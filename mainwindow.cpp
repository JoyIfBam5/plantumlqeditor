#include "mainwindow.h"
#include "previewwidget.h"
#include "preferencesdialog.h"
#include "assistantxmlreader.h"

#include <QtGui>
#include <QtSvg>

namespace {
const int MAX_RECENT_DOCUMENT_SIZE = 10;
const int STATUSBAR_TIMEOUT = 3000; // in miliseconds
const QString TITLE_FORMAT_STRING = "%1[*] - %2";
const QString EXPORT_TO_MENU_FORMAT_STRING = QObject::tr("Export to %1");
const QString EXPORT_TO_LABEL_FORMAT_STRING = QObject::tr("Export to: %1");
const QString AUTOREFRESH_STATUS_LABEL = QObject::tr("Auto-refresh");

const QString SETTINGS_MAIN_SECTION = "MainWindow";
const QString SETTINGS_GEOMETRY = "geometry";
const QString SETTINGS_WINDOW_STATE = "window_state";
const QString SETTINGS_SHOW_STATUSBAR = "show_statusbar";
const QString SETTINGS_AUTOREFRESH_ENABLED = "autorefresh_enabled";
const QString SETTINGS_AUTOREFRESH_TIMEOUT = "autorefresh_timeout";
const int SETTINGS_AUTOREFRESH_TIMEOUT_DEFAULT = 5000; // in miliseconds
const QString SETTINGS_IMAGE_FORMAT = "image_format";
const QString SETTINGS_JAVA_PATH = "java";
const QString SETTINGS_JAVA_PATH_DEFAULT = "/usr/bin/java";
const QString SETTINGS_PLATUML_PATH = "plantuml";
const QString SETTINGS_PLATUML_PATH_DEFAULT = "/usr/bin/plantuml";
const QString SETTINGS_ASSISTANT_XML_PATH = "assistant_xml";

const QString SETTINGS_RECENT_DOCUMENTS_SECTION = "recent_documents";
const QString SETTINGS_RECENT_DOCUMENTS_DOCUMENT = "document";

const QSize ASSISTANT_ICON_SIZE(128, 128);

QIcon iconFromSvg(QSize size, const QString& path)
{
    QPixmap pixmap(size);
    QPainter painter(&pixmap);
    const QRect bounding_rect(QPoint(0, 0), size);

    if (!path.isEmpty()) {
        painter.setRenderHint(QPainter::Antialiasing, true);

        painter.setBrush(QBrush(Qt::white, Qt::SolidPattern));
        painter.setPen(Qt::NoPen);
        painter.drawRect(bounding_rect);

        QSvgRenderer svg(path);
        QSize target_size = svg.defaultSize();
        target_size.scale(size, Qt::KeepAspectRatio);
        QRect target_rect = QRect(QPoint(0, 0), target_size);
        target_rect.translate(bounding_rect.center() - target_rect.center());
        svg.render(&painter, target_rect);
    } else {
        painter.setBrush(QBrush(Qt::white, Qt::SolidPattern));
        painter.setPen(Qt::NoPen);
        painter.drawRect(bounding_rect);

        const int margin = 5;
        QRect target_rect = bounding_rect.adjusted(margin, margin, -margin, -margin);
        painter.setPen(Qt::SolidLine);
        painter.drawRect(target_rect);
        painter.drawLine(target_rect.topLeft(), target_rect.bottomRight() + QPoint(1, 1));
        painter.drawLine(target_rect.bottomLeft() + QPoint(0, 1), target_rect.topRight() + QPoint(1, 0));
    }

    QIcon icon;
    icon.addPixmap(pixmap);
    return icon;
}

QListWidget* newAssistantListWidget(const QSize& icon_size, QWidget* parent)
{
    QListWidget* view = new QListWidget(parent);
    view->setUniformItemSizes(true);
    view->setMovement(QListView::Static);
    view->setResizeMode(QListView::Adjust);
    view->setIconSize(icon_size);
    view->setViewMode(QListView::IconMode);
    return view;
}

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_hasValidPaths(false)
    , m_process(0)
    , m_currentImageFormat(SvgFormat)
    , m_needsRefresh(false)
{
    setWindowTitle(TITLE_FORMAT_STRING
                   .arg("")
                   .arg(qApp->applicationName())
                   );

    m_recentDocumentsSignalMapper = new QSignalMapper(this);
    connect(m_recentDocumentsSignalMapper, SIGNAL(mapped(QString)),
            this, SLOT(onRecentDocumentsActionTriggered(QString)));

    m_autoRefreshTimer = new QTimer(this);
    connect(m_autoRefreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));

    m_imageFormatNames[SvgFormat] = "svg";
    m_imageFormatNames[PngFormat] = "png";

    m_imageWidget = new PreviewWidget(this);
    setCentralWidget(m_imageWidget);

    m_exportPathLabel = new QLabel(this);
    m_exportPathLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_exportPathLabel->setMinimumWidth(200);
    m_exportPathLabel->setText(EXPORT_TO_LABEL_FORMAT_STRING.arg(""));
    m_exportPathLabel->setEnabled(false);

    m_currentImageFormatLabel = new QLabel(this);
    m_currentImageFormatLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    m_autorefreshLabel = new QLabel(this);
    m_autorefreshLabel->setText(AUTOREFRESH_STATUS_LABEL);
    m_autorefreshLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    statusBar()->addPermanentWidget(m_exportPathLabel);
    statusBar()->addPermanentWidget(m_autorefreshLabel);
    statusBar()->addPermanentWidget(m_currentImageFormatLabel);

    createDockWindows();
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();

    setUnifiedTitleAndToolBarOnMac(true);

    readSettings();
}

MainWindow::~MainWindow()
{
}

void MainWindow::newDocument()
{
    if (!maybeSave()) {
        return;
    }

    m_documentPath.clear();
    m_exportPath.clear();
    m_cachedImage.clear();
    m_exportImageAction->setText(EXPORT_TO_MENU_FORMAT_STRING.arg(""));
    m_exportPathLabel->setText(EXPORT_TO_LABEL_FORMAT_STRING.arg(""));
    m_exportPathLabel->setEnabled(false);

    QString text = "@startuml\n\nclass Foo\n\n@enduml";
    m_editor->setPlainText(text);
    setWindowTitle(TITLE_FORMAT_STRING
                   .arg(tr("Untitled"))
                   .arg(qApp->applicationName())
                   );
    setWindowModified(false);
    refresh();

    enableUndoRedoActions();
}

void MainWindow::undo()
{
    QTextDocument *document = m_editor->document();
    document->undo();
    enableUndoRedoActions();
}

void MainWindow::redo()
{
    QTextDocument *document = m_editor->document();
    document->redo();
    enableUndoRedoActions();
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About %1").arg(qApp->applicationName()),
                       tr(
                           "The <b>%1</b> allows simple edit and preview of UML "
                           "diagrams generated with <u>%2</u>.<br>"
                           "<br>"
                           "<u>%2</u> and <u>%3</u> must be installed before "
                           "using the editor.<br>"
                           "<br>"
                           "<center>Copyright (c) 2012 - Ionutz Borcoman</center>"
                           )
                       .arg(qApp->applicationName()).arg("PlantUML").arg("graphiz")
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

    if (!m_hasValidPaths) {
        qDebug() << "please configure paths for plantuml and java. aborting...";
        statusBar()->showMessage(tr("PlantUML or Java not found. Please set them correctly in the \"Preferences\" dialog!"));
        return;
    }

    QByteArray current_document = m_editor->toPlainText().toAscii();
    if (current_document.isEmpty()) {
        qDebug() << "empty document. skipping...";
        return;
    }
    m_needsRefresh = false;

    statusBar()->showMessage(tr("Refreshing..."));

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
            << "-jar" << m_platUmlPath
            << QString("-t%1").arg(m_imageFormatNames[m_currentImageFormat])
            << "-pipe";

    m_process = new QProcess(this);
    m_process->start(m_javaPath, arguments);
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
    m_cachedImage = m_process->readAll();
    m_imageWidget->load(m_cachedImage);
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
        m_currentImageFormatLabel->setText(m_imageFormatNames[m_currentImageFormat].toUpper());
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
    m_autorefreshLabel->setEnabled(state);
}

void MainWindow::onEditorChanged()
{
    m_needsRefresh = true;
    setWindowModified(true);

    enableUndoRedoActions();
}

void MainWindow::onRefreshActionTriggered()
{
    m_needsRefresh = true;
    refresh();
}

void MainWindow::onPreferencesActionTriggered()
{
    const int TIMEOUT_SCALE = 1000;
    PreferencesDialog dialog(this);
    dialog.setJavaPath(m_javaPath);
    dialog.setPlantUmlPath(m_platUmlPath);
    dialog.setAutoRefreshTimeout(m_autoRefreshTimer->interval() / TIMEOUT_SCALE);
    dialog.setAssistantXml(m_assistantXmlPath);
    dialog.exec();

    if (dialog.result() == QDialog::Accepted) {
        m_javaPath = dialog.javaPath();
        m_platUmlPath = dialog.plantUmlPath();
        m_autoRefreshTimer->setInterval(dialog.autoRefreshTimeout() * TIMEOUT_SCALE);
        checkPaths();

        reloadAssistantXml(dialog.assistantXml());
    }
}

void MainWindow::onOpenDocumentActionTriggered()
{
    openDocument("");
}

void MainWindow::onSaveActionTriggered()
{
    saveDocument(m_documentPath);
}

void MainWindow::onSaveAsActionTriggered()
{
    saveDocument("");
}

void MainWindow::onExportImageActionTriggered()
{
    exportImage(m_exportPath);
}

void MainWindow::onExportAsImageActionTriggered()
{
    exportImage("");
}

void MainWindow::onClearRecentDocumentsActionTriggered()
{
    m_recentDocumentsList.clear();
    updateRecentDocumentsMenu();
}

void MainWindow::onRecentDocumentsActionTriggered(const QString &path)
{
    openDocument(path);
}

void MainWindow::onAssistanItemClicked(QListWidgetItem *item)
{
    insertAssistantCode(item->data(Qt::UserRole).toString());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

bool MainWindow::maybeSave()
{
    if (m_editor->document()->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, qApp->applicationName(),
                                   tr("The document has been modified.\n"
                                      "Do you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return saveDocument(m_documentPath);
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void MainWindow::readSettings()
{
    QSettings settings;

    settings.beginGroup(SETTINGS_MAIN_SECTION);

    m_javaPath = settings.value(SETTINGS_JAVA_PATH, SETTINGS_JAVA_PATH_DEFAULT).toString();
    m_platUmlPath = settings.value(SETTINGS_PLATUML_PATH, SETTINGS_PLATUML_PATH_DEFAULT).toString();
    checkPaths();

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
    m_autorefreshLabel->setEnabled(autorefresh_enabled);

    m_currentImageFormat = m_imageFormatNames.key(settings.value(SETTINGS_IMAGE_FORMAT, m_imageFormatNames[SvgFormat]).toString());
    if (m_currentImageFormat == SvgFormat) {
        m_svgPreviewAction->setChecked(true);
    } else if (m_currentImageFormat == PngFormat) {
        m_pngPreviewAction->setChecked(true);
    }
    m_currentImageFormatLabel->setText(m_imageFormatNames[m_currentImageFormat].toUpper());

    reloadAssistantXml(settings.value(SETTINGS_ASSISTANT_XML_PATH).toString());

    settings.endGroup();

    int size = settings.beginReadArray(SETTINGS_RECENT_DOCUMENTS_SECTION);
    for (int index = 0; index < qMin(size, MAX_RECENT_DOCUMENT_SIZE); ++index) {
        settings.setArrayIndex(index);
        m_recentDocumentsList.append(settings.value(SETTINGS_RECENT_DOCUMENTS_DOCUMENT).toString());
    }
    settings.endArray();
    updateRecentDocumentsMenu();
}

void MainWindow::writeSettings()
{
    QSettings settings;

    settings.beginGroup(SETTINGS_MAIN_SECTION);
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
    settings.setValue(SETTINGS_WINDOW_STATE, saveState());
    settings.setValue(SETTINGS_SHOW_STATUSBAR, m_showStatusBarAction->isChecked());
    settings.setValue(SETTINGS_AUTOREFRESH_ENABLED, m_autoRefreshAction->isChecked());
    settings.setValue(SETTINGS_IMAGE_FORMAT, m_imageFormatNames[m_currentImageFormat]);
    settings.setValue(SETTINGS_AUTOREFRESH_TIMEOUT, m_autoRefreshTimer->interval());
    settings.setValue(SETTINGS_JAVA_PATH, m_javaPath);
    settings.setValue(SETTINGS_PLATUML_PATH, m_platUmlPath);
    settings.setValue(SETTINGS_ASSISTANT_XML_PATH, m_assistantXmlPath);
    settings.endGroup();

    settings.remove(SETTINGS_RECENT_DOCUMENTS_SECTION);
    settings.beginWriteArray(SETTINGS_RECENT_DOCUMENTS_SECTION);
    for(int index = 0; index < m_recentDocumentsList.size(); ++index) {
        settings.setArrayIndex(index);
        settings.setValue(SETTINGS_RECENT_DOCUMENTS_DOCUMENT, m_recentDocumentsList.at(index));
    }
    settings.endArray();
}

void MainWindow::openDocument(const QString &name)
{
    if (!maybeSave()) {
        return;
    }

    QString tmp_name = name;

    if (tmp_name.isEmpty()) {
        tmp_name = QFileDialog::getOpenFileName(this,
                                                tr("Select a file to open"),
                                                QString(),
                                                "PlantUML (*.plantuml);; All Files (*.*)"
                                                );
        if (tmp_name.isEmpty()) {
            return;
        }
    }

    QFile file(tmp_name);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    m_editor->setPlainText(file.readAll());
    setWindowModified(false);
    m_documentPath = tmp_name;
    setWindowTitle(TITLE_FORMAT_STRING
                   .arg(QFileInfo(tmp_name).fileName())
                   .arg(qApp->applicationName())
                   );
    m_needsRefresh = true;
    refresh();
    updateRecentDocumentsList(tmp_name);
}

bool MainWindow::saveDocument(const QString &name)
{
    QString tmp_name = name;
    if (tmp_name.isEmpty()) {
        tmp_name = QFileDialog::getSaveFileName(this,
                                                tr("Select where to store the document"),
                                                m_documentPath,
                                                "PlantUML (*.plantuml);; All Files (*.*)"
                                                );
        if (tmp_name.isEmpty()) {
            return false;
        }
    }

    qDebug() << "saving document in:" << tmp_name;
    QFile file(tmp_name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    file.write(m_editor->toPlainText().toAscii());
    setWindowModified(false);
    m_documentPath = tmp_name;
    setWindowTitle(TITLE_FORMAT_STRING
                   .arg(QFileInfo(tmp_name).fileName())
                   .arg(qApp->applicationName())
                   );
    statusBar()->showMessage(tr("Document save in %1").arg(tmp_name), STATUSBAR_TIMEOUT);
    updateRecentDocumentsList(tmp_name);
    return true;
}

void MainWindow::exportImage(const QString &name)
{
    if (m_cachedImage.isEmpty()) {
        qDebug() << "no image to export. aborting...";
        return;
    }

    QString tmp_name = name;
    if (tmp_name.isEmpty()) {
        tmp_name = QFileDialog::getSaveFileName(this,
                                                tr("Select where to export the image"),
                                                m_documentPath,
                                                "Image (*.svg *.png);; All Files (*.*)"
                                                );
        if (tmp_name.isEmpty()) {
            return;
        }
    }

    qDebug() << "exporting image in:" << tmp_name;

    QFile file(tmp_name);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }
    file.write(m_cachedImage);
    m_exportImageAction->setText(EXPORT_TO_MENU_FORMAT_STRING.arg(tmp_name));
    m_exportPath = tmp_name;
    QString short_tmp_name = QFileInfo(tmp_name).fileName();
    statusBar()->showMessage(tr("Image exported in %1").arg(short_tmp_name), STATUSBAR_TIMEOUT);
    m_exportPathLabel->setText(EXPORT_TO_LABEL_FORMAT_STRING.arg(short_tmp_name));
    m_exportPathLabel->setEnabled(true);
}

void MainWindow::createActions()
{
    // File menu
    m_newDocumentAction = new QAction(QIcon::fromTheme("document-new"), tr("&New"), this);
    m_newDocumentAction->setShortcut(QKeySequence::New);
    connect(m_newDocumentAction, SIGNAL(triggered()), this, SLOT(newDocument()));

    m_openDocumentAction = new QAction(QIcon::fromTheme("document-open"), tr("&Open"), this);
    m_openDocumentAction->setShortcuts(QKeySequence::Open);
    connect(m_openDocumentAction, SIGNAL(triggered()), this, SLOT(onOpenDocumentActionTriggered()));

    m_saveDocumentAction = new QAction(QIcon::fromTheme("document-save"), tr("&Save"), this);
    m_saveDocumentAction->setShortcuts(QKeySequence::Save);
    connect(m_saveDocumentAction, SIGNAL(triggered()), this, SLOT(onSaveActionTriggered()));

    m_saveAsDocumentAction = new QAction(QIcon::fromTheme("document-save-as"), tr("Save As..."), this);
    m_saveAsDocumentAction->setShortcuts(QKeySequence::SaveAs);
    connect(m_saveAsDocumentAction, SIGNAL(triggered()), this, SLOT(onSaveAsActionTriggered()));

    m_exportImageAction = new QAction(EXPORT_TO_MENU_FORMAT_STRING.arg(""), this);
    m_exportImageAction->setShortcut(Qt::CTRL + Qt::Key_E);
    connect(m_exportImageAction, SIGNAL(triggered()), this, SLOT(onExportImageActionTriggered()));

    m_exportAsImageAction = new QAction(tr("Export as ..."), this);
    m_exportAsImageAction->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_E);
    connect(m_exportAsImageAction, SIGNAL(triggered()), this, SLOT(onExportAsImageActionTriggered()));

    m_quitAction = new QAction(QIcon::fromTheme("application-exit"), tr("&Quit"), this);
    m_quitAction->setShortcuts(QKeySequence::Quit);
    m_quitAction->setStatusTip(tr("Quit the application"));
    connect(m_quitAction, SIGNAL(triggered()), this, SLOT(close()));

    // Recent Documents menu
    m_clearRecentDocumentsAction = new QAction(tr("Clear Menu"), this);
    connect(m_clearRecentDocumentsAction, SIGNAL(triggered()), this, SLOT(onClearRecentDocumentsActionTriggered()));

    // Edit menu
    m_undoAction = new QAction(QIcon::fromTheme("edit-undo"), tr("&Undo"), this);
    m_undoAction->setShortcuts(QKeySequence::Undo);
    connect(m_undoAction, SIGNAL(triggered()), this, SLOT(undo()));

    m_redoAction = new QAction(QIcon::fromTheme("edit-redo"), tr("&Redo"), this);
    m_redoAction->setShortcuts(QKeySequence::Redo);
    connect(m_redoAction, SIGNAL(triggered()), this, SLOT(redo()));

    // Tools menu
    m_pngPreviewAction = new QAction(tr("PNG"), this);
    m_pngPreviewAction->setCheckable(true);
    m_pngPreviewAction->setStatusTip(tr("Tell PlantUML to produce PNG output"));
    connect(m_pngPreviewAction, SIGNAL(toggled(bool)), this, SLOT(changeImageFormat()));

    m_svgPreviewAction = new QAction(tr("SVG"), this);
    m_svgPreviewAction->setCheckable(true);
    m_svgPreviewAction->setStatusTip(tr("Tell PlantUML to produce SVG output"));
    connect(m_svgPreviewAction, SIGNAL(toggled(bool)), this, SLOT(changeImageFormat()));

    QActionGroup* output_action_group = new QActionGroup(this);
    output_action_group->setExclusive(true);
    output_action_group->addAction(m_pngPreviewAction);
    output_action_group->addAction(m_svgPreviewAction);
    m_svgPreviewAction->setChecked(true);

    m_refreshAction = new QAction(QIcon::fromTheme("view-refresh"), tr("Refresh"), this);
    m_refreshAction->setShortcuts(QKeySequence::Refresh);
    m_refreshAction->setStatusTip(tr("Call PlantUML to regenerate the UML image"));
    connect(m_refreshAction, SIGNAL(triggered()), this, SLOT(onRefreshActionTriggered()));

    m_autoRefreshAction = new QAction(tr("Auto-Refresh"), this);
    m_autoRefreshAction->setCheckable(true);
    connect(m_autoRefreshAction, SIGNAL(toggled(bool)), this, SLOT(onAutoRefreshActionToggled(bool)));

    // Settings menu
    m_showMainToolbarAction = new QAction(tr("Show toolbar"), this);
    m_showMainToolbarAction->setCheckable(true);

    m_showStatusBarAction = new QAction(tr("Show statusbar"), this);
    m_showStatusBarAction->setCheckable(true);

    m_preferencesAction = new QAction(QIcon::fromTheme("preferences-other"), tr("Preferences"), this);
    connect(m_preferencesAction, SIGNAL(triggered()), this, SLOT(onPreferencesActionTriggered()));

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
    m_recentDocumentsMenu = m_fileMenu->addMenu(tr("Recent Documents"));
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exportImageAction);
    m_fileMenu->addAction(m_exportAsImageAction);
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
    m_settingsMenu->addSeparator();
    m_settingsMenu->addAction(m_showAssistantDockAction);
    m_settingsMenu->addAction(m_showEditorDockAction);
    m_settingsMenu->addSeparator();
    m_settingsMenu->addAction(m_pngPreviewAction);
    m_settingsMenu->addAction(m_svgPreviewAction);
    m_settingsMenu->addSeparator();
    m_settingsMenu->addAction(m_autoRefreshAction);
    m_settingsMenu->addSeparator();
    m_settingsMenu->addAction(m_preferencesAction);

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
    m_mainToolBar->addAction(m_showEditorDockAction);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(m_undoAction);
    m_mainToolBar->addAction(m_redoAction);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(m_refreshAction);
    m_mainToolBar->addSeparator();
    m_mainToolBar->addAction(m_preferencesAction);
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"), STATUSBAR_TIMEOUT);
}

void MainWindow::createDockWindows()
{
    QDockWidget *dock = new QDockWidget(tr("Text Editor"), this);
    m_editor = new QTextEdit(dock);
    connect(m_editor->document(), SIGNAL(contentsChanged()), this, SLOT(onEditorChanged()));
    dock->setWidget(m_editor);
    dock->setObjectName("text_editor");
    addDockWidget(Qt::RightDockWidgetArea, dock);

    m_showEditorDockAction = dock->toggleViewAction();
    m_showEditorDockAction->setIconVisibleInMenu(false);
    m_showEditorDockAction->setStatusTip("Show or hide the document editor");
    m_showEditorDockAction->setIcon(QIcon::fromTheme("accessories-text-editor"));

    dock = new QDockWidget(tr("Assistant"), this);
    m_assistantToolBox = new QToolBox(dock);
    dock->setWidget(m_assistantToolBox);
    dock->setObjectName("assistant");
    addDockWidget(Qt::LeftDockWidgetArea, dock);

    m_showAssistantDockAction = dock->toggleViewAction();
}

void MainWindow::enableUndoRedoActions()
{
    QTextDocument *document = m_editor->document();
    m_undoAction->setEnabled(document->isUndoAvailable());
    m_redoAction->setEnabled(document->isRedoAvailable());
}

void MainWindow::checkPaths()
{
    m_hasValidPaths = QFileInfo(m_javaPath).exists() &&
            QFileInfo(m_platUmlPath).exists();
}

void MainWindow::updateRecentDocumentsList(const QString &path)
{
    if (!path.isEmpty()) {
        if (m_recentDocumentsList.size() == 0 || path != m_recentDocumentsList[0]) {
            m_recentDocumentsList.insert(0, path);

            // remove duplicates
            int index = m_recentDocumentsList.lastIndexOf(path);
            while (index > 0) {
                m_recentDocumentsList.removeAt(index);
                index = m_recentDocumentsList.lastIndexOf(path);
            }

            // keep at most MAX_RECENT_DOCUMENT_SIZE
            while (m_recentDocumentsList.size() > MAX_RECENT_DOCUMENT_SIZE) {
                m_recentDocumentsList = m_recentDocumentsList.mid(0, MAX_RECENT_DOCUMENT_SIZE);
            }
        }
        updateRecentDocumentsMenu();
    }
}

void MainWindow::updateRecentDocumentsMenu()
{
    foreach (QAction* action, m_recentDocumentsMenu->actions()) {
        if (action != m_clearRecentDocumentsAction) {
            action->deleteLater();
        }
    }
    m_recentDocumentsMenu->clear();

    foreach(const QString& path, m_recentDocumentsList) {
        QAction *action = new QAction(path, this);
        m_recentDocumentsMenu->addAction(action);
        connect(action, SIGNAL(triggered()), m_recentDocumentsSignalMapper, SLOT(map()));
        m_recentDocumentsSignalMapper->setMapping(action, path);
    }
    if (m_recentDocumentsList.size()) {
        m_recentDocumentsMenu->addSeparator();
    }
    m_recentDocumentsMenu->addAction(m_clearRecentDocumentsAction);
}

void MainWindow::reloadAssistantXml(const QString &path)
{
    if (m_assistantXmlPath != path) {
        m_assistantXmlPath = path;

        foreach (QListWidget* widget, m_assistantWidgets) {
            widget->deleteLater();
        }
        m_assistantWidgets.clear();

        if (m_assistantXmlPath.isEmpty()) {
            qDebug() << "no assistant defined";
        } else {
            qDebug() << "using assistant" << m_assistantXmlPath;
            AssistantXmlReader reader;
            reader.readFile(m_assistantXmlPath);
            for (int i = 0; i < reader.size(); ++i) {
                const Assistant* assistant = reader.assistant(i);
                QListWidget* view = newAssistantListWidget(ASSISTANT_ICON_SIZE, this);
                for (int j = 0; j < assistant->size(); ++j) {
                    const AssistantItem* assistantItem = assistant->item(j);
                    QListWidgetItem* listWidgetItem =
                            new QListWidgetItem(iconFromSvg(ASSISTANT_ICON_SIZE, assistantItem->icon()),
                                                assistantItem->name(), view);
                    listWidgetItem->setData(Qt::UserRole, assistantItem->data());
                }
                m_assistantToolBox->addItem(view, assistant->name());
                connect(view, SIGNAL(itemClicked(QListWidgetItem*)),
                        this, SLOT(onAssistanItemClicked(QListWidgetItem*)));
                m_assistantWidgets << view;
            }
        }
    }
}

void MainWindow::insertAssistantCode(const QString &code)
{
    if (code.isEmpty())
        return;

    QTextCursor cursor = m_editor->textCursor();
    if (!cursor.isNull()) {
        cursor.beginEditBlock();
        cursor.removeSelectedText();
        cursor.insertText(code);
        cursor.endEditBlock();
    }
}
