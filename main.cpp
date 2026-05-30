#include "mainwindow.h"
#include <QApplication>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("ProxyLab");
    app.setOrganizationName("ProxyLab");

    QFont font("Segoe UI", 9);
    app.setFont(font);

    MainWindow w;
    w.show();
    return app.exec();
}
