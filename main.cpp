#include <QApplication>
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

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
