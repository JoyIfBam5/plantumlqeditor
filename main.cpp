#include <QtSingleApplication>
#include <QDebug>
#include "mainwindow.h"

namespace {
const char* APPLICATION_NAME = "PlantUML Editor";
const char* ORGANIZATION_NAME   = "Ionutz Borcoman";
const char* ORGANIZATION_DOMAIN = "borco.net";
}

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(ORGANIZATION_NAME);
    QCoreApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
    QCoreApplication::setApplicationName(APPLICATION_NAME);

    QtSingleApplication a(argc, argv);

    QString open_document_path;
    if (argc == 2) {
        open_document_path = argv[1];
    }

    if (a.isRunning()) {
        a.sendMessage(open_document_path);
        qDebug() << qPrintable(QObject::tr("%1 already running. Requested instance to open: %2. Now exitting...")
                               .arg(APPLICATION_NAME)
                               .arg(open_document_path));
        return 0;
    }

    MainWindow w;
    if (open_document_path.isEmpty()) {
        w.newDocument();
    } else {
        w.openDocument(open_document_path);
    }
    w.show();

    return a.exec();
}
